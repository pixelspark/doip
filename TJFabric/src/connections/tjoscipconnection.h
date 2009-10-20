#ifndef _TJ_OSCUDP_CONNECTION_H
#define _TJ_OSCUDP_CONNECTION_H

#include "../../../TJShared/include/tjshared.h"
#include "../../include/tjfabricconnection.h"
#include "../../include/tjfabricgroup.h"
#include "../../../TJNP/include/tjsocketutil.h"
#include "../../../TJNP/include/tjnetworkaddress.h"
#include "../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include "../../../TJScout/include/tjservice.h"
#include "../../../TJNP/include/tjslip.h"

#ifdef TJ_OS_POSIX
	#include <arpa/inet.h>
#endif

#ifdef TJ_OS_WIN
	#include <Winsock2.h>
#endif

namespace tj {
	namespace fabric {
		namespace connections {
			class OSCOverIPConnectionDefinition: public tj::fabric::ConnectionDefinition {
				public:
					virtual ~OSCOverIPConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
					virtual std::wstring GetFramingType() const;
				
				protected:
					OSCOverIPConnectionDefinition(const tj::shared::String& upperProtocol);
					std::wstring _format;
					std::wstring _framing;
					std::wstring _address;
					unsigned short _port;
			};
			
			class OSCOverIPConnection: public Connection, public tj::np::SocketListener {
				public:
					virtual ~OSCOverIPConnection();
					virtual void OnReceive(tj::np::NativeSocket ns) = 0;
					virtual void OnReceiveBundle(osc::ReceivedBundle rb);
					virtual void OnReceiveMessage(osc::ReceivedMessage rb);
					virtual void Send(tj::shared::strong< Message > msg);
					
				protected:
					OSCOverIPConnection();
					virtual void StartInbound(tj::np::NativeSocket in4Socket, tj::np::NativeSocket in6Socket);
					virtual void StartOutbound(const tj::np::NetworkAddress& na, unsigned short port, tj::np::NativeSocket outSocket, bool useSendTo);
					virtual void StopOutbound();
					virtual void StopInbound();
					virtual Direction GetDirection() const;
					virtual void SetFramingType(const std::wstring& ft);
					virtual void AddInboundConnection(tj::np::NativeSocket ns);
					virtual void RemoveInboundConnection(tj::np::NativeSocket ns);
				
					tj::shared::CriticalSection _lock;
				
				private:
					tj::np::NativeSocket _outSocket;
					tj::np::NativeSocket _inSocket;
					tj::np::NativeSocket _in4Socket;
					std::deque<tj::np::NativeSocket> _additionalIncomingSockets;
					tj::shared::ref< tj::np::SocketListenerThread> _listenerThread;
					Direction _direction;
					tj::np::NetworkAddress _toAddress;
					unsigned short _toPort;
					bool _useSendTo;
					std::wstring _framing;
			};
			
			/** OSC-over-TCP/IP with SLIP framing **/
			class OSCOverTCPConnection;
			
			class OSCOverTCPConnectionDefinition: public OSCOverIPConnectionDefinition {
				friend class OSCOverTCPConnection;
				
				public:
					OSCOverTCPConnectionDefinition();
					virtual ~OSCOverTCPConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
			};
			
			class OSCOverTCPConnection: public OSCOverIPConnection {
				public:
					OSCOverTCPConnection();
					virtual ~OSCOverTCPConnection();
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d);
					virtual void Create(const std::wstring& address, unsigned short port, Direction d);
					virtual void OnReceive(tj::np::NativeSocket ns);
				
				protected:
					tj::shared::ref<OSCOverTCPConnectionDefinition> _def;
					tj::shared::ref< tj::scout::ServiceRegistration > _serviceRegistration;
					tj::np::NativeSocket _in4ServerSocket;
					tj::np::NativeSocket _in6ServerSocket;
					std::map<tj::np::NativeSocket, tj::shared::ref<tj::np::QueueSLIPFrameDecoder> > _streams;
			};
			
			/** OSC-over-UDP/IP **/
			class OSCOverUDPConnection;
			
			class OSCOverUDPConnectionDefinition: public OSCOverIPConnectionDefinition {
				friend class OSCOverUDPConnection;
				
				public:
					OSCOverUDPConnectionDefinition();
					virtual ~OSCOverUDPConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
			};
			
			class OSCOverUDPConnection: public OSCOverIPConnection {
				public:
					OSCOverUDPConnection();
					virtual ~OSCOverUDPConnection();
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d);
					virtual void Create(const std::wstring& address, unsigned short port, Direction d);
					virtual void OnReceive(tj::np::NativeSocket ns);
					
				protected:
					tj::shared::ref<OSCOverUDPConnectionDefinition> _def;
					tj::shared::ref< tj::scout::ServiceRegistration > _serviceRegistration;
			};			
							
		}
	}
}

#endif