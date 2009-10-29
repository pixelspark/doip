#ifndef _TJ_FABRIC_SERVER_H
#define _TJ_FABRIC_SERVER_H

#include "../../../TJNP/include/tjwebserver.h"
#include "tjfabric.h"
#include "tjfabricengine.h"

namespace tj {
	namespace fabric {
		/** The FabricProcess class generates a unique 'magic' key for each fabric process. This is
		 used to prevent discovery of services published by the process itself: when the service supports
		 it, services published from this process should have the EPMagicNumber attribute set to the
		 process magic number. When discovering services, the EPMagicNumber attribute should be checked.
		 If it is present, it should be compared with the process magic number. If the magic numbers are
		 equal, the discovery is ignored **/
		class FabricProcess {
			public:
				static tj::shared::String GetServerMagic();
			
			protected:
				static tj::shared::String _magic;
		};
		
		/** Manages WebServer instances **/
		class WebServerManager {
			public:
				virtual ~WebServerManager();
				static tj::shared::strong<WebServerManager> Instance();
				virtual tj::shared::ref<tj::np::WebServer> CreateServer(unsigned short port);
			
				const static unsigned short KPortDontCare = 0;
				
			protected:
				WebServerManager();
				
				tj::shared::CriticalSection _lock;
				std::map< unsigned short, tj::shared::ref<tj::np::WebServer> > _servers;
				static tj::shared::ref<WebServerManager> _instance;
		};
		
		/** Serves an XML-file containing the definitions of the messages this fabric will accept **/
		class FabricDefinitionResolver: public tj::np::FileRequestResolver {
			public:
				FabricDefinitionResolver(tj::shared::ref<Fabric> model);
				virtual ~FabricDefinitionResolver();
				virtual tj::np::FileRequestResolver::Resolution Resolve(tj::shared::ref<tj::np::FileRequest> frq, std::wstring& file, std::wstring& error, char** data, unsigned int& dataLength);
				
			protected:
				tj::shared::ref<Fabric> _fabric;
		};

		/** Sends HTTP requests to the queue as messages **/
		class FabricMessageResolver: public tj::np::FileRequestResolver {
			public:
				FabricMessageResolver(tj::shared::ref<FabricEngine> model, const std::wstring& prefix);
				virtual ~FabricMessageResolver();
				virtual tj::np::FileRequestResolver::Resolution Resolve(tj::shared::ref<tj::np::FileRequest> frq, std::wstring& file, std::wstring& error, char** data, unsigned int& dataLength);
				
			protected:
				tj::shared::ref<FabricEngine> _fabric;
				std::wstring _prefix;
		};
	}
}

#endif