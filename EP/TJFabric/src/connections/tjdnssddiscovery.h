#ifndef _TJ_DNSSD_DISCOVERY_H
#define _TJ_DNSSD_DISCOVERY_H

#include "../../../../TJScout/include/tjresolver.h"
#include "../../include/tjfabricgroup.h"
#include "../../include/tjfabricconnection.h"

namespace tj {
	namespace fabric {
		namespace connections {
			class DNSSDDiscoveryDefinition: public DiscoveryDefinition {
				public:
					DNSSDDiscoveryDefinition();
					virtual ~DNSSDDiscoveryDefinition();
					virtual void Load(TiXmlElement* me);
					virtual void Save(TiXmlElement* me);
				
				protected:
					std::wstring _serviceType;
			};
			
			class DNSSDDiscovery: public Discovery, public tj::shared::Listener<tj::scout::ResolveRequest::ServiceNotification> {
				public:
					DNSSDDiscovery();
					virtual ~DNSSDDiscovery();
					virtual void Create(tj::shared::strong<DiscoveryDefinition> def);
					virtual void Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data);
				
				protected:
					tj::shared::ref< tj::scout::ResolveRequest > _resolver;
			};
		}
	}
}

#endif