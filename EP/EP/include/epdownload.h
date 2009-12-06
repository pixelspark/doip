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
		class EP_EXPORTED EPDownloadedDefinition: public tj::np::Download {
			public:
				EPDownloadedDefinition(tj::shared::strong<tj::scout::Service> service, const tj::shared::String& path);
				virtual ~EPDownloadedDefinition();
				virtual void OnCreated();
				virtual tj::shared::strong<tj::scout::Service> GetService();
			
				struct EPDownloadNotification {
					EPDownloadNotification(tj::shared::ref<EPEndpoint> ep, tj::shared::strong<tj::scout::Service> service);
					tj::shared::ref<EPEndpoint> endpoint;
					tj::shared::strong<tj::scout::Service> service;
				};
			
				tj::shared::Listenable<EPDownloadNotification> EventDownloaded;
			
			protected:
				tj::shared::strong<tj::scout::Service> _service;
				virtual void OnDownloadComplete(tj::shared::ref<tj::shared::CodeWriter> cw);
		};
	}
}

#pragma warning(pop)
#endif