#ifndef _TJ_EP_SERVER_MGR_H
#define _TJ_EP_SERVER_MGR_H

#include "epinternal.h"
#include "ependpoint.h"
#include <TJNP/include/tjwebserver.h>

namespace tj {
	namespace ep {
		/** The EPServerManager class generates a unique 'magic' key for each fabric process. This is
		 used to prevent discovery of services published by the process itself: when the service supports
		 it, services published from this process should have the EPMagicNumber attribute set to the
		 process magic number. When discovering services, the EPMagicNumber attribute should be checked.
		 If it is present, it should be compared with the process magic number. If the magic numbers are
		 equal, the discovery is ignored **/
		class EP_EXPORTED EPServerManager {
			public:
				virtual ~EPServerManager();
				static tj::shared::strong<EPServerManager> Instance();
				virtual tj::shared::ref<tj::np::WebServer> CreateServer(unsigned short port);
				virtual tj::shared::String GetServerMagic();
				
				const static unsigned short KPortDontCare = 0;
				
			protected:
				EPServerManager();
				
				tj::shared::CriticalSection _lock;
				std::map< unsigned short, tj::shared::ref<tj::np::WebServer> > _servers;
				static tj::shared::ref<EPServerManager> _instance;
				tj::shared::String _magic;
		};
		
		/** Serves an XML-file containing the definitions of the messages this fabric will accept **/
		class EP_EXPORTED EPDefinitionWebItem: public tj::np::WebItemResource {
			public:
				EPDefinitionWebItem(tj::shared::ref<EPEndpoint> model);
				virtual ~EPDefinitionWebItem();
				virtual tj::np::Resolution Get(tj::shared::ref<tj::np::WebRequest> frq, std::wstring& error, char** data, tj::shared::Bytes& dataLength);
				
			protected:
				tj::shared::ref<EPEndpoint> _endpoint;
		};
	}
}

#endif