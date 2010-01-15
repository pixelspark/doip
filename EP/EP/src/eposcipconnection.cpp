#include "../include/eposcipconnection.h"
#include "../include/epmessage.h"

#include <OSCPack/OscOutboundPacketStream.h>
#include <OSCPack/OscReceivedElements.h>
#include <OSCPack/OscPacketListener.h>
#include <OSCPack/OscPrintReceivedElements.h>

#include <limits>
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
using namespace tj::np;
using namespace tj::ep;

class OSCOverIPConnectionChannel: public ConnectionChannel {
	public:
		OSCOverIPConnectionChannel(NativeSocket ns): _socket(ns) {
		}

		virtual ~OSCOverIPConnectionChannel() {
		}

		NativeSocket _socket;
};

/** OSCUtil **/
class OSCUtil {
	public:
		static bool ArgumentToAny(const osc::ReceivedMessageArgument& arg, tj::shared::Any& any);
};

bool OSCUtil::ArgumentToAny(const osc::ReceivedMessageArgument& arg, tj::shared::Any& any) {
	const char type = arg.TypeTag();
	switch(type) {
		case osc::TRUE_TYPE_TAG:
			any = Any(true);
			return true;
			
		case osc::FALSE_TYPE_TAG:
			any = Any(false);
			return true;
			
		case osc::NIL_TYPE_TAG:
			any = Any();
			return true;
			
		case osc::INFINITUM_TYPE_TAG:
			any = Any(std::numeric_limits<double>::infinity());
			return true;
			
		case osc::INT32_TYPE_TAG:
			any = Any((int)arg.AsInt32());
			return true;
			
		case osc::FLOAT_TYPE_TAG:
			any = Any((float)arg.AsFloat());
			return true;
			
		case osc::CHAR_TYPE_TAG:
			any = Any(Stringify(arg.AsChar()));
			return true;
			
		case osc::RGBA_COLOR_TYPE_TAG: {
			strong<Tuple> data = GC::Hold(new Tuple(4));
			unsigned int color = arg.AsRgbaColor();
			
			data->Set(0, Any(double(int((color >> 24) & 0xFF)) / 255.0));
			data->Set(1, Any(double(int((color >> 16) & 0xFF)) / 255.0));
			data->Set(2, Any(double(int((color >> 8) & 0xFF)) / 255.0));
			data->Set(3, Any(double(int(color & 0xFF)) / 255.0));
			any = Any(data);
			return true;								
		}
			
		case osc::MIDI_MESSAGE_TYPE_TAG: {
			strong<Tuple> data = GC::Hold(new Tuple(4));
			unsigned int msg = arg.AsMidiMessage();
			
			data->Set(0, Any(double(int((msg >> 24) & 0xFF)) / 255.0));
			data->Set(1, Any(double(int((msg >> 16) & 0xFF)) / 255.0));
			data->Set(2, Any(double(int((msg >> 8) & 0xFF)) / 255.0));
			data->Set(3, Any(double(int(msg & 0xFF)) / 255.0));
			any = Any(data);
			return true;											  
		}
			
		case osc::DOUBLE_TYPE_TAG:
			any = Any(arg.AsDouble());
			return true;
			
		case osc::STRING_TYPE_TAG:
			any = Any(Wcs(std::string(arg.AsString())));
			return true;
			
		case osc::SYMBOL_TYPE_TAG:
			any = Any(Wcs(std::string(arg.AsSymbol())));
			return true;
			
		case osc::BLOB_TYPE_TAG:
			// TODO: implement this in some way... We cannot convert blob data to a string,
			// since it can contain \0 characters. It is thus not very suited for Any...
			
		case osc::INT64_TYPE_TAG:
			// TODO: add Any::TypeInt64 or something like that...
			
		case osc::TIME_TAG_TYPE_TAG:
			// TODO: conversion to Time type (and addition of Any::TypeTime?)
			
		default:
			return false;
	};
}

