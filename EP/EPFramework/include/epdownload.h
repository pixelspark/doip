#ifndef _TJ_EP_DOWNLOAD_H
#define _TJ_EP_DOWNLOAD_H

#include "internal/tjep.h"
#include "ependpoint.h"
#include "../../../TJNP/include/tjhttp.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		class EP_EXPORTED EPDownloadedDefinition: public tj::np::Download {
			public:
				EPDownloadedDefinition(const tj::np::NetworkAddress& host, unsigned short port, const tj::shared::String& path);
				virtual ~EPDownloadedDefinition();
				virtual void OnCreated();
			
				struct EPDownloadNotification {
					EPDownloadNotification(tj::shared::ref<EPEndpoint> ep);
					tj::shared::ref<EPEndpoint> endpoint;
				};
			
				tj::shared::Listenable<EPDownloadNotification> EventDownloaded;
			
			protected:
				virtual void OnDownloadComplete(tj::shared::ref<tj::shared::CodeWriter> cw);
		};
	}
}

#pragma warning(pop)
#endif