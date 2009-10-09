#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricgroup.h"
#include "../include/tjfabricconnection.h"
#include "../include/tjfabricmessage.h"
#include "../include/tjfabricserver.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::np;
using namespace tj::scout;

FabricEngine::FabricEngine(): _fabric(GC::Hold(new Fabric())) {
}

FabricEngine::~FabricEngine() {
}

void FabricEngine::OnCreated() {
	_queue = GC::Hold(new Queue(this)); 
	
	// TODO: make web server configurable through fabric file
	_webServer = GC::Hold(new WebServer(7961));
	_webServer->AddResolver(L"/ep/definition", ref<FileRequestResolver>(GC::Hold(new FabricDefinitionResolver(_fabric))));
	_webServer->AddResolver(L"/ep/message", ref<FileRequestResolver>(GC::Hold(new FabricMessageResolver(this, L"/ep/message"))));
	_serviceRegistration = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_dashboard._tcp", L"Dashboard", 7961);
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

void FabricEngine::Send(const String& gid, strong<Message> m) {
	std::map< ref<Group>, ref<ConnectedGroup> >::iterator it = _groups.begin();
	while(it!=_groups.end()) {
		ref<Group> group = it->first;
		ref<ConnectedGroup> cg = it->second;
		if(cg && group && group->GetID()==gid) {
			cg->Send(m);
			return;
		}
		++it;
	}
}

void FabricEngine::Notify(ref<Object> source, const MessageNotification& data) {
	_queue->Add(data.message);
}

void FabricEngine::Connect(bool t) {
	if(t) {
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
				cg->Connect(true);
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
	}
}