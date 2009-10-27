#include "tjoscipconnection.h"
#include "../../include/tjfabricmessage.h"
#include "../../include/tjfabricengine.h"
#include "tjoscutil.h"
#include "../../../../Libraries/OSCPack/osc/OscOutboundPacketStream.h"
#include "../../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include "../../../../Libraries/OSCPack/osc/OscPacketListener.h"
#include "../../../../Libraries/OSCPack/osc/OscPrintReceivedElements.h"

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

/** OSCOverIPConnectionDefinition **/
OSCOverIPConnectionDefinition::OSCOverIPConnectionDefinition(const String& upperProtocol): ConnectionDefinition(upperProtocol), _format(L"osc") {
} 

OSCOverIPConnectionDefinition::~OSCOverIPConnectionDefinition() {
}

void OSCOverIPConnectionDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall<std::wstring>(me, "address", _address);
	SaveAttributeSmall<int>(me, "port", int(_port));
	SaveAttributeSmall<std::wstring>(me, "format", _format);
	SaveAttributeSmall<std::wstring>(me, "framing", _framing);
}

void OSCOverIPConnectionDefinition::Load(TiXmlElement* me) {
	_address = LoadAttributeSmall<std::wstring>(me, "address", _address);
	_port = (unsigned short)LoadAttributeSmall<int>(me, "port", (int)_port);
	_format = LoadAttributeSmall<std::wstring>(me, "format", _format);	
	_framing = LoadAttributeSmall<std::wstring>(me,"framing", _framing);
}

std::wstring OSCOverIPConnectionDefinition::GetFramingType() const {
	return _framing;
}

/** OSCOverIPConnection **/
OSCOverIPConnection::OSCOverIPConnection(): _useSendTo(false), _outSocket(-1), _inSocket(-1), _in4Socket(-1), _toPort(0), _toAddress(L""), _direction(DirectionNone) {
}

OSCOverIPConnection::~OSCOverIPConnection() {
	StopOutbound();
	StopInbound();
}

Direction OSCOverIPConnection::GetDirection() const {
	return _direction;
}

void OSCOverIPConnection::StopInbound() {
	if((_direction & DirectionInbound) !=0) {
		_listenerThread->Stop();
		_listenerThread = null; // This will call ~SocketListenerThread => Thread::WaitForCompletion on the thread
		
		#ifdef TJ_OS_POSIX
			close(_inSocket);
			close(_in4Socket);
		#endif
				
		#ifdef TJ_OS_WIN
			closesocket(_inSocket);
			closesocket(_in4Socket);
		#endif
		
		std::deque<NativeSocket>::iterator it = _additionalIncomingSockets.begin();
		while(it!=_additionalIncomingSockets.end()) {
			#ifdef TJ_OS_POSIX
				close(*it);
			#endif
			
			#ifdef TJ_OS_WIN
				closesocket(*it);
			#endif
			
			++it;
		}
		
		_additionalIncomingSockets.clear();
		_direction = (Direction)(_direction & (~DirectionInbound));
	}
}

void OSCOverIPConnection::AddInboundConnection(NativeSocket ns) {
	ThreadLock lock(&_lock);
	if((_direction & DirectionInbound)!=0) {
		_additionalIncomingSockets.push_back(ns);
		if(_listenerThread) {
			_listenerThread->AddListener(ns, this);
		}
	}
}

void OSCOverIPConnection::RemoveInboundConnection(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	std::deque<tj::np::NativeSocket>::iterator it = _additionalIncomingSockets.begin();
	while(it!=_additionalIncomingSockets.end()) {
		if((*it) == ns) {
			_additionalIncomingSockets.erase(it);
			break;
		}
		++it;
	}
	
	if(_listenerThread) {
		_listenerThread->RemoveListener(ns);
	}
	
	#ifdef TJ_OS_POSIX
		close(ns);
	#endif
	
	#ifdef TJ_OS_WIN
		closesocket(ns);
	#endif
}

void OSCOverIPConnection::StopOutbound() {
	if((_direction & DirectionOutbound) != 0) {
		#ifdef TJ_OS_POSIX
			close(_outSocket);
		#endif
					
		#ifdef TJ_OS_WIN
			closesocket(_outSocket);
		#endif	
		
		_direction = (Direction)(_direction & (~DirectionOutbound));
	}
}

void OSCOverIPConnection::StartOutbound(const tj::np::NetworkAddress& na, unsigned short port, NativeSocket outSocket, bool useSendTo) {
	ThreadLock lock(&_lock);
	StopOutbound();
	
	_toAddress = na;
	_toPort = port;
	_outSocket = outSocket;
	_useSendTo = useSendTo;
	_direction = (Direction)(_direction | DirectionOutbound);
}

