#include "../include/tjfabricregistry.h" 
#include "../include/tjfabricutil.h"
#include "../include/tjfabricengine.h"
#include "../include/tjfabricqueue.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

ref<FabricRegistry> FabricRegistry::_instance;

FabricRegistry::FabricRegistry() {
}

FabricRegistry::~FabricRegistry() {
}

strong<FabricRegistry> FabricRegistry::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new FabricRegistry());
	}
	return _instance;
}

void FabricRegistry::Send(const String& pattern, strong<Message> m) {
	ThreadLock lock(&_lock);
	std::map<std::wstring, weak<FabricEngine> >::iterator it = _registrations.begin();
	while(it!=_registrations.end()) {
		ref<FabricEngine> fe = it->second;
		if(fe) {
			if(Pattern::Matches(pattern, it->first)) {
				fe->GetQueue()->Add(m);
			}
		}
		++it;
	}
}

ref<FabricRegistration> FabricRegistry::Register(const String& fqdn, ref<FabricEngine> fe) {
	ThreadLock lock(&_lock);
	_registrations[fqdn] = fe;
	return GC::Hold(new FabricRegistration(fqdn));
}

void FabricRegistry::Unregister(const String& fqdn) {
	ThreadLock lock(&_lock);
	std::map<String, weak<FabricEngine> >::iterator it = _registrations.find(fqdn);
	if(it!=_registrations.end()) {
		_registrations.erase(it);
	}
}

/** FabricRegistration **/
FabricRegistration::FabricRegistration(const String& fqdn): _fqdn(fqdn) {
}

FabricRegistration::~FabricRegistration() {
	FabricRegistry::Instance()->Unregister(_fqdn);
}