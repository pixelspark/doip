#include "tjoscudpconnection.h"
#include "../../include/tjfabricmessage.h"
#include "tjoscutil.h"
#include "../../../Libraries/OSCPack/osc/OscOutboundPacketStream.h"
#include "../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include "../../../Libraries/OSCPack/osc/OscPacketListener.h"
#include "../../../Libraries/OSCPack/osc/OscPrintReceivedElements.h"

#include <errno.h>

#ifdef TJ_OS_POSIX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif

#ifdef TJ_OS_WIN
	#include <Winsock2.h>
#endif

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::fabric::connections;
using namespace tj::np;

/** NetworkAddress **/
NetworkAddress::NetworkAddress(const String& s, bool passive): _family(AddressFamilyNone) {
	if(s.length()>0) {
		memset(&_address, 0, sizeof(_address));
		addrinfo* firstResult;
		addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_flags = AI_V4MAPPED|(passive ? AI_PASSIVE : 0)|AI_ADDRCONFIG;
		hints.ai_socktype = 0;
		hints.ai_protocol = 0;
		
		bool success = false;
		std::string mbs = Mbs(s);
		int r = getaddrinfo(mbs.c_str(), NULL, &hints, &firstResult);
		if(r==0) {
			if(firstResult!=NULL) {
				if(firstResult->ai_family==AF_INET6) {
					_family = AddressFamilyIPv6;
					memcpy(&_address, firstResult->ai_addr, sizeof(sockaddr_in6));
					success = true;
				}
				else if(firstResult->ai_family==AF_INET) {
					_family = AddressFamilyIPv4;
					memcpy(&_v4address, firstResult->ai_addr, sizeof(sockaddr_in));
					success = true;
				}
				freeaddrinfo(firstResult);
			}
		}
		else {
			#ifdef TJ_OS_POSIX
				std::wstring error = Wcs(std::string(gai_strerror(r)));
			#endif

			#ifdef TJ_OS_WIN
				std::wstring error = std::wstring(gai_strerror(r));
			#endif
			
			Log::Write(L"TJNP/NetworkAddress", L"getaddrinfo() failed: " + error);
		}
	}
}

NetworkAddress::~NetworkAddress() {
}

AddressFamily NetworkAddress::GetAddressFamily() const {
	return _family;
}

bool NetworkAddress::GetIPv6SocketAddress(sockaddr_in6* addr) const {
	if(_family==AddressFamilyIPv6) {
		memcpy(addr, &_address, sizeof(_address));
		return true;
	}
	return false;
}

bool NetworkAddress::GetIPv4SocketAddress(sockaddr_in* addr) const {
	if(_family==AddressFamilyIPv4) {
		memcpy(addr, &_v4address, sizeof(_v4address));
		return true;
	}
	return false;
}

std::wstring NetworkAddress::ToString() const {
	if(_family==AddressFamilyNone) {
		return L"";
	}
	else if(_family==AddressFamilyIPv6) {
		char buffer[255];
		memset(buffer, 0, sizeof(char)*255);
		std::string friendlyAddress = inet_ntop(AF_INET6, (void*)&(_address.sin6_addr), buffer, 255);
		return Wcs(friendlyAddress)+L'%'+StringifyHex(_address.sin6_scope_id);
	}
	else if(_family==AddressFamilyIPv4) {
		char buffer[255];
		memset(buffer, 0, sizeof(char)*255);
		std::string friendlyAddress = inet_ntop(AF_INET, (void*)&(_v4address.sin_addr), buffer, 255);
		return Wcs(friendlyAddress);
	}
	return L"[???]";
}

/** OSCOverUDPConnectionDefinition **/
OSCOverUDPConnectionDefinition::OSCOverUDPConnectionDefinition(): ConnectionDefinition(L"udp") {
}

OSCOverUDPConnectionDefinition::~OSCOverUDPConnectionDefinition() {
}

void OSCOverUDPConnectionDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall<std::wstring>(me, "address", _address);
	SaveAttributeSmall<int>(me, "port", int(_port));
}

void OSCOverUDPConnectionDefinition::Load(TiXmlElement* me) {
	_address = LoadAttributeSmall<std::wstring>(me, "address", _address);
	_port = (unsigned short)LoadAttributeSmall<int>(me, "port", (int)_port);
}

