#ifndef _TJ_FABRIC_ENGINE_H
#define _TJ_FABRIC_ENGINE_H

#include <TJShared/include/tjshared.h>
#include <TJNP/include/tjwebserver.h>
#include <TJScout/include/tjservice.h>
#include <TJScript/include/tjscript.h>
#include <EP/include/epmessage.h>
#include "tjfabric.h"
#include "tjfabricconnection.h"
#include "tjfabricregistry.h"

namespace tj {
	namespace fabric {
		class Queue;
		class Group;
		
		class FabricEngine: public virtual tj::shared::Object, public tj::shared::Listener<tj::ep::MessageNotification>, public tj::shared::Listener<DiscoveryScriptNotification> {
			public:
				FabricEngine();
				virtual ~FabricEngine();
				virtual tj::shared::strong<Fabric> GetFabric();
				virtual tj::shared::strong<Queue> GetQueue();
				virtual void SetFabric(tj::shared::strong<Fabric> f);
				virtual void Connect(bool t);
				virtual void Send(const tj::shared::String& gid, tj::shared::strong<tj::ep::Message> m, tj::shared::ref<tj::ep::ReplyHandler> handler);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const tj::ep::MessageNotification& data);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const DiscoveryScriptNotification& data);
			
			protected:
				virtual void OnCreated();
				virtual void Clear();
			
				tj::shared::CriticalSection _lock;
				tj::shared::ref<Fabric> _fabric;
				tj::shared::ref<Queue> _queue;
				tj::shared::ref<FabricRegistration> _registration;
				std::map< tj::shared::ref<Group>, tj::shared::ref<ConnectedGroup> > _groups;
		};
	}
}

#endif