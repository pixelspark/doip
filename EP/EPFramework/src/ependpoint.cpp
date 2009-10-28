#include "../include/ependpoint.h"
using namespace tj::shared;
using namespace tj::ep;

/** EPEndpoint **/
EPEndpoint::~EPEndpoint() {
}

/** EPMethod **/
EPMethod::~EPMethod() {
}

/** EPParameter **/
EPParameter::~EPParameter() {
}

/** EPTransport **/
EPTransport::~EPTransport() {
}

/** EPEndpointDefinition **/
EPEndpointDefinition::EPEndpointDefinition() {
	Clone();
}

EPEndpointDefinition::~EPEndpointDefinition() {
}

void EPEndpointDefinition::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall(me, "id", _id);
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_namespace = LoadAttributeSmall(me, "namespace", _namespace);
	_version = LoadAttributeSmall(me, "version", _version);
	_dynamic = Bool::FromString(LoadAttributeSmall<std::wstring>(me, "dynamic", Bool::ToString(_dynamic)).c_str());
	
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

	TiXmlElement* transports = me->FirstChildElement("transport");
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

void EPEndpointDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me,"id", _id);
	SaveAttributeSmall(me, "namespace", _namespace);
	SaveAttributeSmall(me, "friendly-name", _friendlyName);
	SaveAttributeSmall(me, "version", _version);
	SaveAttributeSmall(me, "dynamic", Bool::ToString(_dynamic));
	
	TiXmlElement methods("methods");
	std::vector< ref<EPMethod> >::iterator it = _methods.begin();
	while(it!=_methods.end()) {
		ref<EPMethod> epm = *it;
		if(epm && epm.IsCastableTo<EPMethodDefinition>()) {
			TiXmlElement method("method");
			ref<EPMethodDefinition>(epm)->Save(&method);
			methods.InsertEndChild(method);
		}
		++it;
	}
	me->InsertEndChild(methods);
	
	TiXmlElement transports("transports");
	std::vector< ref<EPTransport> >::iterator tit = _transports.begin();
	while(tit!=_transports.end()) {
		ref<EPTransport> ept = *tit;
		if(ept && ept.IsCastableTo<EPTransportDefinition>()) {
			TiXmlElement transport("transport");
			ref<EPTransportDefinition>(ept)->Save(&transport);
			transports.InsertEndChild(transport);
		}
		++tit;
	}
	me->InsertEndChild(transports);
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
	return _friendlyName;
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

EPMethodDefinition::~EPMethodDefinition() {
}

String EPMethodDefinition::GetID() const {
	return _id;
}

String EPMethodDefinition::GetFriendlyName() const {
	return _friendlyName;
}

void  EPMethodDefinition::GetPaths(std::set<EPPath>& pathList) const {
	std::set<EPPath>::const_iterator it = _paths.begin();
	while(it!=_paths.end()) {
		pathList.insert(*it);
	}
}

void EPMethodDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", _id);
	SaveAttributeSmall(me, "friendly-name", _friendlyName);
	
	std::set<EPPath>::const_iterator it = _paths.begin();
	while(it!=_paths.end()) {
		TiXmlElement path("path");
		TiXmlText pathText(Mbs(*it));
		path.InsertEndChild(pathText);
		me->InsertEndChild(path);
	}
	
	std::vector< ref<EPParameter> >::const_iterator pit = _parameters.begin();
	while(pit!=_parameters.end()) {
		ref<EPParameter> epp = *pit;
		if(epp && epp.IsCastableTo<EPParameterDefinition>()) {
			TiXmlElement param("param");
			ref<EPParameterDefinition>(epp)->Save(&param);
			me->InsertEndChild(param);
		}
		++pit;
	}
}

void EPMethodDefinition::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall(me, "id", _id);
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	
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
EPParameterDefinition::EPParameterDefinition() {
}

EPParameterDefinition::~EPParameterDefinition() {
}

String EPParameterDefinition::GetFriendlyName() const {
	return _friendlyName;
}

String EPParameterDefinition::GetType() const {
	return _type;
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
	return Any(_defaultValue).Force(GetValueType());
}

void EPParameterDefinition::Load(TiXmlElement* me) {
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_type = LoadAttributeSmall(me, "type", _type);
	_minimumValue = LoadAttributeSmall(me, "min", _minimumValue);
	_maximumValue = LoadAttributeSmall(me, "max", _maximumValue);
	_defaultValue = LoadAttributeSmall(me, "default", _defaultValue);
}

void EPParameterDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "friendly-name", _friendlyName);
	SaveAttributeSmall(me, "type", _type);
	SaveAttributeSmall(me, "min", _minimumValue);
	SaveAttributeSmall(me, "max", _maximumValue);
	SaveAttributeSmall(me, "default", _defaultValue);
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

void EPTransportDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "type", _type);
	SaveAttributeSmall(me, "format", _format);
	SaveAttributeSmall(me, "address", _address);
	SaveAttributeSmall(me, "framing", _framing);
	SaveAttributeSmall(me, "port", int(_port));
}

void EPTransportDefinition::Load(TiXmlElement* me) {
	_type = LoadAttributeSmall(me, "type", _type);
	_format = LoadAttributeSmall(me, "format", _format);
	_address = LoadAttributeSmall(me, "address", _address);
	_framing = LoadAttributeSmall(me, "framing", _framing);
	_port = LoadAttributeSmall<int>(me, "port", _port);
}