/** OSCOverUDPConnection **/
OSCOverUDPConnection::OSCOverUDPConnection(): _outSocket(-1), _inSocket(-1), _in4Socket(-1), _toAddress(L"") {
}

OSCOverUDPConnection::~OSCOverUDPConnection() {
	if(_outSocket!=-1) {
		#ifdef TJ_OS_POSIX
			close(_outSocket);
		#endif

		#ifdef TJ_OS_WIN
			closesocket(_outSocket);
		#endif	
	}
	
	if(_inSocket!=-1) {
		_listenerThread->Stop();
		#ifdef TJ_OS_POSIX
			close(_inSocket);
			close(_in4Socket);
		#endif

		#ifdef TJ_OS_WIN
			closesocket(_inSocket);
			closesocket(_in4Socket);
		#endif
	}
}

void OSCOverUDPConnection::Create(const std::wstring& address, unsigned short port, Direction direction) {
	// Create outgoing socket
	if((direction & DirectionOutbound)!=0) {
		_toAddress = NetworkAddress(address);
		_toPort = port;
		int on = 1;
		_outSocket = socket((_toAddress.GetAddressFamily()==AddressFamilyIPv6) ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(_outSocket==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not create UDP socket");
		}
		
		setsockopt(_outSocket,SOL_SOCKET,SO_BROADCAST,(const char*)&on, sizeof(int));
		setsockopt(_outSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&on, sizeof(int));
		Log::Write(L"TJFabric/OSCOverUDPConnection", std::wstring(L"Connected outbound OSC-over-UDP (")+Stringify(address)+L":"+Stringify(port)+L")");
	}
	
	// Create server socket and thread
	if((direction & DirectionInbound)!=0) {
		_inSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		_in4Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		
		// Fill in the interface information for the IPv6 listener socket
		in6_addr any = IN6ADDR_ANY_INIT;
		sockaddr_in6 addr;
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(port);
		addr.sin6_addr = any;
		
		// Addresses for the IPv4 listener socket
		sockaddr_in addr4;
		memset(&addr4, 0, sizeof(addr4));
		addr4.sin_family = AF_INET;
		addr4.sin_port = htons(port);
		addr4.sin_addr.s_addr = INADDR_ANY;
		addr4.sin_len = sizeof(sockaddr_in);
		
		int on = 1;
		setsockopt(_inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		setsockopt(_in4Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		
		// Bind IPv6 socket
		int err = bind(_inSocket, (sockaddr*)&addr, sizeof(addr));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not bind IPv6 server socket, error="+Stringify(errno));
			return;
		}
		
		// Bind IPv4 socket
		err = bind(_in4Socket, (sockaddr*)&addr4, sizeof(addr4));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not bind IPv4 server socket, error="+Stringify(errno));
			return;
		}
		
		// try to make us member of the multicast group (IPv6)
		struct ipv6_mreq mreq;
		inet_pton(AF_INET6, Mbs(address).c_str(), &(mreq.ipv6mr_multiaddr));
		mreq.ipv6mr_interface = 0;
		setsockopt(_inSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, sizeof(mreq));
		
		// try to make us member of the multicast group (IPv4)
		struct ip_mreq mreq4;
		mreq4.imr_interface.s_addr = INADDR_ANY;
		mreq4.imr_multiaddr.s_addr = addr4.sin_addr.s_addr;
		setsockopt(_in4Socket, IPPROTO_IPV4, IP_ADD_MEMBERSHIP, (char*)&mreq4, sizeof(mreq4));
		
		_listenerThread = GC::Hold(new SocketListenerThread());
		_listenerThread->AddListener(_inSocket, this);
		_listenerThread->AddListener(_in4Socket, this);
		_listenerThread->Start();
		
		Log::Write(L"TJFabric/OSCOverUDPConnection", std::wstring(L"Connected inbound OSC-over-UDP (")+Stringify(address)+L":"+Stringify(port)+L")");
	}	
}

void OSCOverUDPConnection::Create(strong<ConnectionDefinition> def, Direction direction) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverUDPConnectionDefinition>()) {
		ref<OSCOverUDPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			_def = cd;
			Create(cd->_address, cd->_port, direction);
		}
	}
	else {
		Throw(L"Invalid connection definition type for this connection type", ExceptionTypeError);
	}
}

