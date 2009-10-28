#include "tjepdisovery.h"
using namespace tj::shared;
using namespace tj::np;
using namespace tj::ep;
using namespace tj::fabric;
using namespace tj::scout;
using namespace tj::fabric::connections;

EPDiscoveryDefinition::EPDiscoveryDefinition(): DNSSDDiscoveryDefinition(L"ep", L"_ep._tcp") {
}

EPDiscoveryDefinition::~EPDiscoveryDefinition() {
}

EPDiscovery::EPDiscovery() {
}

EPDiscovery::~EPDiscovery() {
}

void EPDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def) {
	DNSSDDiscovery::Create(def);
}

void EPDiscovery::Notify(ref<Object> src, const EPDownloadedDefinition::EPDownloadNotification& data) {
	ref<EPEndpoint> epe = data.endpoint;
	if(epe) {
		Log::Write(L"TJFabric/EPDiscovery", L"Found endpoint; friendly name="+epe->GetFriendlyName()+L" fqdn="+epe->GetFullIdentifier());
		
		ThreadLock lock(&_lock);
		_downloading.erase(ref<EPDownloadedDefinition>(src));
	}
}

void EPDiscovery::Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data) {
	ThreadLock lock(&_lock);
	
	if(data.online) {
		std::wstring defPath;
		ref<Service> service = data.service;
		if(service->GetAttribute(L"EPDefinitionPath", defPath)) {
			Log::Write(L"TJFabric/EPDiscovery", L"Found EP endpoint; will download definitions at http://"+service->GetHostName()+L":"+Stringify(service->GetPort())+defPath);
			ref<EPDownloadedDefinition> epd = GC::Hold(new EPDownloadedDefinition(NetworkAddress(service->GetHostName()), service->GetPort(), defPath));
			
			epd->EventDownloaded.AddListener(this);
			_downloading.insert(epd);
		}
		
		/*ref<OSCOverUDPConnection> con = GC::Hold(new OSCOverUDPConnection());
		con->Create(data.service->GetHostName(), data.service->GetPort(), DirectionOutbound);
		EventDiscovered.Fire(this, DiscoveryNotification(Timestamp(true), ref<Connection>(con), true));*/
	}
}