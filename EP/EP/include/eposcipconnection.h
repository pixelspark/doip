/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 /* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_EP_OSCIP_CONNECTION_H
#define _TJ_EP_OSCIP_CONNECTION_H

#include <TJShared/include/tjshared.h>
#include <TJNP/include/tjsocket.h>
#include <TJNP/include/tjnetworkaddress.h>
#include <TJScout/include/tjservice.h>
#include <TJNP/include/tjslip.h>

#include "epinternal.h"
#include "epconnection.h"

namespace osc {
	class ReceivedBundle;
	class ReceivedMessage;
}

namespace tj {
	namespace ep {
		class EP_EXPORTED OSCOverIPConnectionDefinition: public ConnectionDefinition {
			public:
				virtual ~OSCOverIPConnectionDefinition();
				virtual void SaveConnection(TiXmlElement* me);
				virtual void LoadConnection(TiXmlElement* me);
				virtual tj::shared::String GetAddress() const;
				virtual tj::shared::String GetFormat() const;
				virtual tj::shared::String GetFraming() const;
				virtual unsigned short GetPort() const;
				virtual void SetAddress(const std::wstring& a);
				virtual void SetFormat(const std::wstring& f);
				virtual void SetFraming(const std::wstring& f);
				virtual void SetPort(const unsigned short port);
				
			protected:
				OSCOverIPConnectionDefinition(const tj::shared::String& upperProtocol);
				std::wstring _format;
				std::wstring _framing;
				std::wstring _address;
				unsigned short _port;
		};
		
		class EP_EXPORTED OSCOverIPConnection: public Connection, public tj::np::SocketListener {
			public:
				virtual ~OSCOverIPConnection();
				virtual void OnReceive(tj::np::NativeSocket ns) = 0;
				virtual void OnReceiveBundle(osc::ReceivedBundle rb, bool isReply, bool endReply, tj::np::NativeSocket ns);
				virtual void OnReceiveMessage(osc::ReceivedMessage rb, bool isReply, bool endReply, tj::np::NativeSocket ns);
				virtual void Send(tj::shared::strong< Message > msg, tj::shared::ref<ReplyHandler> rh, tj::shared::ref<ConnectionChannel> cc);
				virtual unsigned short GetInboundPort() const;
			
			protected:
				OSCOverIPConnection();
				virtual void StartInbound(tj::np::NativeSocket inSocket, bool handleReplies);
				virtual void StartInboundReplies(tj::np::NativeSocket inSocket);
				virtual void StartOutbound(const tj::np::NetworkAddress& na, unsigned short port, tj::np::NativeSocket outSocket, bool useSendTo);
				virtual void StopOutbound();
				virtual void StopInbound();
				virtual Direction GetDirection() const;
				virtual bool IsHandlingReplies() const;
				virtual void SetFramingType(const std::wstring& ft);
				virtual void AddInboundConnection(tj::np::NativeSocket ns);
				virtual void RemoveInboundConnection(tj::np::NativeSocket ns);
				virtual void PopReplyHandler();
				
				tj::shared::CriticalSection _lock;
				tj::np::NativeSocket _outSocket;
				
			private:
				tj::np::NativeSocket _inSocket;
				std::deque<tj::np::NativeSocket> _additionalIncomingSockets;
				tj::shared::ref< tj::np::SocketListenerThread> _listenerThread;
				Direction _direction;
				tj::np::NetworkAddress _toAddress;
				unsigned short _toPort;
				unsigned short _inPort;
				bool _useSendTo;
				bool _handlingReplies;
				std::wstring _framing;
				std::deque< std::pair< tj::shared::ref<ReplyHandler>, tj::shared::ref<Message> > > _replyQueue;
		};
		
		/** OSC-over-TCP/IP with SLIP framing **/
		class OSCOverTCPConnection;
		
		class EP_EXPORTED OSCOverTCPConnectionDefinition: public OSCOverIPConnectionDefinition {
			friend class OSCOverTCPConnection;
			
			public:
				OSCOverTCPConnectionDefinition();
				virtual ~OSCOverTCPConnectionDefinition();
		};
		
		class EP_EXPORTED OSCOverTCPConnection: public OSCOverIPConnection {
			public:
				OSCOverTCPConnection();
				virtual ~OSCOverTCPConnection();
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<EPEndpoint> parent);
				virtual void Create(const tj::np::NetworkAddress& address, unsigned short port, Direction d);
				virtual void CreateForTransport(tj::shared::strong<tj::ep::EPTransport> ept, const tj::np::NetworkAddress& address);
				virtual void OnReceive(tj::np::NativeSocket ns);
				
			protected:
				tj::shared::ref<OSCOverTCPConnectionDefinition> _def;
				tj::shared::ref< tj::scout::ServiceRegistration > _serviceRegistration;
				tj::np::NativeSocket _inServerSocket;
				tj::shared::ref<tj::np::QueueSLIPFrameDecoder> _replyStream;
				std::map<tj::np::NativeSocket, tj::shared::ref<tj::np::QueueSLIPFrameDecoder> > _streams;
		};
		
		/** OSC-over-UDP/IP **/
		class OSCOverUDPConnection;
		
		class EP_EXPORTED OSCOverUDPConnectionDefinition: public OSCOverIPConnectionDefinition {
			friend class OSCOverUDPConnection;
			
			public:
				OSCOverUDPConnectionDefinition();
				virtual ~OSCOverUDPConnectionDefinition();
		};
		
		class EP_EXPORTED OSCOverUDPConnection: public OSCOverIPConnection {
			public:
				OSCOverUDPConnection();
				virtual ~OSCOverUDPConnection();
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<EPEndpoint> parent);
				virtual void Create(const tj::np::NetworkAddress& address, unsigned short port, Direction d);
				virtual void CreateForTransport(tj::shared::strong<tj::ep::EPTransport> ept, const tj::np::NetworkAddress& address);
				virtual void OnReceive(tj::np::NativeSocket ns);
				
			protected:
				tj::shared::ref<OSCOverUDPConnectionDefinition> _def;
				tj::shared::ref< tj::scout::ServiceRegistration > _serviceRegistration;
		};			
	}
}

#endif