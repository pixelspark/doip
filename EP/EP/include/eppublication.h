/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 /* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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
