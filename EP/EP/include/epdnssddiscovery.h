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
 
 #ifndef _TJ_EP_DNSSD_DISCOVERY_H
#define _TJ_EP_DNSSD_DISCOVERY_H

#include <TJScout/include/tjresolver.h>
#include "epinternal.h"
#include "epdiscovery.h"
#include "epcondition.h"
#include "epdownload.h"

namespace tj {
	namespace ep {
		class EP_EXPORTED DNSSDDiscoveryDefinition: public DiscoveryDefinition {
			friend class DNSSDDiscovery;
			
			public:
				DNSSDDiscoveryDefinition();
				virtual ~DNSSDDiscoveryDefinition();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				
			protected:
				DNSSDDiscoveryDefinition(const std::wstring& type, const std::wstring& defaultServiceType);
				std::wstring _serviceType;
		};
		
		class EP_EXPORTED DNSSDDiscovery: public Discovery, public tj::shared::Listener<tj::scout::ResolveRequest::ServiceNotification> {
			public:
				DNSSDDiscovery();
				virtual ~DNSSDDiscovery();
				virtual void Create(tj::shared::strong<DiscoveryDefinition> def, const tj::shared::String& ownMagic);
				virtual void Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data);
				
			protected:
				tj::shared::ref< tj::scout::ResolveRequest > _resolver;
		};
		
		class EP_EXPORTED EPDiscoveryDefinition: public DNSSDDiscoveryDefinition {
			friend class EPDiscovery;
			
			public:
				EPDiscoveryDefinition();
				virtual ~EPDiscoveryDefinition();
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* me);
				
			protected:
				tj::shared::ref<EPCondition> _condition;
		};
		
		class EP_EXPORTED EPDiscovery: public DNSSDDiscovery, public tj::shared::Listener<EPDownloadedDefinition::EPDownloadNotification> {
			public:
				EPDiscovery();
				virtual ~EPDiscovery();
				virtual void Create(tj::shared::strong<DiscoveryDefinition> def, const tj::shared::String& ownMagic);
				virtual void Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data);
				virtual void Notify(tj::shared::ref<Object> src, const EPDownloadedDefinition::EPDownloadNotification& data);
				
			protected:
				tj::shared::CriticalSection _lock;
				tj::shared::String _ownMagic;
				std::set< tj::shared::ref<EPDownloadedDefinition> > _downloading;
				std::map< tj::shared::ref<tj::scout::Service>, tj::shared::weak<Connection> > _discovered;
				std::map< tj::shared::ref<tj::scout::Service>, tj::shared::weak<EPEndpoint> > _discoveredEndpoints;
				std::set<tj::shared::String> _discoveredIDs;
				tj::shared::ref<EPCondition> _condition;
		};
	}
}

#endif