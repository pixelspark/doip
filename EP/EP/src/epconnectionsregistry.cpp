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
 
 #include "../include/epconnection.h"
#include "../include/epdiscovery.h"
#include "../include/eposcipconnection.h"
#include "../include/epdnssddiscovery.h"

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
}

/** DiscoveryDefinitionFactory **/
DiscoveryDefinitionFactory::DiscoveryDefinitionFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<DNSSDDiscoveryDefinition, DiscoveryDefinition>(L"DNS-SD/mDNS discovery")));
	RegisterPrototype(L"ep", GC::Hold(new SubclassedPrototype<EPDiscoveryDefinition, DiscoveryDefinition>(L"EP discovery (over DNS-SD/mDNS)")));
}
