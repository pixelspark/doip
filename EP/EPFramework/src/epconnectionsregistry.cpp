#include "../include/epconnection.h"
#include "../include/epdiscovery.h"
#include "../include/eposcipconnection.h"
#include "../include/epdnssddiscovery.h"
#include "../include/epserverconnection.h"

using namespace tj::shared;
using namespace tj::ep;

ref<ConnectionFactory> ConnectionFactory::_instance;
ref<DiscoveryFactory> DiscoveryFactory::_instance;
ref<ConnectionDefinitionFactory> ConnectionDefinitionFactory::_instance;
ref<DiscoveryDefinitionFactory> DiscoveryDefinitionFactory::_instance;

/** ConnectionFactory **/
ConnectionFactory::ConnectionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<OSCOverUDPConnection, Connection>(L"OSC-over-UDP")));
	RegisterPrototype(L"tcp", GC::Hold(new SubclassedPrototype<OSCOverTCPConnection, Connection>(L"OSC-over-TCP")));
	RegisterPrototype(L"epserver", GC::Hold(new SubclassedPrototype<EPServerConnection, Connection>(L"EP provider")));
}

/** DiscoveryFactory **/
DiscoveryFactory::DiscoveryFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<DNSSDDiscovery, Discovery>(L"DNS-SD/mDNS discovery")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<EPDiscovery, Discovery>(L"EP discovery (over DNS-SD/mDNS)")));
}

/** ConnectionDefinitionFactory **/
ConnectionDefinitionFactory::ConnectionDefinitionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<OSCOverUDPConnectionDefinition, ConnectionDefinition>(L"OSC-over-UDP")));
	RegisterPrototype(L"tcp", GC::Hold(new SubclassedPrototype<OSCOverTCPConnectionDefinition, ConnectionDefinition>(L"OSC-over-TCP")));
	RegisterPrototype(L"epserver", GC::Hold(new SubclassedPrototype<EPServerDefinition, ConnectionDefinition>(L"EP provider")));
}

/** DiscoveryDefinitionFactory **/
DiscoveryDefinitionFactory::DiscoveryDefinitionFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<DNSSDDiscoveryDefinition, DiscoveryDefinition>(L"DNS-SD/mDNS discovery")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<EPDiscoveryDefinition, DiscoveryDefinition>(L"EP discovery (over DNS-SD/mDNS)")));
}
