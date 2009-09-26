#ifndef _TJ_OSCUDP_CONNECTION_H
#define _TJ_OSCUDP_CONNECTION_H

#include "../../../TJShared/include/tjshared.h"
#include "../../include/tjfabricconnection.h"
#include "../../include/tjfabricgroup.h"
#include "../../../TJNP/include/tjsocketutil.h"
#include "../../../Libraries/OSCPack/osc/OscReceivedElements.h"
#include <arpa/inet.h>

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
			
			enum AddressFamily {
				AddressFamilyNone = 0,
				AddressFamilyIPv4,
				AddressFamilyIPv6,
			};
			
			class NetworkAddress {
				public:
					NetworkAddress(const tj::shared::String& spec, bool passive = false);
					~NetworkAddress();
					std::wstring ToString() const;
					void GetSocketAddress(sockaddr_in6* addr) const;
					
				
				protected:
					AddressFamily _family;
					sockaddr_in6 _address;
					sockaddr_in _v4address;
					
			};
			
			class OSCOverUDPConnection: public Connection, public tj::np::SocketListener {
				public:
					OSCOverUDPConnection();
					virtual ~OSCOverUDPConnection();
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d);
					virtual void Send(tj::shared::strong< Message > msg);
					virtual void OnReceive(tj::np::NativeSocket ns);
					virtual void OnReceiveBundle(osc::ReceivedBundle rb);
					virtual void OnReceiveMessage(osc::ReceivedMessage rb);
				
				protected:
					tj::shared::CriticalSection _lock;
					tj::np::NativeSocket _outSocket;
					tj::np::NativeSocket _inSocket;
					NetworkAddress _toAddress;
					tj::shared::ref< tj::np::SocketListenerThread> _listenerThread;
					tj::shared::ref<OSCOverUDPConnectionDefinition> _def;
			};
		}
	}
}

#endif