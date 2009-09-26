#include "../include/tjfabricgroup.h"
#include "../include/tjfabricutil.h"
#include "connections/tjoscudpconnection.h"
using namespace tj::shared;
using namespace tj::fabric;

ref<ConnectionDefinitionFactory> ConnectionDefinitionFactory::_instance;

/** ConnectionDefinition **/
ConnectionDefinition::~ConnectionDefinition() {
}

ConnectionDefinition::ConnectionDefinition(const std::wstring& type): _type(type) {
}

std::wstring ConnectionDefinition::GetType() const {
	return _type;
}

/** ConnectionDefinitionFactory **/
ConnectionDefinitionFactory::ConnectionDefinitionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<connections::OSCOverUDPConnectionDefinition, ConnectionDefinition>(L"OSC-over-UDP")));
}

ConnectionDefinitionFactory::~ConnectionDefinitionFactory() {
}

ref<ConnectionDefinition> ConnectionDefinitionFactory::Load(TiXmlElement* me) {
	std::wstring type = LoadAttributeSmall<std::wstring>(me, "type", L"");
	ref<ConnectionDefinition> cd = CreateObjectOfType(type);
	if(cd) {
		cd->Load(me);
	}
	return cd;
}

void ConnectionDefinitionFactory::Save(strong<ConnectionDefinition> c, TiXmlElement* me) {
	SaveAttributeSmall(me, "type", c->GetType());
	c->Save(me);
}

tj::shared::strong<ConnectionDefinitionFactory> ConnectionDefinitionFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ConnectionDefinitionFactory());
	}
	return _instance;
}

/** Group **/
Group::Group(): _direction(DirectionNone) {
	Clone();
}

Group::~Group() {
}

void Group::Save(TiXmlElement* me) {
	SaveAttributeSmall<std::wstring>(me, "id", _id);
	
	std::wstring dirValue = L"none";
	switch(_direction) {
		case DirectionInbound:
			dirValue = L"in";
			break;
			
		case DirectionOutbound:
			dirValue = L"out";
			break;
		
		case DirectionNone:
		default:
			break;			
	};
	SaveAttributeSmall(me, "direction", dirValue);
	
	// Save connection definitions
	std::deque< ref<ConnectionDefinition> >::iterator rit = _connections.begin();
	while(rit!=_connections.end()) {
		ref<ConnectionDefinition> cd = *rit;
		if(cd) {
			TiXmlElement connElement("connection");
			ConnectionDefinitionFactory::Instance()->Save(cd, &connElement);
			me->InsertEndChild(connElement);
		}
		++rit;
	}
	
	// Save filter patterns
	std::deque<String>::const_iterator it = _filter.begin();
	while(it!=_filter.end()) {
		TiXmlElement filter("filter");
		SaveAttributeSmall(&filter, "pattern", *it);
		me->InsertEndChild(filter);
		++it;
	}
}

void Group::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall<std::wstring>(me, "id", _id);
	std::wstring dirValue = LoadAttributeSmall<std::wstring>(me, "direction", L"");
	if(dirValue==L"in") {
		_direction = DirectionInbound;
	}
	else if(dirValue==L"out") {
		_direction = DirectionOutbound;
	}
	
	// Load connection definitions
	TiXmlElement* connection = me->FirstChildElement("connection");
	while(connection!=0) {
		ref<ConnectionDefinition> cd = ConnectionDefinitionFactory::Instance()->Load(connection);
		if(cd) {
			_connections.push_back(cd);
		}
		else {
			std::wstring type = LoadAttributeSmall<std::wstring>(connection, "type", L"");
			Log::Write(L"TJFabric/Group", std::wstring(L"A connection could not be loaded (probably because the type is not supported). Type was '")+type+std::wstring(L"'."));
		}
		connection = connection->NextSiblingElement("connection");
	}
	
	// Load filter definitions
	TiXmlElement* filter = me->FirstChildElement("filter");
	while(filter!=0) {
		std::wstring pattern = LoadAttributeSmall<std::wstring>(filter, "pattern", L"");
		_filter.push_back(pattern);
		filter = filter->NextSiblingElement("filter");
	}
}

bool Group::PassesFilter(const String& p) const {
	if(_filter.size()==0) {
		return true;
	}
	
	std::deque<String>::const_iterator it = _filter.begin();
	while(it!=_filter.end()) {
		if(Pattern::Matches(*it, p)) {
			return true;
		}
		++it;
	}
	return false;
}

void Group::Clone() {
	_id = Util::RandomIdentifier(L'G');
}

void Group::Clear() {
	Clone();
	_connections.clear();
	_filter.clear();
}

std::wstring Group::GetID() const {
	return _id;
}

Direction Group::GetDirection() const {
	return _direction;
}