/** OSCOverIPConnectionDefinition **/
OSCOverIPConnectionDefinition::OSCOverIPConnectionDefinition(const String& upperProtocol): ConnectionDefinition(upperProtocol), _format(L"osc") {
} 

OSCOverIPConnectionDefinition::~OSCOverIPConnectionDefinition() {
}

void OSCOverIPConnectionDefinition::SetAddress(const std::wstring& a) {
	_address = a;
}

void OSCOverIPConnectionDefinition::SetFormat(const std::wstring& f) {
	_format = f;
}

void OSCOverIPConnectionDefinition::SetFraming(const std::wstring& f) {
	_framing = f;
}

void OSCOverIPConnectionDefinition::SetPort(const unsigned short port) {
	_port = port;
}

tj::shared::String OSCOverIPConnectionDefinition::GetAddress() const {
	return _address;
}

tj::shared::String OSCOverIPConnectionDefinition::GetFormat() const {
	return _format;
}

tj::shared::String OSCOverIPConnectionDefinition::GetFraming() const {
	return _framing;
}

unsigned short OSCOverIPConnectionDefinition::GetPort() const {
	return _port;
}

void OSCOverIPConnectionDefinition::SaveConnection(TiXmlElement* me) {
	SaveAttributeSmall<std::wstring>(me, "address", _address);
	SaveAttributeSmall<int>(me, "port", int(_port));
	SaveAttributeSmall<std::wstring>(me, "format", _format);
	SaveAttributeSmall<std::wstring>(me, "framing", _framing);
}

void OSCOverIPConnectionDefinition::LoadConnection(TiXmlElement* me) {
	_address = LoadAttributeSmall<std::wstring>(me, "address", _address);
	_port = (unsigned short)LoadAttributeSmall<int>(me, "port", (int)_port);
	_format = LoadAttributeSmall<std::wstring>(me, "format", _format);	
	_framing = LoadAttributeSmall<std::wstring>(me,"framing", _framing);
}

