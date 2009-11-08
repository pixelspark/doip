#include "../include/ependpoint.h"
#include "../../../TJNP/include/tjpattern.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np::pattern;

const wchar_t* EPParameter::KTypeBoolean = L"bool";
const wchar_t* EPParameter::KTypeString = L"string";
const wchar_t* EPParameter::KTypeDouble = L"double";
const wchar_t* EPParameter::KTypeInt32 = L"int32";
const wchar_t* EPParameter::KTypeNull = L"null";

/** EPEndpoint **/
EPEndpoint::~EPEndpoint() {
}

String EPEndpoint::GetFullIdentifier() const {
	String pid = GetID();
	String ns = GetNamespace();
	std::wostringstream wos;
	if(ns.length()>0) {
		wos << ns << L'.';
	}
	wos << pid;
	return wos.str();
}

/** EPReply **/
EPReply::~EPReply() {
}

void EPReply::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "path", GetPath());

	std::vector< ref<EPParameter> > parameterList;
	GetParameters(parameterList);
	std::vector< ref<EPParameter> >::iterator it = parameterList.begin();
	while(it!=parameterList.end()) {
		ref<EPParameter> pp = *it;
		if(pp) {
			TiXmlElement paramElement("parameter");
			pp->Save(&paramElement);
			me->InsertEndChild(paramElement);
		}
		++it;
	}
}

/** EPMethod **/
EPMethod::~EPMethod() {
}

void EPMethod::GetReplies(std::vector< ref<EPReply> >& replyList) const {
}

/** EPParameter **/
EPParameter::~EPParameter() {
}

wchar_t EPParameter::GetValueTypeTag() const {
	Any::Type type = GetValueType();
	switch(type) {
		case Any::TypeString:
			return L's';
			break;
			
		case Any::TypeObject:
			return L'o';
			break;
			
		case Any::TypeInteger:
			return L'i';
			break;
			
		case Any::TypeDouble:
			return L'd';
			break;
			
		case Any::TypeBool:
			return L'T';
			break;
			
		case Any::TypeNull:
		default:
			return L'N';
	}
}

Any::Type EPParameter::GetValueType() const {
	String type = GetType();
	if(type==L"string") {
		return Any::TypeString;
	}
	else if(type==L"bool") {
		return Any::TypeBool;
	}
	else if(type==L"int32") {
		return Any::TypeInteger;
	}
	else if(type==L"double") {
		return Any::TypeDouble;
	}
	else if(type==L"null") {
		return Any::TypeNull;
	}
	else {
		return Any::TypeNull;
	}
}

/** EPTransport **/
EPTransport::~EPTransport() {
}

/** EPEndpointDefinition **/
EPEndpointDefinition::EPEndpointDefinition(): _dynamic(true) {
	Clone();
}

EPEndpointDefinition::~EPEndpointDefinition() {
}

