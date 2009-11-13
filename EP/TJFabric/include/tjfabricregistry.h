#ifndef _TJ_FABRIC_REGISTRY_H
#define _TJ_FABRIC_REGISTRY_H

#include <TJShared/include/tjshared.h>
#include <EP/include/epmessage.h>

namespace tj {
	namespace fabric {
		class FabricEngine;
		
		class FabricRegistration: public virtual tj::shared::Object {
			friend class FabricRegistry;
			
			public:
				FabricRegistration(const tj::shared::String& fqdn);
				virtual ~FabricRegistration();
			
			protected:
				tj::shared::String _fqdn;
		};
		
		class FabricRegistry: public virtual tj::shared::Object {
			friend class FabricRegistration;
			
			public:
				virtual ~FabricRegistry();
				static tj::shared::strong<FabricRegistry> Instance();
				virtual tj::shared::ref<FabricRegistration> Register(const tj::shared::String& fqdn, tj::shared::ref<FabricEngine> fe);
				virtual void Send(const tj::shared::String& pattern, tj::shared::strong<tj::ep::Message> msg);
			
			protected:
				virtual void Unregister(const tj::shared::String& fqdn);
				FabricRegistry();
				tj::shared::CriticalSection _lock;
				std::map< tj::shared::String, tj::shared::weak<FabricEngine> > _registrations;
				static tj::shared::ref<FabricRegistry> _instance;
		};
	}
}

#endif