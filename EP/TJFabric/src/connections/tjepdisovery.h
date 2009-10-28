#ifndef _TJ_FABRIC_EP_DISCOVERY_H
#define _TJ_FABRIC_EP_DISCOVERY_H

#include "tjdnssddiscovery.h"
#include "../../../EPFramework/include/ependpoint.h"
#include "../../../EPFramework/include/epdownload.h"

namespace tj {
	namespace fabric {
		namespace connections {
			class EPDiscoveryDefinition: public DNSSDDiscoveryDefinition {
				public:
					EPDiscoveryDefinition();
					virtual ~EPDiscoveryDefinition();
			};
			
			class EPDiscovery: public DNSSDDiscovery, public tj::shared::Listener<tj::ep::EPDownloadedDefinition::EPDownloadNotification> {
				public:
					EPDiscovery();
					virtual ~EPDiscovery();
					virtual void Create(tj::shared::strong<DiscoveryDefinition> def);
					virtual void Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data);
					virtual void Notify(tj::shared::ref<Object> src, const tj::ep::EPDownloadedDefinition::EPDownloadNotification& data);
				
				protected:
					tj::shared::CriticalSection _lock;
					std::set< tj::shared::ref<tj::ep::EPDownloadedDefinition> > _downloading;
			};
			
			
		}
	}
}

#endif