/** OSCOverIPConnection **/
OSCOverIPConnection::OSCOverIPConnection(): _inPort(0), _useSendTo(false), _handlingReplies(false), _outSocket(-1), _inSocket(-1), _toPort(0), _toAddress(L""), _direction(DirectionNone) {
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
		_listenerThread->RemoveListener(_inSocket);
		_listenerThread = null;
		_handlingReplies = false;
		_inPort = 0;
		_listenerThread = null; // This will call ~SocketListenerThread => Thread::WaitForCompletion on the thread
		
		#ifdef TJ_OS_POSIX
			close(_inSocket);
		#endif
		
		#ifdef TJ_OS_WIN
			closesocket(_inSocket);
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

bool OSCOverIPConnection::IsHandlingReplies() const {
	return _handlingReplies;
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

void OSCOverIPConnection::StartInboundReplies(NativeSocket outSocket) {
	ThreadLock lock(&_lock);
	StopInbound();
	
	// Update flags
	_direction = Direction(_direction | DirectionInbound);
	_inSocket = outSocket;
	_handlingReplies = true;
	
	// Start listener thread
	_listenerThread = SocketListenerThread::DefaultInstance();
	_listenerThread->AddListener(outSocket, this);
}

void OSCOverIPConnection::StartInbound(NativeSocket inSocket, bool handleReplies) {
	ThreadLock lock(&_lock);
	StopInbound();
	
	// Get port number
	sockaddr_in6 address;
	socklen_t len = sizeof(sockaddr_in6);
	memset(&address, 0, sizeof(sockaddr_in6));
	if(getsockname(inSocket, (sockaddr*)&address, &len)==0) {
		_inPort = ntohs(address.sin6_port);
		Log::Write(L"EPFramework/OSCOverIPConnection", L"Inbound IP server chose port number: "+Stringify(_inPort));
	}  
	
	// Update flags
	_direction = Direction(_direction | DirectionInbound);
	_inSocket = inSocket;
	
	
	// Start listener thread or use the default one
	_listenerThread = SocketListenerThread::DefaultInstance();
	_listenerThread->AddListener(inSocket, this);
	if(handleReplies) {
		_listenerThread->AddListener(_outSocket, this);
		_handlingReplies = true;
	}
}

void OSCOverIPConnection::OnReceiveMessage(osc::ReceivedMessage rm, bool isReply, bool endReply, NativeSocket ns) {
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
	
	if(isReply) {
		ThreadLock lock(&_lock);
		std::deque< std::pair< ref<ReplyHandler>, ref<Message> > >::iterator it = _replyQueue.begin();
		if(it!=_replyQueue.end()) {
			std::pair< ref<ReplyHandler>, ref<Message> >& data = *it;
			if(data.first && data.second) {
				data.first->OnReceiveReply(data.second, msg, ref<Connection>(ref<OSCOverIPConnection>(this)), GC::Hold(new OSCOverIPConnectionChannel(ns)));
			}
			else {
				// Nil reply handler; ignore
			}
		}
		
		if(endReply) {
			PopReplyHandler();
		}
	}
	else {
		EventMessageReceived.Fire(this, MessageNotification(Timestamp(true), msg, this, GC::Hold(new OSCOverIPConnectionChannel(ns))));
	}
}

unsigned short OSCOverIPConnection::GetInboundPort() const {
	return _inPort;
}

void OSCOverIPConnection::OnReceiveBundle(osc::ReceivedBundle rb, bool isReply, bool endReply, NativeSocket ns) {
	ThreadLock lock(&_lock);
	osc::ReceivedBundle::const_iterator it = rb.ElementsBegin(); 
	while(it!=rb.ElementsEnd()) {
		if(it->IsBundle()) {
			OnReceiveBundle(osc::ReceivedBundle(*it), isReply, false, ns);
		}
		else {
			OnReceiveMessage(osc::ReceivedMessage(*it), isReply, false, ns);
		}
		++it;
	}

	if(endReply) {
		PopReplyHandler();
	}
}

void OSCOverIPConnection::PopReplyHandler() {
	ThreadLock lock(&_lock);
	std::deque< std::pair< ref<ReplyHandler>, ref<Message> > >::iterator it = _replyQueue.begin();
	if(it!=_replyQueue.end()) {
		{
			std::pair< ref<ReplyHandler>, ref<Message> >& data = *it;
			if(data.first && data.second) {
				data.first->OnEndReply(data.second);
			}
			else {
				// 'nil' handler; reply is expected, but client doesn't care about it
			}
		}
		_replyQueue.pop_front();
	}
	else {
		Log::Write(L"EPFramework/OSCOverIPConnection", L"A reply to a message was received, but no handler is installed to handle it");
	}
}

void OSCOverIPConnection::SetFramingType(const std::wstring& ft) {
	_framing = ft;
}

void OSCOverIPConnection::Send(strong<Message> msg, ref<ReplyHandler> rh, ref<ConnectionChannel> cc) {
	ThreadLock lock(&_lock);

	/** If the client supplies a specific channel (=socket) to send to, use it; if not, use the outgoing socket if 
	we are an outbound connection; otherwise, ignore **/
	NativeSocket outSocket = _outSocket;
	if(cc && cc.IsCastableTo<OSCOverIPConnectionChannel>()) {
		outSocket = ref<OSCOverIPConnectionChannel>(cc)->_socket;
	}
	else if((_direction & DirectionOutbound)==0) {
		return;
	}

	if(outSocket==-1) {
		Log::Write(L"EPFramework/OSCOverIPConnection", std::wstring(L"Could not send message, outgoing socket is invalid (to-address=")+_toAddress.ToString()+std::wstring(L":")+Stringify(_toPort));
		return;
	}
	
	// Create the OSC representation of the message
	bool isDebug = Zones::IsDebug();
	char buffer[2048];
	char* packetBuffer = buffer;
	bool deletePacketBuffer = false;
	osc::OutboundPacketStream outPacket(&(buffer[0]), 2047);
	outPacket << osc::BeginMessage(Mbs(msg->GetPath()).c_str());
	std::wostringstream wos;
	if(isDebug) {
		wos << StringifyHex(outSocket) << L"<= " << msg->GetPath() << L",";
	}
	
	for(unsigned int a=0;a<msg->GetParameterCount();a++) {
		Any value = msg->GetParameter(a);
		switch(value.GetType()) {
			case Any::TypeBool:
				outPacket << (bool)value;
				if(isDebug) {
					wos << L'b';
				}
				break;
				
			case Any::TypeDouble:
				outPacket << (double)value;
				if(isDebug) {
					wos << L'd';
				}
				break;
				
			case Any::TypeInteger:
				outPacket << (osc::int32)(int)value;
				if(isDebug) {
					wos << L'i';
				}
				break;
				
			case Any::TypeString:
				outPacket << Mbs(value.ToString()).c_str();
				if(isDebug) {
					wos << L's';
				}
				break;
				
			default:
			case Any::TypeObject:
			case Any::TypeTuple:
			case Any::TypeNull:
				outPacket << osc::Nil;
				if(isDebug) {
					wos << L'0';
				}
				break;
		};
	}
	outPacket << osc::EndMessage;
	
	// Framing
	// TODO: use some kind of FramingFactory, so all other connection types can benefit from this
	// also, it would be nice if the resulting Framing object could be cached
	Bytes packetSize = outPacket.Size();
	if(_framing==L"slip") {
		ref<DataWriter> cw = GC::Hold(new DataWriter(4096));
		SLIPFrameDecoder::EncodeSLIPFrame((const unsigned char*)buffer, (Bytes)packetSize, cw);
		packetSize = cw->GetSize();
		packetBuffer = cw->TakeOverBuffer(true);
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
		
		if(sendto(outSocket, (const char*)packetBuffer, (int)packetSize, 0, reinterpret_cast<const sockaddr*>(toAddress), toAddressSize)==-1) {
			Log::Write(L"EPFramework/OSCOverIPConnection", L"sendto() failed, error="+Util::GetDescriptionOfSystemError(errno));
		}
		
		if(isDebug) {
			wos << L" => " << _toAddress.ToString() << L":" << _toPort;
		}
	}
	else {
		if(send(outSocket, (const char*)packetBuffer, (int)packetSize, 0)==-1) {
			Log::Write(L"EPFramework/OSCOverIPConnection", L"send() failed, error="+Stringify(errno));
		}
	}
	
	if(deletePacketBuffer) {
		delete[] packetBuffer;
	}
	
	if(isDebug) {
		Log::Write(L"EPFramework/OSCOverIPConnection", wos.str());
	}

	// Register reply handler
	if(IsHandlingReplies()) {
		if(rh) {
			_replyQueue.push_back(std::pair< ref<ReplyHandler>, ref<Message> >(rh, msg));
		}
		else {
			Log::Write(L"EPFramework/OSCOverIPConnection", L"Handling replies, but no reply handler given; inserting nil handler!");
			_replyQueue.push_back(std::pair< ref<ReplyHandler>, ref<Message> >(null,null));
		}
	}
	else {
		if(rh) {
			Log::Write(L"EPFramework/OSCIPConnection", L"A reply handler was given for a sent message, but this connection is not handling replies");
		}
	}
}

/** OSCOverUDPConnectionDefinition **/
OSCOverUDPConnectionDefinition::OSCOverUDPConnectionDefinition(): OSCOverIPConnectionDefinition(L"udp") {
}

OSCOverUDPConnectionDefinition::~OSCOverUDPConnectionDefinition() {
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
			OnReceiveBundle(osc::ReceivedBundle(msg), false, false, ns);
		}
		else {
			OnReceiveMessage(osc::ReceivedMessage(msg), false, false, ns);
		}
	}
}

