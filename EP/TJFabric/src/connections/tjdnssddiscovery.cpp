#include "tjdnssddiscovery.h"
#include "tjoscipconnection.h"
#include "../../../../TJScout/include/tjscout.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::fabric::connections;
using namespace tj::scout;

DNSSDDiscoveryDefinition::DNSSDDiscoveryDefinition(): DiscoveryDefinition(L"dnssd") {
}

DNSSDDiscoveryDefinition::~DNSSDDiscoveryDefinition() {
}

void DNSSDDiscoveryDefinition::Load(TiXmlElement* me) {
	_serviceType = LoadAttributeSmall<std::wstring>(me, "service-type", _serviceType);
}

void DNSSDDiscoveryDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "service-type", _serviceType);
}

DNSSDDiscovery::DNSSDDiscovery() {
}

DNSSDDiscovery::~DNSSDDiscovery() {
}

void DNSSDDiscovery::Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data) {
	if(data.online) {
		ref<OSCOverUDPConnection> con = GC::Hold(new OSCOverUDPConnection());
		con->Create(data.service->GetHostName(), data.service->GetPort(), DirectionOutbound);
		EventDiscovered.Fire(this, DiscoveryNotification(Timestamp(true), ref<Connection>(con), true));
	}
}

void DNSSDDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def) {
	if(ref<DiscoveryDefinition>(def).IsCastableTo<DNSSDDiscoveryDefinition>()) {
		ServiceDescription sd;
		sd.AddType(ServiceDiscoveryDNSSD, L"_osc._udp");
		_resolver = GC::Hold(new ResolveRequest(sd));
		_resolver->EventService.AddListener(ref<DNSSDDiscovery>(this));
		Scout::Instance()->Resolve(_resolver);
	}
}
