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
			
			// TODO FIXME save the scripts!
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

bool Group::GetDiscoveryScript(ref<DiscoveryDefinition> disco, String& scriptSource, bool appear) const {
	std::map< ref<DiscoveryDefinition>,String >::const_iterator it;
	if(appear) {
		it = _appearDiscoveryScripts.find(disco);
		if(it!=_appearDiscoveryScripts.end()) {
			scriptSource = it->second;
			return true;
		}
	}
	else {
		it = _disappearDiscoveryScripts.find(disco);
		if(it!=_disappearDiscoveryScripts.end()) {
			scriptSource = it->second;
			return true;
		}
	}
	
	return false;
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
			// Try to load a discovery script
			std::wstring script = LoadAttribute<std::wstring>(disco, "entry", L"");
			_appearDiscoveryScripts[cd] = script;
			
			std::wstring leaveScript = LoadAttribute<std::wstring>(disco, "exit", L"");
			_disappearDiscoveryScripts[cd] = leaveScript;
			_discoveries.push_back(cd);
		}
		else {
			std::wstring type = LoadAttributeSmall<std::wstring>(disco, "type", L"");
			Log::Write(L"TJFabric/Group", std::wstring(L"A discovery definition could not be loaded (probably because the type is not supported). Type was '")+type+std::wstring(L"'."));
		}
		disco = disco->NextSiblingElement("discover");
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