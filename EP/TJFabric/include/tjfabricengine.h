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
 
 #ifndef _TJ_FABRIC_ENGINE_H
#define _TJ_FABRIC_ENGINE_H

#include <TJShared/include/tjshared.h>
#include <TJNP/include/tjwebserver.h>
#include <TJScout/include/tjservice.h>
#include <TJScript/include/tjscript.h>
#include <EP/include/epmessage.h>
#include <EP/include/eppublication.h>
#include "tjfabric.h"
#include "tjfabricconnection.h"

namespace tj {
	namespace fabric {
		class Queue;
		class Group;
		class FabricState;
		
		class FabricEngine: public virtual tj::shared::Object, public tj::shared::Listener<tj::ep::MessageNotification>, public tj::shared::Listener<DiscoveryScriptNotification> {
			friend class FabricState;
			
			public:
				FabricEngine();
				virtual ~FabricEngine();
				virtual tj::shared::strong<Fabric> GetFabric();
				virtual tj::shared::strong<Queue> GetQueue();
				virtual void SetFabric(tj::shared::strong<Fabric> f);
				virtual void Connect(bool t);
				virtual tj::shared::ref<FabricState> GetState();
				virtual void Send(const tj::shared::String& gid, tj::shared::strong<tj::ep::Message> m, tj::shared::ref<tj::ep::ReplyHandler> handler);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const tj::ep::MessageNotification& data);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const DiscoveryScriptNotification& data);
			
			protected:
				virtual void OnCreated();
				virtual void Clear();
			
				tj::shared::CriticalSection _lock;
				tj::shared::ref<Fabric> _fabric;
				tj::shared::ref<Queue> _queue;
				tj::shared::ref<tj::ep::EPPublication> _publication;
				std::map< tj::shared::ref<Group>, tj::shared::ref<ConnectedGroup> > _groups;
				tj::shared::ref< tj::ep::EPStateDefinition > _state;
				tj::shared::ref< FabricState > _fabricState;
		};
		
		class FabricState: public virtual tj::shared::Object, public tj::ep::EPState, public tj::script::Scriptable {
			public:
				FabricState(tj::shared::ref<FabricEngine> fe);
				virtual ~FabricState();
				virtual void GetState(ValueMap& vals);
				virtual tj::shared::Any GetValue(const tj::shared::String& key);
				virtual tj::shared::ref<tj::script::Scriptable> Execute(tj::script::Command c, tj::shared::ref<tj::script::ParameterList> p);
				virtual bool Set(tj::script::Field field, tj::shared::ref<tj::script::Scriptable> value);
				virtual void Commit();
			
			protected:
				bool _shouldCommit;
				tj::shared::CriticalSection _lock;
				tj::shared::weak< FabricEngine> _fe;
		};
	}
}

#endif