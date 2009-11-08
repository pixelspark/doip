#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricgroup.h"
#include "../include/tjfabricconnection.h"

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::np;
using namespace tj::scout;
using namespace tj::ep;
using namespace tj::script;

FabricEngine::FabricEngine(): _fabric(GC::Hold(new Fabric())) {
}

FabricEngine::~FabricEngine() {
}

void FabricEngine::OnCreated() {
	_queue = GC::Hold(new Queue(this)); 
}

strong<Fabric> FabricEngine::GetFabric() {
	return _fabric;
}

strong<Queue> FabricEngine::GetQueue() {
	return _queue;
}

void FabricEngine::SetFabric(strong<Fabric> f) {
	ThreadLock lock(&_lock);
	Clear();
	_fabric = f;
}

void FabricEngine::Clear() {
	ThreadLock lock(&_lock);
	Connect(false);
	_fabric->Clear();
	_queue->Clear();
}

void FabricEngine::Send(const String& gid, strong<Message> m, ref<ReplyHandler> rh) {
	std::map< ref<Group>, ref<ConnectedGroup> >::iterator it = _groups.begin();
	while(it!=_groups.end()) {
		ref<Group> group = it->first;
		ref<ConnectedGroup> cg = it->second;
		if(cg && group && group->GetID()==gid) {
			cg->Send(m, ref<FabricEngine>(this), rh);
			return;
		}
		++it;
	}
}

void FabricEngine::Notify(ref<Object> source, const MessageNotification& data) {
	_queue->Add(data.message, data.source, data.channel);
}

void FabricEngine::Notify(ref<Object> source, const DiscoveryScriptNotification& data) {
	_queue->AddDiscoveryScriptCall(data.definition, data.connection, data.scriptSource);
}

void FabricEngine::Connect(bool t) {
	if(t) {
		_registration = FabricRegistry::Instance()->Register(_fabric->GetFullIdentifier(), this);
		
		// Iterate through all groups and connect them
		std::map< ref<Group>, ref<ConnectedGroup> > newGroups;
		
		std::deque< ref<Group> >::iterator it = _fabric->_groups.begin();
		while(it!=_fabric->_groups.end()) {
			ref<Group> group = *it;
			if(group) {
				std::map< ref<Group>, ref<ConnectedGroup> >::iterator eit = _groups.find(group);
				ref<ConnectedGroup> cg;
				if(eit!=_groups.end()) {
					cg = eit->second;
				}
				else {
					cg = GC::Hold(new ConnectedGroup(group));
				}
				
				cg->EventMessageReceived.AddListener(ref<FabricEngine>(this));
				cg->EventDiscoveryScript.AddListener(ref<FabricEngine>(this));
				cg->Connect(true, ref<FabricEngine>(this));
				newGroups[group] = cg;
			}
			++it;
		}
		
		_groups = newGroups;
	}
	else {
		/* No need to call Connect(false) on each group, a ConnectedGroup will
		automatically disconnect when it is destroyed. */
		_groups.clear();
		_registration = null;
	}
}