void OSCOverIPConnection::StartInbound(NativeSocket in4Socket, NativeSocket in6Socket) {
	ThreadLock lock(&_lock);
	StopInbound();
	
	// Update flags
	_direction = Direction(_direction | DirectionInbound);
	_in4Socket = in4Socket;
	_inSocket = in6Socket;

	// Start listener thread
	_listenerThread = GC::Hold(new SocketListenerThread());
	_listenerThread->AddListener(_inSocket, this);
	_listenerThread->AddListener(_in4Socket, this);
	_listenerThread->Start();
}

void OSCOverIPConnection::OnReceiveMessage(osc::ReceivedMessage rm) {
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

void OSCOverIPConnection::OnReceiveBundle(osc::ReceivedBundle rb) {
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

void OSCOverIPConnection::SetFramingType(const std::wstring& ft) {
	_framing = ft;
}

void OSCOverIPConnection::Send(strong<Message> msg) {
	ThreadLock lock(&_lock);
	
	if((_direction & DirectionOutbound)==0) {
		return;
	}
	
	if(_outSocket==-1) {
		Log::Write(L"TJFabric/OSCOverIPConnection", std::wstring(L"Could not send message, outgoing socket is invalid (to-address=")+_toAddress.ToString()+std::wstring(L":")+Stringify(_toPort));
		return;
	}

	// Create the OSC representation of the message
	char buffer[2048];
	char* packetBuffer = buffer;
	bool deletePacketBuffer = false;
	osc::OutboundPacketStream outPacket(&(buffer[0]), 2047);
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
	
	// Framing
	// TODO: use some kind of FramingFactory, so all other connection types can benefit from this
	// also, it would be nice if the resulting Framing object could be cached
	unsigned int packetSize = outPacket.Size();
	if(_framing==L"slip") {
		ref<CodeWriter> cw = GC::Hold(new CodeWriter(4096));
		SLIPFrameDecoder::EncodeSLIPFrame((const unsigned char*)buffer, packetSize, cw);
		packetSize = cw->GetSize();
		packetBuffer = cw->TakeOverBuffer();
		deletePacketBuffer = true;
	}
	
	// Really send the message
	if(_useSendTo) {
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
		
		if(sendto(_outSocket, (const char*)packetBuffer, packetSize, 0, reinterpret_cast<const sockaddr*>(toAddress), toAddressSize)==-1) {
			Log::Write(L"TJFabric/OSCOverIPConnection", L"sendto() failed, error="+Stringify(errno));
		}
		
		wos << L" => " << _toAddress.ToString();
	}
	else {
		if(send(_outSocket, (const char*)packetBuffer, packetSize, 0)==-1) {
			Log::Write(L"TJFabric/OSCOverIPConnection", L"send() failed, error="+Stringify(errno));
		}
	}
	
	if(deletePacketBuffer) {
		delete[] packetBuffer;
	}
	Log::Write(L"TJFabric/OSCOverIPConnection", wos.str());
}

/** OSCOverUDPConnectionDefinition **/
OSCOverUDPConnectionDefinition::OSCOverUDPConnectionDefinition(): OSCOverIPConnectionDefinition(L"udp") {
}

OSCOverUDPConnectionDefinition::~OSCOverUDPConnectionDefinition() {
}

void OSCOverUDPConnectionDefinition::Save(TiXmlElement* me) {
	OSCOverIPConnectionDefinition::Save(me);
}

void OSCOverUDPConnectionDefinition::Load(TiXmlElement* me) {
	OSCOverIPConnectionDefinition::Load(me);
}

/** OSCOverUDPConnection **/
OSCOverUDPConnection::OSCOverUDPConnection() {
}

OSCOverUDPConnection::~OSCOverUDPConnection() {
}

void OSCOverUDPConnection::OnReceive(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	if((GetDirection() & DirectionInbound)!=0) {
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
}

void OSCOverUDPConnection::Create(const std::wstring& address, unsigned short port, Direction direction) {
	ThreadLock lock(&_lock);
	NativeSocket outSocket, inSocket, in4Socket;
	outSocket = inSocket = in4Socket = -1;
	NetworkAddress networkAddress(address);
	
	// Create outgoing socket
	if((direction & DirectionOutbound)!=0) {
		int on = 1;
		outSocket = socket((networkAddress.GetAddressFamily()==AddressFamilyIPv6) ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(outSocket==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not create UDP socket");
		}
		
		setsockopt(outSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(int));
		setsockopt(outSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		Log::Write(L"TJFabric/OSCOverUDPConnection", std::wstring(L"Connected outbound OSC-over-UDP (")+Stringify(address)+L":"+Stringify(port)+L")");
		StartOutbound(networkAddress, port, outSocket, true);
	}
	
	// Create server socket and thread
	if((direction & DirectionInbound)!=0) {
		inSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		in4Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		
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

		#ifdef TJ_OS_POSIX
			addr4.sin_len = sizeof(sockaddr_in);
		#endif
		
		int on = 1;
		setsockopt(inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		setsockopt(in4Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		
		// Bind IPv6 socket
		int err = bind(inSocket, (sockaddr*)&addr, sizeof(addr));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not bind IPv6 server socket, error="+Stringify(errno));
			return;
		}
		
		// Bind IPv4 socket
		err = bind(in4Socket, (sockaddr*)&addr4, sizeof(addr4));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverUDPConnection", L"Could not bind IPv4 server socket, error="+Stringify(errno));
			return;
		}
		
		// try to make us member of the multicast group (IPv6)
		struct ipv6_mreq mreq;
		inet_pton(AF_INET6, Mbs(address).c_str(), &(mreq.ipv6mr_multiaddr));
		mreq.ipv6mr_interface = 0;
		setsockopt(inSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, sizeof(mreq));
		
		// try to make us member of the multicast group (IPv4)
		struct ip_mreq mreq4;
		mreq4.imr_interface.s_addr = INADDR_ANY;
		mreq4.imr_multiaddr.s_addr = addr4.sin_addr.s_addr;
		setsockopt(in4Socket, IPPROTO_IPV4, IP_ADD_MEMBERSHIP, (char*)&mreq4, sizeof(mreq4));
		
		StartInbound(in4Socket, inSocket);
		Log::Write(L"TJFabric/OSCOverUDPConnection", std::wstring(L"Connected inbound OSC-over-UDP (")+Stringify(address)+L":"+Stringify(port)+L")");
	}	
}

void OSCOverUDPConnection::Create(strong<ConnectionDefinition> def, Direction direction, strong<FabricEngine> fe) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverUDPConnectionDefinition>()) {
		ref<OSCOverUDPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			if(cd->_format==L"osc") {
				_def = cd;
				SetFramingType(cd->GetFramingType());
				Create(cd->_address, cd->_port, direction);
			}
			else {
				Throw(L"Invalid format type for UDP connection", ExceptionTypeError);
			}
		}
	}
	else {
		Throw(L"Invalid connection definition type for this connection type", ExceptionTypeError);
	}
}

/** OSCOverTCPConnectionDefinition **/
OSCOverTCPConnectionDefinition::OSCOverTCPConnectionDefinition(): OSCOverIPConnectionDefinition(L"tcp") {
}

OSCOverTCPConnectionDefinition::~OSCOverTCPConnectionDefinition() {
}

void OSCOverTCPConnectionDefinition::Save(TiXmlElement* me) {
	OSCOverIPConnectionDefinition::Save(me);
}

void OSCOverTCPConnectionDefinition::Load(TiXmlElement* me) {
	OSCOverIPConnectionDefinition::Load(me);
}

/** OSCOverTCPConnection **/
OSCOverTCPConnection::OSCOverTCPConnection() {
}

OSCOverTCPConnection::~OSCOverTCPConnection() {
}

void OSCOverTCPConnection::OnReceive(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	if((GetDirection() & DirectionInbound)!=0) {
		if(ns==_in4ServerSocket || ns==_in6ServerSocket) {
			NativeSocket cs = accept(ns, 0, 0);
			if(cs!=-1) {
				_streams[cs] = GC::Hold(new QueueSLIPFrameDecoder());
				AddInboundConnection(cs);
			}
			else {
				Log::Write(L"TJFabric/OSCOverTCPConnection", L"Accept failed: errno="+Stringify(errno));
			}
		}
		else {
			// Process data
			char buffer[4096];
			int r = recv(ns, buffer, 4095, 0);
			std::map<NativeSocket, ref<QueueSLIPFrameDecoder> >::iterator it = _streams.find(ns);
			if(r==0) {
				if(it!=_streams.end()) {
					_streams.erase(it);
				}
				
				RemoveInboundConnection(ns);
			}
			else if(r>0 && it!=_streams.end()) {
				ref<QueueSLIPFrameDecoder> decoder =  it->second;
				if(decoder) {
					decoder->Append((const unsigned char*)buffer, r);
					
					// Process any finished messages
					ref<Code> buffer = decoder->NextPacket();
					while(buffer) {
						osc::ReceivedPacket msg(buffer->GetBuffer(), buffer->GetSize());
						if(msg.IsBundle()) {
							OnReceiveBundle(osc::ReceivedBundle(msg));
						}
						else {
							OnReceiveMessage(osc::ReceivedMessage(msg));
						}
						buffer = decoder->NextPacket();
					}
				}
			}
		}
	}
}

void OSCOverTCPConnection::Create(const std::wstring& address, unsigned short port, Direction direction) {
	ThreadLock lock(&_lock);
	NativeSocket outSocket;
	outSocket = _in4ServerSocket = _in6ServerSocket = -1;
	_streams.clear();
	NetworkAddress networkAddress(address);
	
	// Create outgoing socket
	if((direction & DirectionOutbound)!=0) {
		outSocket = socket((networkAddress.GetAddressFamily()==AddressFamilyIPv6) ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(outSocket==-1) {
			Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not create TCP socket");
		}
		else {
			void* toAddress = 0;
			unsigned int toAddressSize = 0;
			sockaddr_in6 addr6;
			sockaddr_in addr4;
			
			if(networkAddress.GetAddressFamily()==AddressFamilyIPv6) {
				networkAddress.GetIPv6SocketAddress(&addr6);
				addr6.sin6_port = htons(port);
				toAddress = reinterpret_cast<void*>(&addr6);
				toAddressSize = sizeof(sockaddr_in6);
			}
			else if(networkAddress.GetAddressFamily()==AddressFamilyIPv4) {
				networkAddress.GetIPv4SocketAddress(&addr4);
				addr4.sin_port = htons(port);
				toAddress = reinterpret_cast<void*>(&addr4);
				toAddressSize = sizeof(sockaddr_in);
			}
			else {
				Log::Write(L"TJFabric/OSCOverTCPConnection", L"Unsupported address family!");
			}
			
			if(connect(outSocket, (const sockaddr*)toAddress, toAddressSize)!=0) {
				Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not connect TCP socket; error="+Stringify(errno));
				#ifdef TJ_OS_POSIX
					close(outSocket);
				#endif
				
				#ifdef TJ_OS_WIN
					closesocket(outSocket);
				#endif
			}
			else {
				StartOutbound(networkAddress, port, outSocket, false);
				Log::Write(L"TJFabric/OSCOverTCPConnection", std::wstring(L"Connected outbound OSC-over-TCP (")+Stringify(address)+L":"+Stringify(port)+L")");
			}
		}
	}
	
	// Create server socket and thread
	if((direction & DirectionInbound)!=0) {
		_in6ServerSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		_in4ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
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
		
		#ifdef TJ_OS_POSIX
			addr4.sin_len = sizeof(sockaddr_in);
		#endif
		
		// Bind IPv6 socket
		int err = bind(_in6ServerSocket, (sockaddr*)&addr, sizeof(addr));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not bind IPv6 server socket, error="+Stringify(errno));
		}
		
		// Bind IPv4 socket
		err = bind(_in4ServerSocket, (sockaddr*)&addr4, sizeof(addr4));
		if(err==-1) {
			Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not bind IPv4 server socket, error="+Stringify(errno));
		}
		
		// Listen IPv4 socket
		err = listen(_in4ServerSocket, 10);
		if(err!=0) {
			Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not listen IPv4 server socket, error="+Stringify(errno));
		}
		
		// Listen IPv6 socket
		err = listen(_in6ServerSocket, 10);
		if(err!=0) {
			Log::Write(L"TJFabric/OSCOverTCPConnection", L"Could not listen IPv6 server socket, error="+Stringify(errno));
		}
		
		StartInbound(_in4ServerSocket, _in6ServerSocket);
		Log::Write(L"TJFabric/OSCOverTCPConnection", std::wstring(L"Connected inbound OSC-over-TCP (")+Stringify(address)+L":"+Stringify(port)+L")");
	}
}

void OSCOverTCPConnection::Create(strong<ConnectionDefinition> def, Direction direction, strong<FabricEngine> fe) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverTCPConnectionDefinition>()) {
		ref<OSCOverTCPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			if(cd->_format==L"osc") {
				_def = cd;
				SetFramingType(cd->GetFramingType());
				Create(cd->_address, cd->_port, direction);
			}
			else {
				Throw(L"Invalid format type for TCP connection", ExceptionTypeError);
			}
		}
	}
	else {
		Throw(L"Invalid connection definition type for this connection type", ExceptionTypeError);
	}
}