void OSCOverUDPConnection::Create(const tj::np::NetworkAddress& address, unsigned short port, Direction direction) {
	ThreadLock lock(&_lock);
	NativeSocket outSocket, inSocket;
	outSocket = inSocket = -1;
	NetworkAddress networkAddress(address);
	
	// Create outgoing socket
	if((direction & DirectionOutbound)!=0) {
		int on = 1;
		outSocket = socket((networkAddress.GetAddressFamily()==AddressFamilyIPv6) ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(outSocket==-1) {
			Log::Write(L"EPFramework/OSCOverUDPConnection", L"Could not create UDP socket");
		}
		
		setsockopt(outSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(int));
		setsockopt(outSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
		Log::Write(L"EPFramework/OSCOverUDPConnection", std::wstring(L"Connected outbound OSC-over-UDP (")+address.ToString()+L":"+Stringify(port)+L")");
		StartOutbound(networkAddress, port, outSocket, true);
	}
	
	// Create server socket and thread
	if((direction & DirectionInbound)!=0) {
		if(networkAddress.GetAddressFamily()==AddressFamilyIPv6) {
			inSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
			
			// Fill in the interface information for the IPv6 listener socket
			in6_addr any = IN6ADDR_ANY_INIT;
			sockaddr_in6 addr;
			addr.sin6_family = AF_INET6;
			addr.sin6_port = htons(port);
			addr.sin6_addr = any;
			
			int on = 1;
			setsockopt(inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
			
			// Bind IPv6 socket
			int err = bind(inSocket, (sockaddr*)&addr, sizeof(addr));
			if(err==-1) {
				Log::Write(L"EPFramework/OSCOverUDPConnection", L"Could not bind IPv6 server socket, error="+Stringify(errno));
				return;
			}
			
			// try to make us member of the multicast group (IPv6)
			struct sockaddr_in6 maddr;
			struct ipv6_mreq mreq;
			if(networkAddress.GetIPv6SocketAddress(&maddr)) {
				mreq.ipv6mr_multiaddr = maddr.sin6_addr;
				mreq.ipv6mr_interface = 0;
				setsockopt(inSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, sizeof(mreq));
			}
		}
		else if(networkAddress.GetAddressFamily()==AddressFamilyIPv4) {
			inSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			
			// Addresses for the IPv4 listener socket
			sockaddr_in addr4;
			memset(&addr4, 0, sizeof(addr4));
			addr4.sin_family = AF_INET;
			addr4.sin_port = htons(port);
			addr4.sin_addr.s_addr = INADDR_ANY;
			
			int on = 1;
			setsockopt(inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
			
			// Bind IPv4 socket
			int err = bind(inSocket, (sockaddr*)&addr4, sizeof(addr4));
			if(err==-1) {
				Log::Write(L"EPFramework/OSCOverUDPConnection", L"Could not bind IPv4 server socket, error="+Stringify(errno));
				return;
			}
			
			// try to make us member of the multicast group (IPv4)
			struct ip_mreq mreq4;
			struct sockaddr_in maddr;
			mreq4.imr_interface.s_addr = INADDR_ANY;
			mreq4.imr_multiaddr.s_addr = addr4.sin_addr.s_addr;
			if(networkAddress.GetIPv4SocketAddress(&maddr)) {
				mreq4.imr_multiaddr = maddr.sin_addr;
				setsockopt(inSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq4, sizeof(mreq4));
			}
		}
		else {
			Throw(L"Unsupported address family", ExceptionTypeError);
		}
		
		StartInbound(inSocket, false);
		Log::Write(L"EPFramework/OSCOverUDPConnection", std::wstring(L"Connected inbound OSC-over-UDP (")+address.ToString()+L":"+Stringify(port)+L")");
	}	
}

void OSCOverUDPConnection::CreateForTransport(strong<EPTransport> ept, const NetworkAddress& address) {
	if(ept->GetType()!=L"udp") {
		Throw(L"Cannot create connection to transport of this type", ExceptionTypeError);
	}
	if(ept->GetFormat()!=L"osc") {
		Throw(L"Cannot create connection to transport: format unsupported", ExceptionTypeError);
	}
	
	SetFramingType(ept->GetFraming());
	std::wstring host = ept->GetAddress();
	Create((host.length()==0) ? address : NetworkAddress(host,true), ept->GetPort(), DirectionOutbound);
}

void OSCOverUDPConnection::Create(strong<ConnectionDefinition> def, Direction direction, ref<EPEndpoint> parent) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverUDPConnectionDefinition>()) {
		ref<OSCOverUDPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			if(cd->_format==L"osc") {
				_def = cd;
				SetFramingType(cd->GetFraming());
				Create(NetworkAddress(cd->_address, true), cd->_port, direction);
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

/** OSCOverTCPConnection **/
OSCOverTCPConnection::OSCOverTCPConnection() {
}

OSCOverTCPConnection::~OSCOverTCPConnection() {
}

void OSCOverTCPConnection::OnReceive(NativeSocket ns) {
	ThreadLock lock(&_lock);
	
	if((GetDirection() & DirectionInbound)!=0) {
		if(ns==_inServerSocket) {
			NativeSocket cs = accept(ns, 0, 0);
			if(cs!=-1) {
				_streams[cs] = GC::Hold(new QueueSLIPFrameDecoder());
				AddInboundConnection(cs);
			}
			else {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Accept failed: errno="+Stringify(errno));
			}
		}
		else {
			// Process data
			bool isReply = (IsHandlingReplies() && ns==_outSocket);
			if(isReply) {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Receiving reply data");
			}

			char buffer[4096];
			int r = recv(ns, buffer, 4095, 0);

			ref<QueueSLIPFrameDecoder> decoder = null;
			if(isReply) {
				if(!_replyStream) {
					_replyStream = GC::Hold(new QueueSLIPFrameDecoder());
				}
				decoder = _replyStream;
			}
			else {
				std::map<NativeSocket, ref<QueueSLIPFrameDecoder> >::iterator it = _streams.find(ns);
				if(r==0) {
					if(it!=_streams.end()) {
						_streams.erase(it);
					}
					RemoveInboundConnection(ns);
				}
				else {
					if(it!=_streams.end()) {
						decoder = it->second;
					}
				}
			}
			
			if(r>0 && decoder) {
				decoder->Append((const unsigned char*)buffer, r);
					
				// Process any finished messages
				ref<DataReader> buffer = decoder->NextPacket();
				while(buffer) {
					osc::ReceivedPacket msg(buffer->GetBuffer(), (unsigned int)buffer->GetSize());
					if(msg.IsBundle()) {
						OnReceiveBundle(osc::ReceivedBundle(msg), isReply, isReply, ns);
					}
					else {
						OnReceiveMessage(osc::ReceivedMessage(msg), isReply, isReply, ns);
					}
					buffer = decoder->NextPacket();
				}
			}
		}
	}
}

void OSCOverTCPConnection::CreateForTransport(strong<EPTransport> ept, const tj::np::NetworkAddress& address) {
	if(ept->GetType()!=L"tcp") {
		Throw(L"Cannot create connection to transport of this type", ExceptionTypeError);
	}
	if(ept->GetFormat()!=L"osc") {
		Throw(L"Cannot create connection to transport: format unsupported", ExceptionTypeError);
	}
	
	SetFramingType(ept->GetFraming());
	std::wstring host = ept->GetAddress();
	Create((host.length()==0) ? address : NetworkAddress(host,true), ept->GetPort(), DirectionOutbound);
}


void OSCOverTCPConnection::Create(const NetworkAddress& networkAddress, unsigned short port, Direction direction) {
	ThreadLock lock(&_lock);
	NativeSocket outSocket;
	outSocket = _inServerSocket = -1;
	_streams.clear();

	/** When direction = OUT|IN, the connection will be outbound, but listening for incoming replies. When the direction
	is IN, the connection will be a TCP-server. When direction=OUT, the connection will be outbound-only **/
	if((direction & DirectionOutbound)!=0) {
		outSocket = socket((networkAddress.GetAddressFamily()==AddressFamilyIPv6) ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(outSocket==-1) {
			Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not create TCP socket; error="+Util::GetDescriptionOfSystemError(errno));
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
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Unsupported address family!");
			}
			
			if(connect(outSocket, (const sockaddr*)toAddress, toAddressSize)!=0) {
				#ifdef TJ_OS_POSIX
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not connect TCP socket; error="+Util::GetDescriptionOfSystemError(errno));
					close(outSocket);
				#endif
				
				#ifdef TJ_OS_WIN
					Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not connect TCP socket; error="+Stringify(WSAGetLastError()));
					closesocket(outSocket);
				#endif
			}
			else {
				StartOutbound(networkAddress, port, outSocket, false);
				Log::Write(L"EPFramework/OSCOverTCPConnection", std::wstring(L"Connected outbound OSC-over-TCP (")+networkAddress.ToString()+L":"+Stringify(port)+L")");

				if((direction & DirectionInbound)!=0) {
					// Also listen for replies
					StartInboundReplies(outSocket);
					Log::Write(L"EPFramework/OSCOverTCPConnection",L"Will also listen for replies");
				}
			}
		}
	}
	
	// Create server socket and thread
	if(direction == DirectionInbound) {
		if(networkAddress.GetAddressFamily()==AddressFamilyIPv6) {
			_inServerSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
			
			// Fill in the interface information for the IPv6 listener socket
			in6_addr any = IN6ADDR_ANY_INIT;
			sockaddr_in6 addr;
			addr.sin6_family = AF_INET6;
			addr.sin6_port = htons(port);
			addr.sin6_addr = any;
			
			// Bind IPv6 socket
			int err = bind(_inServerSocket, (sockaddr*)&addr, sizeof(addr));
			if(err==-1) {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not bind IPv6 server socket, error="+Util::GetDescriptionOfSystemError(errno));
			}
			
			// Listen IPv6 socket
			err = listen(_inServerSocket, 10);
			if(err!=0) {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not listen IPv6 server socket, error="+Util::GetDescriptionOfSystemError(errno));
			}
		}
		else if(networkAddress.GetAddressFamily()==AddressFamilyIPv4) {
			_inServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			
			// Addresses for the IPv4 listener socket
			sockaddr_in addr4;
			memset(&addr4, 0, sizeof(addr4));
			addr4.sin_family = AF_INET;
			addr4.sin_port = htons(port);
			addr4.sin_addr.s_addr = INADDR_ANY;
			
			// Bind IPv4 socket
			int err = bind(_inServerSocket, (sockaddr*)&addr4, sizeof(addr4));
			if(err==-1) {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not bind IPv4 server socket, error="+Util::GetDescriptionOfSystemError(errno));
			}
			
			// Listen IPv4 socket
			err = listen(_inServerSocket, 10);
			if(err!=0) {
				Log::Write(L"EPFramework/OSCOverTCPConnection", L"Could not listen IPv4 server socket, error="+Util::GetDescriptionOfSystemError(errno));
			}
		}
		else {
			Throw(L"Unsupported address family", ExceptionTypeError);
		}
		
		StartInbound(_inServerSocket, (direction & DirectionOutbound)!=0);
		Log::Write(L"EPFramework/OSCOverTCPConnection", std::wstring(L"Connected inbound OSC-over-TCP (")+networkAddress.ToString()+L":"+Stringify(port)+L")");
	}
}

void OSCOverTCPConnection::Create(strong<ConnectionDefinition> def, Direction direction, ref<EPEndpoint> parent) {
	if(ref<ConnectionDefinition>(def).IsCastableTo<OSCOverTCPConnectionDefinition>()) {
		ref<OSCOverTCPConnectionDefinition> cd = ref<ConnectionDefinition>(def);
		if(cd) {
			if(cd->_format==L"osc") {
				_def = cd;
				SetFramingType(cd->GetFraming());
				Create(NetworkAddress(cd->_address,true), cd->_port, direction);
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
