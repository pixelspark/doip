#ifndef _TJ_EP_DISCOVERY_H
#define _TJ_EP_DISCOVERY_H

#include "epinternal.h"
#include "epconnection.h"

namespace tj {
	namespace ep {
		class EP_EXPORTED DiscoveryDefinition: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				virtual ~DiscoveryDefinition();
				virtual tj::shared::String GetType() const;
				
			protected:
				DiscoveryDefinition(const tj::shared::String& type);
				tj::shared::String _type;
		};
		
		class EP_EXPORTED DiscoveryDefinitionFactory: public tj::shared::PrototypeBasedFactory<DiscoveryDefinition> {
			public:
				virtual ~DiscoveryDefinitionFactory();
				virtual tj::shared::ref<DiscoveryDefinition> Load(TiXmlElement* me);
				virtual void Save(tj::shared::strong<DiscoveryDefinition> c, TiXmlElement* me);
				
				static tj::shared::strong<DiscoveryDefinitionFactory> Instance();
				
			protected:
				DiscoveryDefinitionFactory();
				static tj::shared::ref<DiscoveryDefinitionFactory> _instance;
		};
		
		struct EP_EXPORTED DiscoveryNotification {
			DiscoveryNotification(const tj::shared::Timestamp& ts, tj::shared::ref<Connection> m, bool add, EPMediationLevel ml);
			
			tj::shared::Timestamp when;
			tj::shared::ref<Connection> connection;
			tj::shared::ref<EPEndpoint> endpoint;
			bool added;
			EPMediationLevel mediationLevel;
		};
		
		class EP_EXPORTED Discovery: public virtual tj::shared::Object {
			public:
				virtual ~Discovery();
				virtual void Create(tj::shared::strong<DiscoveryDefinition> def, const tj::shared::String& ownMagic) = 0;
				
				tj::shared::Listenable<DiscoveryNotification> EventDiscovered;
		};
		
		class EP_EXPORTED DiscoveryFactory: public virtual tj::shared::PrototypeBasedFactory< Discovery > {
			public:
				virtual ~DiscoveryFactory();
				virtual tj::shared::ref<Discovery> CreateFromDefinition(tj::shared::strong<DiscoveryDefinition> cd, const tj::shared::String& ownMagic = L"");
				static tj::shared::strong< DiscoveryFactory > Instance();
				
			protected:
				DiscoveryFactory();
				static tj::shared::ref< DiscoveryFactory > _instance;
		};
	}
}

#endif