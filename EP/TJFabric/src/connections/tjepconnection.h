#ifndef _TJ_EP_CONNECTION_H
#define _TJ_EP_CONNECTION_H

#include "../../../../TJShared/include/tjshared.h"
#include "../../include/tjfabricconnection.h"
#include "../../include/tjfabricgroup.h"
#include "../../include/tjfabricengine.h"
#include "../../../../TJScout/include/tjservice.h"
#include "../../../../TJNP/include/tjwebserver.h"

namespace tj {
	namespace fabric {
		namespace connections {
			/** An 'EP' connection is sort of a virtual connection that only creates a server to 'serve' the
			 definitions of the methods the fabric accepts over HTTP (following the EP standard) **/
			class EPConnectionDefinition: public tj::fabric::ConnectionDefinition {
				friend class EPConnection;
				
				public:
					EPConnectionDefinition();
					virtual ~EPConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
					
				protected:
					unsigned short _port;
					std::wstring _customPath;
			};
			
			class EPConnection: public Connection {
				public:
					EPConnection();
					virtual ~EPConnection();
					virtual void CreateForTransport(tj::shared::strong<tj::ep::EPTransport> ept, const tj::np::NetworkAddress& address);
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<FabricEngine> fe);
					virtual void Send(tj::shared::strong<Message> msg);
				
				protected:
					tj::shared::CriticalSection _lock;
					tj::shared::ref<tj::np::WebServer> _server;
					tj::shared::ref<tj::scout::ServiceRegistration> _serviceRegistration;
			};
		}
	}
}

#endif