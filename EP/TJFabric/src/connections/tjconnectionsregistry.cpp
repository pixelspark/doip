#include "../../include/tjfabricmessage.h"
#include "../../include/tjfabricconnection.h"
#include "tjoscipconnection.h"
#include "tjepconnection.h"
#include "tjdnssddiscovery.h"
#include "tjbetweenfabricconnection.h"
#include "tjepdisovery.h"
using namespace tj::shared;
using namespace tj::fabric;

ref<ConnectionFactory> ConnectionFactory::_instance;
ref<DiscoveryFactory> DiscoveryFactory::_instance;
ref<ConnectionDefinitionFactory> ConnectionDefinitionFactory::_instance;
ref<DiscoveryDefinitionFactory> DiscoveryDefinitionFactory::_instance;

/** ConnectionFactory **/
ConnectionFactory::ConnectionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<connections::OSCOverUDPConnection, Connection>(L"OSC-over-UDP")));
	RegisterPrototype(L"tcp", GC::Hold(new SubclassedPrototype<connections::OSCOverTCPConnection, Connection>(L"OSC-over-TCP")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<connections::EPConnection, Connection>(L"EP provider")));
	RegisterPrototype(L"fabric", GC::Hold(new SubclassedPrototype<connections::BetweenConnection, Connection>(L"Between-fabric")));
}

/** DiscoveryFactory **/
DiscoveryFactory::DiscoveryFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<connections::DNSSDDiscovery, Discovery>(L"DNS-SD/mDNS discovery")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<connections::EPDiscovery, Discovery>(L"EP discovery (over DNS-SD/mDNS)")));
}

/** ConnectionDefinitionFactory **/
ConnectionDefinitionFactory::ConnectionDefinitionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<connections::OSCOverUDPConnectionDefinition, ConnectionDefinition>(L"OSC-over-UDP")));
	RegisterPrototype(L"tcp", GC::Hold(new SubclassedPrototype<connections::OSCOverTCPConnectionDefinition, ConnectionDefinition>(L"OSC-over-TCP")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<connections::EPConnectionDefinition, ConnectionDefinition>(L"EP provider")));
	RegisterPrototype(L"fabric", GC::Hold(new SubclassedPrototype<connections::BetweenConnectionDefinition, ConnectionDefinition>(L"Between-fabric")));
}

/** DiscoveryDefinitionFactory **/
DiscoveryDefinitionFactory::DiscoveryDefinitionFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<connections::DNSSDDiscoveryDefinition, DiscoveryDefinition>(L"DNS-SD/mDNS discovery")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<connections::EPDiscoveryDefinition, DiscoveryDefinition>(L"EP discovery (over DNS-SD/mDNS)")));
}
