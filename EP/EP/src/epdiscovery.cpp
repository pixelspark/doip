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
 
 #include "../include/epdiscovery.h"
#include "../include/epconnection.h"
using namespace tj::shared;
using namespace tj::ep;

/** EPRemoteState **/
EPRemoteState::EPRemoteState(ref<EPEndpoint> ep): _ep(ep) {
}

EPRemoteState::~EPRemoteState() {
}

ref<EPEndpoint> EPRemoteState::GetEndpoint() {
	return _ep;
}

/** DiscoveryDefinition **/
DiscoveryDefinition::~DiscoveryDefinition() {
}

DiscoveryDefinition::DiscoveryDefinition(const std::wstring& type): _type(type) {
}

std::wstring DiscoveryDefinition::GetType() const {
	return _type;
}

/** DiscoveryDefinitionFactory **/
DiscoveryDefinitionFactory::~DiscoveryDefinitionFactory() {
}

ref<DiscoveryDefinition> DiscoveryDefinitionFactory::Load(TiXmlElement* me) {
	std::wstring type = LoadAttributeSmall<std::wstring>(me, "type", L"");
	std::wstring format = LoadAttributeSmall<std::wstring>(me, "format", L"osc");
	ref<DiscoveryDefinition> cd = CreateObjectOfType(type);
	if(cd) {
		cd->Load(me);
	}
	return cd;
}

void DiscoveryDefinitionFactory::Save(strong<DiscoveryDefinition> c, TiXmlElement* me) {
	SaveAttributeSmall(me, "type", c->GetType());
	c->Save(me);
}

tj::shared::strong<DiscoveryDefinitionFactory> DiscoveryDefinitionFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new DiscoveryDefinitionFactory());
	}
	return _instance;
}

/** Discovery **/
Discovery::~Discovery() {
}

DiscoveryNotification::DiscoveryNotification(const Timestamp& ts, ref<Connection> m, bool a, EPMediationLevel ml): when(ts), connection(m), added(a), mediationLevel(ml) {
}

DiscoveryNotification::DiscoveryNotification(): added(false), mediationLevel(0), when(true) {
}

/** DiscoveryFactory **/
DiscoveryFactory::~DiscoveryFactory() {
}

ref<Discovery> DiscoveryFactory::CreateFromDefinition(strong<DiscoveryDefinition> cd, const String& ownMagic) {
	std::wstring type = cd->GetType();
	ref<Discovery> conn = CreateObjectOfType(type);
	if(conn) {
		conn->Create(cd, ownMagic);
	}
	return conn;
}

strong< DiscoveryFactory > DiscoveryFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new DiscoveryFactory());
	}
	
	return _instance;
}
