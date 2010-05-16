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
 
 #include "../include/ependpointdefinition.h"
#include <TJNP/include/tjpattern.h>
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np::pattern;

/** EPEndpointDefinition **/
EPEndpointDefinition::EPEndpointDefinition(): _dynamic(true), _level(0) {
	Clone();
}

EPEndpointDefinition::~EPEndpointDefinition() {
}

bool EPEndpoint::SortsAfter(const EPEndpoint& o) const {
	return GetFriendlyName() > o.GetFriendlyName();
}

void EPEndpointDefinition::Save(TiXmlElement* me) {
	EPEndpoint::Save(me);
}

void EPEndpointDefinition::GetTags(std::set<EPTag>& tagList) const {
	std::set<EPTag>::const_iterator it = _tags.begin();
	while(it!=_tags.end()) {
		tagList.insert(*it);
		++it;
	}
}

void EPEndpointDefinition::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall(me, "id", _id);
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_namespace = LoadAttributeSmall(me, "namespace", _namespace);
	_version = LoadAttributeSmall(me, "version", _version);
	_dynamic = Bool::FromString(LoadAttributeSmall<std::wstring>(me, "dynamic", Bool::ToString(_dynamic)).c_str());
	_level = LoadAttributeSmall(me, "mediation-level", _level);
	
	TiXmlElement* tags = me->FirstChildElement("tags");
	if(tags!=0) {
		TiXmlElement* tag = tags->FirstChildElement("tag");
		while(tag!=0) {
			TiXmlNode* text = tag->FirstChild();
			if(text!=0) {
				_tags.insert(Wcs(text->Value() == 0 ? "" : std::string(text->Value())));
			}
			tag = tag->NextSiblingElement("tag");
		}
	}
	
	TiXmlElement* methods = me->FirstChildElement("methods");
	if(methods!=0) {
		TiXmlElement* method = methods->FirstChildElement("method");
		while(method!=0) {
			ref<EPMethodDefinition> epm = GC::Hold(new EPMethodDefinition());
			epm->Load(method);
			_methods.push_back(epm);
			method = method->NextSiblingElement("method");
		}
	}
	
	TiXmlElement* transports = me->FirstChildElement("transports");
	if(transports!=0) {
		TiXmlElement* transport = transports->FirstChildElement("transport");
		while(transport!=0) {
			ref<EPTransportDefinition> etd = GC::Hold(new EPTransportDefinition());
			etd->Load(transport);
			_transports.push_back(etd);
			transport = transport->NextSiblingElement("transport");
		}
	}
}

void EPEndpointDefinition::GetTransports(std::vector< ref<EPTransport> >& transportsList) const {
	std::vector< ref<EPTransport> >::const_iterator it = _transports.begin();
	while(it!=_transports.end()) {
		transportsList.push_back(*it);
		++it;
	}
}

void EPEndpointDefinition::GetMethods(std::vector< ref<EPMethod> >& methodList) const {
	std::vector< ref<EPMethod> >::const_iterator it = _methods.begin();
	while(it!=_methods.end()) {
		methodList.push_back(*it);
		++it;
	}
}

void EPEndpointDefinition::Clone() {
	_id = Util::RandomIdentifier(L'E');
}

String EPEndpointDefinition::GetID() const {
	return _id;
}

String EPEndpointDefinition::GetNamespace() const {
	return _namespace;
}

String EPEndpointDefinition::GetFullIdentifier() const {
	std::wostringstream wos;
	if(_namespace.length()>0) {
		wos << _namespace << L'.';
	}
	wos << _id;
	return wos.str();
}

String EPEndpointDefinition::GetFriendlyName() const {
	return (_friendlyName.length() > 0) ? _friendlyName : _id;
}

String EPEndpointDefinition::GetVersion() const {
	return _version;
}

bool EPEndpointDefinition::IsDynamic() const {
	return _dynamic;
}

/** EPMethodDefinition **/
EPMethodDefinition::EPMethodDefinition() {
	Clone();
}

EPMethodDefinition::EPMethodDefinition(const String& ids, const String& path, const String& friendly): _id(ids), _friendlyName(friendly) {
	_paths.insert(path);
}

EPMethodDefinition::~EPMethodDefinition() {
}

String EPMethodDefinition::GetEnabledBinding() const {
	return _enabledBinding;
}

String EPMethodDefinition::GetID() const {
	return _id;
}

String EPMethodDefinition::GetFriendlyName() const {
	return _friendlyName;
}

