#ifndef _TJ_FABRIC_SERVER_H
#define _TJ_FABRIC_SERVER_H

#include "../../TJNP/include/tjwebserver.h"
#include "tjfabric.h"
#include "tjfabricengine.h"

namespace tj {
	namespace fabric {
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