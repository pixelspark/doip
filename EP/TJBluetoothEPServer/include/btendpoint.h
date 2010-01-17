#ifndef _BTEP_ENDPOINT_H
#define _BTEP_ENDPOINT_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpointserver.h>
#include <EP/include/epmessage.h>
#include <EP/include/eppublication.h>

namespace tj {
	namespace ep {
		namespace bt {
			using namespace tj::shared;
			using namespace tj::ep;
			
			class BTEndpoint: public EPEndpointServer<BTEndpoint> {
				public:
					BTEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName);
					virtual ~BTEndpoint();
					virtual EPMediationLevel GetMediationLevel() const;
					virtual void OnCreated();
					
				protected:
					ref<EPPublication> _publication;
			};
			
		}
	}
}

#endif