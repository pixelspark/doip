#include "../include/epdiscovery.h"
#include "../include/epconnection.h"
using namespace tj::shared;
using namespace tj::ep;

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

DiscoveryNotification::DiscoveryNotification(const Timestamp& ts, strong<Connection> m, bool a, EPMediationLevel ml): when(ts), connection(m), added(a), mediationLevel(ml) {
}

/** DiscoveryFactory **/
DiscoveryFactory::~DiscoveryFactory() {
}

ref<Discovery> DiscoveryFactory::CreateFromDefinition(strong<DiscoveryDefinition> cd) {
	std::wstring type = cd->GetType();
	ref<Discovery> conn = CreateObjectOfType(type);
	if(conn) {
		conn->Create(cd);
	}
	return conn;
}

strong< DiscoveryFactory > DiscoveryFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new DiscoveryFactory());
	}
	
	return _instance;
}
