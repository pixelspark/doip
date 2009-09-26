#include "tjoscudpconnection.h"
#include "../../include/tjfabricmessage.h"
#include "tjoscutil.h"
#include "../../../Libraries/OSCPack/osc/OscOutboundPacketStream.h"
#include "../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include "../../../Libraries/OSCPack/osc/OscPacketListener.h"
#include "../../../Libraries/OSCPack/osc/OscPrintReceivedElements.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

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
		hints.ai_family = AF_INET6;
		hints.ai_flags = AI_V4MAPPED|(passive ? AI_PASSIVE : 0);
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
				freeaddrinfo(firstResult);
			}
		}
		else {
			std::cout << gai_strerror(r) << std::endl;
		}
	}
}

NetworkAddress::~NetworkAddress() {
}

void NetworkAddress::GetSocketAddress(sockaddr_in6* addr) const {
	memcpy(addr, &_address, sizeof(_address));
}

std::wstring NetworkAddress::ToString() const {
	if(_family==AddressFamilyNone) {
		return L"";
	}
	
	char buffer[255];
	memset(buffer, 0, sizeof(char)*255);
	std::string friendlyAddress = inet_ntop(AF_INET6, &(_address.sin6_addr), buffer, 255);
	return Wcs(friendlyAddress);
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
OSCOverUDPConnection::OSCOverUDPConnection(): _outSocket(-1), _inSocket(-1), _toAddress(L"") {
}

OSCOverUDPConnection::~OSCOverUDPConnection() {
	if(_outSocket!=-1) {
		close(_outSocket);
	}
	
	if(_inSocket!=-1) {
		_listenerThread->Stop();
		close(_inSocket);
	}
}

void OSCOverUDPConnection::Create(strong<ConnectionDefinition> def, Direction direction) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverUDPConnectionDefinition>()) {
		ref<OSCOverUDPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			_def = cd;
			
			// Create outgoing socket
			if((direction & DirectionOutbound)!=0) {
				_toAddress = NetworkAddress(cd->_address);
				int on = 1;
				_outSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
				if(_outSocket==-1) {
					Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not create UDP socket");
				}
				
				setsockopt(_outSocket,SOL_SOCKET,SO_BROADCAST,(const char*)&on, sizeof(int));
				setsockopt(_outSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&on, sizeof(int));
				Log::Write(L"TJFabric/OSCOverUDPConnection", std::wstring(L"Connected outbound OSC-over-UDP (")+Stringify(_def->_address)+L":"+Stringify(_def->_port)+L")");
			}
			
			// Create server socket and thread
			if((direction & DirectionInbound)!=0) {
				_inSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
				
				// Fill in the interface information
				/*sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_port = htons(cd->_port);
				addr.sin_addr.s_addr = INADDR_ANY;*/
				in6_addr any = IN6ADDR_ANY_INIT;
				sockaddr_in6 addr;
				addr.sin6_family = AF_INET6;
				addr.sin6_port = htons(cd->_port);
				addr.sin6_addr = any;
							
				int on = 1;
				setsockopt(_inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
				
				int err = bind(_inSocket, (sockaddr*)&addr, sizeof(addr));
				if(err==-1) {
					Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not bind server socket, error="+Stringify(errno));
					return;
				}
				
				// try to make us member of the multicast group
				struct ipv6_mreq mreq;
				inet_pton(AF_INET6, Mbs(_def->_address).c_str(), &(mreq.ipv6mr_multiaddr));
				mreq.ipv6mr_interface = 0;
				setsockopt(_inSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, sizeof(mreq));
				
				_listenerThread = GC::Hold(new SocketListenerThread(_inSocket, this));
				_listenerThread->Start();
			}
		}
	}
	else {
		Throw(L"Invalid connection definition type for this connection type", ExceptionTypeError);
	}
}

void OSCOverUDPConnection::OnReceive(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	sockaddr_in from;
	socklen_t size = (int)sizeof(from);
	char receiveBuffer[2048];
	int ret = recvfrom(_inSocket, receiveBuffer, 2048-1, 0, (sockaddr*)&from, &size);
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
		return;
	}
	
	sockaddr_in6 addr;
	_toAddress.GetSocketAddress(&addr);
	addr.sin6_port = htons(_def->_port);
	char* buffer[2048];
	osc::OutboundPacketStream outPacket((char*)buffer, 2047);
	
	outPacket << osc::BeginMessage(Mbs(msg->GetPath()).c_str());
	for(unsigned int a=0;a<msg->GetParameterCount();a++) {
		Any value = msg->GetParameter(a);
		switch(value.GetType()) {
			case Any::TypeBool:
				outPacket << (bool)value;
				break;
				
			case Any::TypeDouble:
				outPacket << (double)value;
				break;
				
			case Any::TypeInteger:
				outPacket << (osc::int32)(int)value;
				break;
				
			case Any::TypeString:
				outPacket << Mbs(value.ToString()).c_str();
				break;
				
			default:
			case Any::TypeObject:
			case Any::TypeTuple:
			case Any::TypeNull:
				outPacket << osc::Nil;
				break;
		};
	}
	outPacket << osc::EndMessage;
	if(sendto(_outSocket, buffer, outPacket.Size(), 0, (const sockaddr*)&addr, sizeof(sockaddr_in6))==-1) {
		Log::Write(L"TJFabric/OSCOverUDPConnection", L"sendto() failed, error="+Stringify(errno));
	}
	std::cout << osc::ReceivedPacket((const char*)buffer, outPacket.Size());
}