void EPMethodDefinition::GetPaths(std::set<EPPath>& pathList) const {
	std::set<EPPath>::const_iterator it = _paths.begin();
	while(it!=_paths.end()) {
		pathList.insert(*it);
		++it;
	}
}

void EPMethodDefinition::SetID(const tj::shared::String& i) {
	_id = i;
}

void EPMethodDefinition::SetFriendlyName(const tj::shared::String& fn) {
	_friendlyName = fn;
}

void EPMethodDefinition::AddPath(const EPPath& pt) {
	_paths.insert(pt);
}

void EPMethodDefinition::AddParameter(ref<EPParameter> p) {
	_parameters.push_back(p);
}

void EPMethodDefinition::Save(TiXmlElement* me) {
	EPMethod::Save(me);
}

String EPMethodDefinition::GetDescription() const {
	return _description;
}

void EPMethodDefinition::SetDescription(const String& ds) {
	_description = ds;
}

void EPMethodDefinition::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall(me, "id", _id);
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_description = LoadAttribute(me, "description", _description);
	_enabledBinding = LoadAttributeSmall(me, "bind-enabled", _enabledBinding);
	
	TiXmlElement* path = me->FirstChildElement("path");
	while(path!=0) {
		TiXmlNode* text = path->FirstChild();
		if(text!=0) {
			_paths.insert(Wcs(text->Value() == 0 ? "" : std::string(text->Value())));
		}
		path = path->NextSiblingElement("path");
	}
	
	TiXmlElement* param = me->FirstChildElement("parameter");
	while(param!=0) {
		ref<EPParameterDefinition> epp = GC::Hold(new EPParameterDefinition());
		epp->Load(param);
		_parameters.push_back(epp);
		param = param->NextSiblingElement("parameter");
	}
}

void EPMethodDefinition::GetParameters(std::vector< ref<EPParameter> >& parameterList) const {
	std::vector< ref<EPParameter> >::const_iterator it = _parameters.begin();
	while(it!=_parameters.end()) {
		parameterList.push_back(*it);
		++it;
	}
}

void EPMethodDefinition::Clone() {
	Util::RandomIdentifier(L'M');
}

/** EPParameterDefinition **/
EPParameterDefinition::EPParameterDefinition(): _nature(NatureUnknown), _minimumValue(L"0"), _maximumValue(L"-1") {
}

EPParameterDefinition::EPParameterDefinition(const tj::shared::String& friendlyName, const tj::shared::String& type, const tj::shared::String& minValue, const tj::shared::String& maxValue, const tj::shared::String& defaultValue, Nature nature, const String& valueBinding):
_friendlyName(friendlyName),
_type(type),
_minimumValue(minValue),
_maximumValue(maxValue),
_defaultValue(defaultValue),
_nature(nature),
_valueBinding(valueBinding)
{
	
	_runtimeDefaultValue = Any(_defaultValue).Force(GetValueType());
}

EPParameterDefinition::~EPParameterDefinition() {
}

String EPParameterDefinition::GetFriendlyName() const {
	return _friendlyName;
}

String EPParameterDefinition::GetType() const {
	return _type;
}

bool EPParameterDefinition::HasOptions() const {
	return _options.size()>0;
}

void EPParameterDefinition::GetOptions(std::set< EPOption >& optionList) const {
	std::set<EPOption>::const_iterator it = _options.begin();
	while(it!=_options.end()) {
		optionList.insert(*it);
		++it;
	}
}

void EPParameterDefinition::AddOption(const EPOption& epo) {
	_options.insert(epo);
}

Any::Type EPParameterDefinition::GetValueType() const {
	if(_type==L"string") {
		return Any::TypeString;
	}
	else if(_type==L"int32") {
		return Any::TypeInteger;
	}
	else if(_type==L"bool") {
		return Any::TypeBool;
	}
	else if(_type==L"double") {
		return Any::TypeDouble;
	}
	else {
		return Any::TypeNull;
	}
}

Any EPParameterDefinition::GetMinimumValue() const {
	return Any(_minimumValue).Force(GetValueType());
}

Any EPParameterDefinition::GetMaximumValue() const {
	return Any(_maximumValue).Force(GetValueType());
}

Any EPParameterDefinition::GetDefaultValue() const {
	return _runtimeDefaultValue;
}

void EPParameterDefinition::SetDefaultValue(const Any& val) {
	_runtimeDefaultValue = val.Force(GetValueType());
}

