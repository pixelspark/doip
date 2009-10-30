#include "../include/tjfabricgroup.h"
#include "../include/tjfabricutil.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

/** Group **/
Group::Group(): _direction(DirectionNone), _lazy(true) {
	Clone();
}

Group::~Group() {
}

void Group::GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const {
	std::deque< ref<ConnectionDefinition> >::const_iterator rit = _connections.begin();
	while(rit!=_connections.end()) {
		ref<ConnectionDefinition> cd = *rit;
		if(cd) {
			transportsList.push_back(cd);
		}
		++rit;
	}
}

void Group::Save(TiXmlElement* me) {
	SaveAttributeSmall<std::wstring>(me, "id", _id);
	SaveAttributeSmall<std::wstring>(me, "prefix", _prefix);
	SaveAttributeSmall<std::wstring>(me, "lazy", Bool::ToString(_lazy));
	
	std::wstring dirValue = L"none";
	switch(_direction) {
		case DirectionInbound:
			dirValue = L"in";
			break;
			
		case DirectionOutbound:
			dirValue = L"out";
			break;
			
		case DirectionBoth:
			dirValue = L"both";
		
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
	
	// Save discovery definitions
	std::deque< ref<DiscoveryDefinition> >::iterator dit = _discoveries.begin();
	while(dit!=_discoveries.end()) {
		ref<DiscoveryDefinition> cd = *dit;
		if(cd) {
			TiXmlElement connElement("discover");
			DiscoveryDefinitionFactory::Instance()->Save(cd, &connElement);
			me->InsertEndChild(connElement);
		}
		++dit;
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

bool Group::IsLazy() const {
	return _lazy;
}

void Group::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall<std::wstring>(me, "id", _id);
	_prefix = LoadAttributeSmall<std::wstring>(me, "prefix", _prefix);
	_lazy = Bool::FromString(LoadAttributeSmall<std::wstring>(me, "lazy", Bool::ToString(_lazy)).c_str());
	std::wstring dirValue = LoadAttributeSmall<std::wstring>(me, "direction", L"");
	
	if(dirValue==L"in") {
		_direction = DirectionInbound;
	}
	else if(dirValue==L"out") {
		_direction = DirectionOutbound;
	}
	else if(dirValue==L"both") {
		_direction = DirectionBoth;
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
	
	// Load discovery definitions
	TiXmlElement* disco = me->FirstChildElement("discover");
	while(disco!=0) {
		ref<DiscoveryDefinition> cd = DiscoveryDefinitionFactory::Instance()->Load(disco);
		if(cd) {
			_discoveries.push_back(cd);
		}
		else {
			std::wstring type = LoadAttributeSmall<std::wstring>(disco, "type", L"");
			Log::Write(L"TJFabric/Group", std::wstring(L"A discovery definition could not be loaded (probably because the type is not supported). Type was '")+type+std::wstring(L"'."));
		}
		disco = disco->NextSiblingElement("disco");
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

String Group::GetID() const {
	return _id;
}

String Group::GetPrefix() const {
	return _prefix;
}

Direction Group::GetDirection() const {
	return _direction;
}