void OSCOverUDPConnection::OnReceive(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	char receiveBuffer[4096];
	int ret = recvfrom(ns, receiveBuffer, 4096-1, 0, NULL, 0);
	if(ret==-1) {
		// Something was wrong
		return;
	}
	
	osc::ReceivedPacket msg(receiveBuffer, ret);
	if(msg.IsBundle()) {
		OnReceiveBundle(osc::ReceivedBundle(msg));
	}
	else {
		OnReceiveMessage(osc::ReceivedMessage(msg));
	}
}

void OSCOverUDPConnection::OnReceiveMessage(osc::ReceivedMessage rm) {
	ref<Message> msg = GC::Hold(new Message(Wcs(rm.AddressPattern())));
	
	// Convert OSC arguments to Any values
	osc::ReceivedMessageArgumentIterator ait = rm.ArgumentsBegin();
	unsigned int i = 0;
	while(ait!=rm.ArgumentsEnd()) {
		Any val;
		if(OSCUtil::ArgumentToAny(*ait, val)) {
			msg->SetParameter(i, val);
		}
		++i;
		++ait;
	}
	
	EventMessageReceived.Fire(this, MessageNotification(Timestamp(true), msg));
}

void OSCOverUDPConnection::OnReceiveBundle(osc::ReceivedBundle rb) {
	osc::ReceivedBundle::const_iterator it = rb.ElementsBegin(); 
	while(it!=rb.ElementsEnd()) {
		if(it->IsBundle()) {
			OnReceiveBundle(osc::ReceivedBundle(*it));
		}
		else {
			OnReceiveMessage(osc::ReceivedMessage(*it));
		}
		++it;
	}
}

void OSCOverUDPConnection::Send(strong<Message> msg) {
	ThreadLock lock(&_lock);
	
	if(_outSocket==-1) {
		Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not send message, outgoing socket is invalid");
		return;
	}
	
	char* buffer[2048];
	osc::OutboundPacketStream outPacket((char*)buffer, 2047);
	outPacket << osc::BeginMessage(Mbs(msg->GetPath()).c_str());
	
	std::wostringstream wos;
	wos << msg->GetPath() << L",";
	
	for(unsigned int a=0;a<msg->GetParameterCount();a++) {
		Any value = msg->GetParameter(a);
		switch(value.GetType()) {
			case Any::TypeBool:
				outPacket << (bool)value;
				wos << L'b';
				break;
				
			case Any::TypeDouble:
				outPacket << (double)value;
				wos << L'd';
				break;
				
			case Any::TypeInteger:
				outPacket << (osc::int32)(int)value;
				wos << L'i';
				break;
				
			case Any::TypeString:
				outPacket << Mbs(value.ToString()).c_str();
				wos << L's';
				break;
				
			default:
			case Any::TypeObject:
			case Any::TypeTuple:
			case Any::TypeNull:
				outPacket << osc::Nil;
				wos << L'0';
				break;
		};
	}
	outPacket << osc::EndMessage;
	
	void* toAddress = 0;
	unsigned int toAddressSize = 0;
	sockaddr_in6 addr6;
	sockaddr_in addr4;
	
	if(_toAddress.GetAddressFamily()==AddressFamilyIPv6) {
		_toAddress.GetIPv6SocketAddress(&addr6);
		addr6.sin6_port = htons(_toPort);
		toAddress = reinterpret_cast<void*>(&addr6);
		toAddressSize = sizeof(sockaddr_in6);
	}
	else if(_toAddress.GetAddressFamily()==AddressFamilyIPv4) {
		_toAddress.GetIPv4SocketAddress(&addr4);
		addr4.sin_port = htons(_toPort);
		toAddress = reinterpret_cast<void*>(&addr4);
		toAddressSize = sizeof(sockaddr_in);
	}
		
	if(sendto(_outSocket, (const char*)buffer, outPacket.Size(), 0, reinterpret_cast<const sockaddr*>(toAddress), toAddressSize)==-1) {
		Log::Write(L"TJFabric/OSCOverUDPConnection", L"sendto() failed, error="+Stringify(errno));
	}
	
	wos << L" => " << _toAddress.ToString();
	Log::Write(L"TJFabric/OSCOverUDPConnection", wos.str());
}