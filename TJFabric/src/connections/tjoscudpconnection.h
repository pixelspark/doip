#ifndef _TJ_OSCUDP_CONNECTION_H
#define _TJ_OSCUDP_CONNECTION_H

#include "../../../TJShared/include/tjshared.h"
#include "../../include/tjfabricconnection.h"
#include "../../include/tjfabricgroup.h"
#include "../../../TJNP/include/tjsocketutil.h"
#include "../../../TJNP/include/tjnetworkaddress.h"
#include "../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include "../../../TJScout/include/tjservice.h"

#ifdef TJ_OS_POSIX
	#include <arpa/inet.h>
#endif

#ifdef TJ_OS_WIN
	#include <Winsock2.h>
#endif

namespace tj {
	namespace fabric {
		namespace connections {
			class OSCOverUDPConnection;
			
			class OSCOverUDPConnectionDefinition: public tj::fabric::ConnectionDefinition {
				friend class OSCOverUDPConnection;
				
				public:
					OSCOverUDPConnectionDefinition();
					virtual ~OSCOverUDPConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
				
				protected:
					std::wstring _address;
					unsigned short _port;
			};
			
			class OSCOverUDPConnection: public Connection, public tj::np::SocketListener {
				public:
					OSCOverUDPConnection();
					virtual ~OSCOverUDPConnection();
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d);
					virtual void Create(const std::wstring& address, unsigned short port, Direction d);
					virtual void Send(tj::shared::strong< Message > msg);
					virtual void OnReceive(tj::np::NativeSocket ns);
					virtual void OnReceiveBundle(osc::ReceivedBundle rb);
					virtual void OnReceiveMessage(osc::ReceivedMessage rb);
				
				protected:
					tj::shared::CriticalSection _lock;
					tj::np::NativeSocket _outSocket;
					tj::np::NativeSocket _inSocket;
					tj::np::NativeSocket _in4Socket;
					tj::np::NetworkAddress _toAddress;
					unsigned short _toPort;
					tj::shared::ref< tj::np::SocketListenerThread> _listenerThread;
					tj::shared::ref<OSCOverUDPConnectionDefinition> _def;
					tj::shared::ref< tj::scout::ServiceRegistration > _serviceRegistration;
			};
		}
	}
}

#endif