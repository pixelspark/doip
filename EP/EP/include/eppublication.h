#ifndef _TJ_EP_DOWNLOAD_H
#define _TJ_EP_DOWNLOAD_H

#include "epinternal.h"
#include "ependpoint.h"
#include <TJScout/include/tjservice.h>
#include <TJNP/include/tjwebserver.h>

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		class EP_EXPORTED EPPublication: public virtual tj::shared::Object, public EPState {
			public:
				EPPublication(tj::shared::strong<EPEndpoint> ep, const std::wstring& magicPostfix = L"");
				virtual ~EPPublication();
				virtual void SetState(const EPState::ValueMap& values);
				virtual void SetStateVariable(const tj::shared::String& key, const tj::shared::Any& value);
				virtual void LoadState(TiXmlElement* root);
				virtual void GetState(EPState::ValueMap& vals);
				virtual tj::shared::Any GetValue(const tj::shared::String& key);
				virtual tj::shared::ref<EPEndpoint> GetEndpoint();
				
			protected:
				virtual void OnCreated();
				virtual void PublishState();
			
				tj::shared::CriticalSection _lock;
				tj::shared::String _stateVersion;
				tj::shared::String _magicPostfix;
				EPState::ValueMap _state;
				tj::shared::ref<tj::np::WebServer> _ws;
				tj::shared::weak<EPEndpoint> _ep;
				tj::shared::ref<tj::scout::ServiceRegistration> _reg;
		};
	}
}

#endif
