#ifndef _TJ_FABRIC_ENGINE_H
#define _TJ_FABRIC_ENGINE_H

#include "../../../TJShared/include/tjshared.h"
#include "../../../TJNP/include/tjwebserver.h"
#include "../../../TJScout/include/tjservice.h"
#include "tjfabric.h"
#include "tjfabricconnection.h"

namespace tj {
	namespace fabric {
		class Queue;
		class Group;
		class Message;
		
		class FabricEngine: public virtual tj::shared::Object, public tj::shared::Listener<MessageNotification> {
			public:
				FabricEngine();
				virtual ~FabricEngine();
				virtual tj::shared::strong<Fabric> GetFabric();
				virtual tj::shared::strong<Queue> GetQueue();
				virtual void SetFabric(tj::shared::strong<Fabric> f);
				virtual void Connect(bool t);
				virtual void Send(const tj::shared::String& gid, tj::shared::strong<Message> m);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const MessageNotification& data);
			
			protected:
				virtual void OnCreated();
				virtual void Clear();
			
				tj::shared::CriticalSection _lock;
				tj::shared::ref<Fabric> _fabric;
				tj::shared::ref<Queue> _queue;
				std::map< tj::shared::ref<Group>, tj::shared::ref<ConnectedGroup> > _groups;
		};
	}
}

#endif