void EPParameterDefinition::SetValueBinding(const String& i) {
	_valueBinding = i;
}

String EPParameterDefinition::GetValueBinding() const {
	return _valueBinding;
}

void EPParameterDefinition::Save(TiXmlElement* me) {
	EPParameter::Save(me);
}

void EPParameterDefinition::Load(TiXmlElement* me) {
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_type = LoadAttributeSmall(me, "type", _type);
	_minimumValue = LoadAttributeSmall(me, "min", _minimumValue);
	_maximumValue = LoadAttributeSmall(me, "max", _maximumValue);
	_defaultValue = LoadAttributeSmall(me, "default", _defaultValue);
	_runtimeDefaultValue = Any(_defaultValue).Force(GetValueType());
	_valueBinding = LoadAttributeSmall(me, "bind-value", _valueBinding);
	
	std::wstring natureString = LoadAttributeSmall<std::wstring>(me, "nature", L"");
	if(natureString==L"discrete") {
		_nature = NatureDiscrete;
	}
	else {
		_nature = NatureUnknown;
	}
	
	TiXmlElement* option = me->FirstChildElement("option");
	Any::Type type = GetValueType();
	while(option!=0) {
		EPOption op;
		op.first = LoadAttributeSmall<String>(option, "name", L"");
		op.second = Any(LoadAttributeSmall<String>(option, "value", L"")).Force(type);
		_options.insert(op);
		option = option->NextSiblingElement("option");
	}
}

EPParameter::Nature EPParameterDefinition::GetNature() const {
	return _nature;
}

/** EPTransportDefinition **/
EPTransportDefinition::EPTransportDefinition(): _port(0) {
}

EPTransportDefinition::~EPTransportDefinition() {
}

tj::shared::String EPTransportDefinition::GetType() const {
	return _type;
}

tj::shared::String EPTransportDefinition::GetAddress() const {
	return _address;
}
tj::shared::String EPTransportDefinition::GetFormat() const {
	return _format;
}

tj::shared::String EPTransportDefinition::GetFraming() const {
	return _framing;
}

unsigned short EPTransportDefinition::GetPort() const {
	return _port;
}

/** EPStateDefinition **/
EPStateDefinition::EPStateDefinition() {
}

EPStateDefinition::~EPStateDefinition() {
}

void EPStateDefinition::GetState(EPState::ValueMap& vals) {
	vals = _vals;
}

tj::shared::Any EPStateDefinition::GetValue(const tj::shared::String& key) {
	EPState::ValueMap::const_iterator it = _vals.find(key);
	if(it!=_vals.end()) {
		return it->second;
	}
	return Any();
}

void EPStateDefinition::SetState(EPState::ValueMap& vals) {
	_vals = vals;
}

void EPStateDefinition::SetValue(const tj::shared::String& key, const tj::shared::Any& value) {
	_vals[key] = value;
}

/** EPReplyDefinition **/
EPReplyDefinition::EPReplyDefinition() {
}

EPReplyDefinition::~EPReplyDefinition() {
}

tj::shared::String EPReplyDefinition::GetPath() const {
	return _path;
}

void EPReplyDefinition::GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const {
	std::vector< ref<EPParameter> >::const_iterator it = _parameters.begin();
	while(it!=_parameters.end()) {
		parameterList.push_back(*it);
		++it;
	}
}

void EPReplyDefinition::Load(TiXmlElement* me) {
	_path = LoadAttributeSmall<std::wstring>(me, "path", _path);
	
	TiXmlElement* param = me->FirstChildElement("parameter");
	while(param!=0) {
		ref<EPParameterDefinition> epp = GC::Hold(new EPParameterDefinition());
		epp->Load(param);
		_parameters.push_back(epp);
		param = param->NextSiblingElement("parameter");
	}
}

void EPReplyDefinition::Save(TiXmlElement* me) {
	EPReply::Save(me);	
}

void EPTransportDefinition::Save(TiXmlElement* me) {
	EPTransport::Save(me);
}

void EPTransportDefinition::Load(TiXmlElement* me) {
	_type = LoadAttributeSmall(me, "type", _type);
	_format = LoadAttributeSmall(me, "format", _format);
	_address = LoadAttributeSmall(me, "address", _address);
	_framing = LoadAttributeSmall(me, "framing", _framing);
	_port = LoadAttributeSmall<int>(me, "port", _port);
}

EPMediationLevel EPEndpointDefinition::GetMediationLevel() const {
	return _level;
}

