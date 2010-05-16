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
#include <TJNP/include/tjhttp.h>
#include <TJScout/include/tjservice.h>

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		class EP_EXPORTED EPDownloadedState: public virtual tj::shared::Object, public tj::np::Download, public EPState {
			public:
				EPDownloadedState(tj::shared::strong<tj::scout::Service> service, const tj::shared::String& path);
				virtual ~EPDownloadedState();
				virtual tj::shared::strong<tj::scout::Service> GetService();
				virtual void GetState(ValueMap& vals);
				virtual tj::shared::Any GetValue(const tj::shared::String& key);
				
				struct EPStateDownloadNotification {
				};
				
				tj::shared::Listenable<EPStateDownloadNotification> EventDownloaded;
				
			protected:
				virtual void LoadState(TiXmlElement* me);
				virtual void OnDownloadComplete(tj::shared::ref<tj::shared::DataWriter> cw);
			
				tj::shared::CriticalSection _lock;
				tj::shared::strong<tj::scout::Service> _service;
				EPState::ValueMap _state;
		};
		
		class EP_EXPORTED EPDownloadedDefinition: public tj::np::Download {
			public:
				EPDownloadedDefinition(tj::shared::strong<tj::scout::Service> service, const tj::shared::String& path);
				virtual ~EPDownloadedDefinition();
				virtual void OnCreated();
				virtual tj::shared::strong<tj::scout::Service> GetService();
				virtual tj::shared::ref<EPEndpoint> GetEndpoint();
			
				struct EPDownloadNotification {
					EPDownloadNotification(tj::shared::ref<EPEndpoint> ep, tj::shared::strong<tj::scout::Service> service);
					tj::shared::ref<EPEndpoint> endpoint;
					tj::shared::strong<tj::scout::Service> service;
				};
			
				tj::shared::Listenable<EPDownloadNotification> EventDownloaded;
			
			protected:
				tj::shared::strong<tj::scout::Service> _service;
				tj::shared::ref<EPEndpoint> _createdEndpoint;
				virtual void OnDownloadComplete(tj::shared::ref<tj::shared::DataWriter> cw);
		};
	}
}

#pragma warning(pop)
#endif