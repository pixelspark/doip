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

/** FabricEngine **/
FabricEngine::FabricEngine(): _fabric(GC::Hold(new Fabric())) {
	_state = GC::Hold(new EPStateDefinition());
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

ref<FabricState> FabricEngine::GetState() {
	if(!_fabricState) {
		_fabricState = GC::Hold(new FabricState(this));
	}
	return _fabricState;
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
	_state = GC::Hold(new EPStateDefinition());
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
		_state = _fabric->CreateDefaultState();
		
		/* Don't advertise the service if the mediation level is below zero (which means 
		that this fabric does not participate at all in mediation */
		if(_fabric->GetMediationLevel()>=0) {
			_publication = GC::Hold(new EPPublication(strong<EPEndpoint>(ref<EPEndpoint>(_fabric)), _fabric->GetID()));
		}
		
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
		_publication = null;
	}
}

/** FabricState **/
FabricState::FabricState(ref<FabricEngine> fe): _fe(fe), _shouldCommit(true) {
}

FabricState::~FabricState() {
}

void FabricState::GetState(EPState::ValueMap& vals) {
	ref<FabricEngine> fe = _fe;
	if(fe) {
		fe->_state->GetState(vals);
	}
}

Any FabricState::GetValue(const tj::shared::String& key) {
	ref<FabricEngine> fe = _fe;
	if(fe) {
		return fe->_state->GetValue(key);
	}
	return Any();
}

ref<Scriptable> FabricState::Execute(Command c, ref<ParameterList> p) {
	ref<FabricEngine> fe = _fe;
	if(fe) {
		return GC::Hold(new ScriptAnyValue(fe->_state->GetValue(c)));
	}
	return null;
}

bool FabricState::Set(Field field, ref<Scriptable> value) {
	ref<FabricEngine> fe = _fe;
	if(fe) {
		ThreadLock lock(&_lock);
		if(value.IsCastableTo<ScriptAny>()) {
			Any previousValue = fe->_state->GetValue(field);
			Any val = ref<ScriptAny>(value)->Unbox();
			
			// Retain value type if previous type was not null
			if(previousValue.GetType()!=Any::TypeNull) {
				val = val.Force(previousValue.GetType());
			}
			fe->_state->SetValue(field, val);
			_shouldCommit = true;
			return true;
		}
	}
	return false;
}

void FabricState::Commit() {
	ThreadLock lock(&_lock);
	if(_shouldCommit) {
		_shouldCommit = false;
		ref<FabricEngine> fe = _fe;
		if(fe) {
			ref<EPPublication> pub = fe->_publication;
			if(pub) {
				EPState::ValueMap stateValues;
				fe->_state->GetState(stateValues);
				pub->SetState(stateValues);
			}
		}
	}
}