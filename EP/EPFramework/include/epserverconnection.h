#ifndef _TJ_EP_SERVER_CONNECTION_H
#define _TJ_EP_SERVER_CONNECTION_H

#include "internal/ep.h"
#include "epconnection.h"
#include "../../../TJScout/include/tjservice.h"
#include "../../../TJNP/include/tjwebserver.h"

namespace tj {
	namespace ep {
		/** An 'EP' connection is sort of 'pseudo'-connection that only creates a server to 'serve' the
		 definitions of the methods the fabric accepts over HTTP (following the EP standard) **/
		class EP_EXPORTED EPServerDefinition: public ConnectionDefinition {
			friend class EPServerConnection;
			
			public:
				EPServerDefinition();
				virtual ~EPServerDefinition();
				virtual void SaveConnection(TiXmlElement* me);
				virtual void LoadConnection(TiXmlElement* me);
				virtual tj::shared::String GetAddress() const;
				virtual tj::shared::String GetFormat() const;
				virtual tj::shared::String GetFraming() const;
				virtual unsigned short GetPort() const;
				
			protected:
				unsigned short _port;
				std::wstring _customPath;
		};
		
		class EP_EXPORTED EPServerConnection: public Connection {
			public:
				EPServerConnection();
				virtual ~EPServerConnection();
				virtual void CreateForTransport(tj::shared::strong<tj::ep::EPTransport> ept, const tj::np::NetworkAddress& address);
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<EPEndpoint> parent);
				virtual void Send(tj::shared::strong<Message> msg, tj::shared::ref<ReplyHandler> rh, tj::shared::ref<ConnectionChannel> cc);
				
			protected:
				tj::shared::CriticalSection _lock;
				tj::shared::ref<tj::np::WebServer> _server;
				tj::shared::ref<tj::scout::ServiceRegistration> _serviceRegistration;
		};
	}
}

#endif