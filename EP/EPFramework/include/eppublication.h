#ifndef _TJ_EP_DOWNLOAD_H
#define _TJ_EP_DOWNLOAD_H

#include "internal/ep.h"
#include "ependpoint.h"
#include "../../../TJScout/include/tjservice.h"
#include "../../../TJNP/include/tjwebserver.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		class EP_EXPORTED EPPublication {
			public:
				EPPublication(tj::shared::strong<EPEndpoint> ep);
				virtual ~EPPublication();
				
			protected:
				tj::shared::ref<tj::np::WebServer> _ws;
				tj::shared::ref<tj::scout::ServiceRegistration> _reg;
		};
	}
}

#endif