void EPEndpointDefinition::Save(TiXmlElement* me) {
	EPEndpoint::Save(me);
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

void EPEndpoint::Save(TiXmlElement* me) {
	SaveAttributeSmall(me,"id", GetID());
	SaveAttributeSmall(me, "namespace", GetNamespace());
	SaveAttributeSmall(me, "friendly-name", GetFriendlyName());
	SaveAttributeSmall(me, "version", GetVersion());
	SaveAttributeSmall(me, "dynamic", Bool::ToString(IsDynamic()));
	
	TiXmlElement methodsElement("methods");
	std::vector< ref<EPMethod> > methods;
	GetMethods(methods);
	
	std::vector< ref<EPMethod> >::iterator it = methods.begin();
	while(it!=methods.end()) {
		ref<EPMethod> epm = *it;
		if(epm) {
			TiXmlElement method("method");
			epm->Save(&method);
			methodsElement.InsertEndChild(method);
		}
		++it;
	}
	me->InsertEndChild(methodsElement);
	
	TiXmlElement transportsElement("transports");
	std::vector< ref<EPTransport> > transports;
	GetTransports(transports);
	
	std::vector< ref<EPTransport> >::iterator tit = transports.begin();
	while(tit!=transports.end()) {
		ref<EPTransport> ept = *tit;
		if(ept) {
			TiXmlElement transport("transport");
			ept->Save(&transport);
			transportsElement.InsertEndChild(transport);
		}
		++tit;
	}
	me->InsertEndChild(transportsElement);
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

/** EPMethod **/
bool EPMethod::Matches(const String& msg, const String& tags) const {
	if(Matches(msg)) {
		// Create the tag that belongs to these parameters
		std::wostringstream positiveTagStream, negativeTagStream;
		
		std::vector< ref<EPParameter> > parameterList;
		GetParameters(parameterList);
		
		// TODO make this more efficient by simply iterating over both strings and bailing out when not equal
		std::vector< ref<EPParameter> >::const_iterator it = parameterList.begin();
		while(it!=parameterList.end()) {
			ref<EPParameter> param = *it;
			if(param) {
				if(param->GetType()==L"bool") {
					positiveTagStream << L"T";
					negativeTagStream << L"F";
				}
				else {
					positiveTagStream << param->GetValueTypeTag();
					negativeTagStream << param->GetValueTypeTag();
				}
			}
			++it;
		}
		
		return (positiveTagStream.str() == tags) || (negativeTagStream.str() == tags);
	}
	
	return false;
}

bool EPMethod::Matches(const std::wstring& msg) const {
	std::set<EPPath> pathList;
	GetPaths(pathList);
	
	std::set<String>::const_iterator it = pathList.begin();
	while(it!=pathList.end()) {
		if(Pattern::Match((*it).c_str(), msg.c_str())) {
			return true;
		}
		++it;
	}
	return false;
}

void EPMethod::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", GetID());
	SaveAttributeSmall(me, "friendly-name", GetFriendlyName());
	
	std::set<EPPath> paths;
	GetPaths(paths);
	
	std::set<EPPath>::const_iterator it = paths.begin();
	while(it!=paths.end()) {
		TiXmlElement path("path");
		TiXmlText pathText(Mbs(*it));
		path.InsertEndChild(pathText);
		me->InsertEndChild(path);
		++it;
	}

	std::vector< ref<EPReply> > replyList;
	GetReplies(replyList);

	std::vector< ref<EPReply> >::const_iterator rpit = replyList.begin();
	while(rpit!=replyList.end()) {
		ref<EPReply> epr = *rpit;
		if(epr) {
			TiXmlElement replyElement("reply");
			epr->Save(&replyElement);
			me->InsertEndChild(replyElement);
		}
		++rpit;
	}
	
	std::vector< ref<EPParameter> > parameters;
	GetParameters(parameters);
	
	std::vector< ref<EPParameter> >::const_iterator pit = parameters.begin();
	while(pit!=parameters.end()) {
		ref<EPParameter> epp = *pit;
		if(epp) {
			TiXmlElement param("parameter");
			epp->Save(&param);
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

EPParameterDefinition::EPParameterDefinition(const tj::shared::String& friendlyName, const tj::shared::String& type, const tj::shared::String& minValue, const tj::shared::String& maxValue, const tj::shared::String& defaultValue):
	_friendlyName(friendlyName),
	_type(type),
	_minimumValue(minValue),
	_maximumValue(maxValue),
	_defaultValue(defaultValue) {
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

void EPParameterDefinition::Save(TiXmlElement* me) {
	EPParameter::Save(me);
}

void EPParameterDefinition::Load(TiXmlElement* me) {
	_friendlyName = LoadAttributeSmall(me, "friendly-name", _friendlyName);
	_type = LoadAttributeSmall(me, "type", _type);
	_minimumValue = LoadAttributeSmall(me, "min", _minimumValue);
	_maximumValue = LoadAttributeSmall(me, "max", _maximumValue);
	_defaultValue = LoadAttributeSmall(me, "default", _defaultValue);
}

void EPParameter::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "friendly-name", GetFriendlyName());
	SaveAttributeSmall(me, "type", GetType());
	SaveAttributeSmall(me, "min", GetMinimumValue().ToString());
	SaveAttributeSmall(me, "max", GetMaximumValue().ToString());
	SaveAttributeSmall(me, "default", GetDefaultValue().ToString());
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

void EPTransport::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "type", GetType());
	SaveAttributeSmall(me, "format", GetFormat());
	SaveAttributeSmall(me, "address", GetAddress());
	SaveAttributeSmall(me, "framing", GetFraming());
	SaveAttributeSmall(me, "port", int(GetPort()));
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