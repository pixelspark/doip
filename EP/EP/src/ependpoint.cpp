#include "../include/ependpoint.h"
#include <TJNP/include/tjpattern.h>
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

void EPEndpoint::GetTags(std::set<EPTag>& tagList) const {
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

bool EPParameter::IsValueValid(const Any& value) const {
	Any::Type vt = GetValueType();
	Any forced = value.Force(vt);
	if(vt==Any::TypeInteger || vt==Any::TypeDouble) {
		if(GetMaximumValue()>GetMinimumValue()) {
			if(forced > GetMaximumValue()) {
				return false;
			}
			
			if(forced < GetMinimumValue()) {
				return false;
			}
		}
	}
	
	return true;
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

void EPEndpoint::Save(TiXmlElement* me) {
	SaveAttributeSmall(me,"id", GetID());
	SaveAttributeSmall(me, "namespace", GetNamespace());
	SaveAttributeSmall(me, "friendly-name", GetFriendlyName());
	SaveAttributeSmall(me, "version", GetVersion());
	SaveAttributeSmall(me, "dynamic", std::wstring(Bool::ToString(IsDynamic())));
	SaveAttributeSmall(me, "mediation-level", GetMediationLevel());
	
	std::set<EPTag> tags;
	GetTags(tags);
	if(tags.size()>0) {
		TiXmlElement tagsElement("tags");
		std::set<EPTag>::const_iterator it = tags.begin();
		while(it!=tags.end()) {
			TiXmlElement tag("tag");
			TiXmlText text(Mbs(*it));
			tag.InsertEndChild(text);
			tagsElement.InsertEndChild(tag);
			++it;
		}
		me->InsertEndChild(tagsElement);
	}
	
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

bool EPMethod::PersistDefaultValues(strong<Message> msg) {
	// Check if there are enough parameters given
	std::vector< ref<EPParameter> > params;
	GetParameters(params);
	if(msg->GetParameterCount() != params.size()) {
		return false;
	}
	
	// Check each value whether it matches the parameter limits
	std::vector< ref<EPParameter> >::iterator it = params.begin();
	unsigned int idx = 0;
	while(it!=params.end()) {
		ref<EPParameter> param = *it;
		if(param) {
			const Any& value = msg->GetParameter(idx);
			if(!param->IsValueValid(value)) {
				Log::Write(L"EPFramework/EPMethod", L"Message does not match method: parameter idx="+Stringify(idx)+L" invalid value");
			}
			else {
				//Log::Write(L"EPFramework/EPMethod", L"Set default parameter i="+Stringify(idx)+L" v="+value.ToString()+L" t="+Stringify(value.GetType())+L" x="+Stringify(param->GetValueType()));
				param->SetDefaultValue(value);
			}
		}
		++idx;
		++it;
	}
	
	return true;
}

bool EPMethod::Matches(tj::shared::strong<Message> msg) const {
	// Check if the path matches with a pattern
	if(!Matches(msg->GetPath())) {
		return false;
	}
	
	// Check if there are enough parameters given
	std::vector< ref<EPParameter> > params;
	GetParameters(params);
	if(msg->GetParameterCount() != params.size()) {
		//Log::Write(L"EPFramework/EPMethod", L"Message does not match method: parameter count is not equal ("+Stringify(msg->GetParameterCount())+L" vs. "+Stringify(params.size())+L")");
		return false;
	}
	
	// Check each value whether it matches the parameter limits
	std::vector< ref<EPParameter> >::iterator it = params.begin();
	unsigned int idx = 0;
	while(it!=params.end()) {
		ref<EPParameter> param = *it;
		if(param) {
			const Any& value = msg->GetParameter(idx);
			if(!param->IsValueValid(value)) {
				Log::Write(L"EPFramework/EPMethod", L"Message does not match method: parameter idx="+Stringify(idx)+L" invalid value");
				return false;
			}
		}
		++idx;
		++it;
	}
	
	// Well, if the message got at this point, it's probably good...
	return true;
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
	SaveAttribute(me, "description", GetDescription());
	SaveAttributeSmall(me, "bind-enabled", GetEnabledBinding());
	
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

void EPParameter::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "friendly-name", GetFriendlyName());
	SaveAttributeSmall(me, "type", GetType());
	SaveAttributeSmall(me, "min", GetMinimumValue().ToString());
	SaveAttributeSmall(me, "max", GetMaximumValue().ToString());
	SaveAttributeSmall(me, "default", GetDefaultValue().ToString());
	SaveAttributeSmall(me, "bind-value", GetValueBinding());
	
	std::wstring natureString = L"default";
	Nature nature = GetNature();
	if(nature==NatureDiscrete) {
		natureString = L"discrete";
	}
	SaveAttributeSmall(me, "nature", natureString);
	
	std::set<EPOption> options;
	GetOptions(options);
	std::set<EPOption>::const_iterator it = options.begin();
	while(it!=options.end()) {
		TiXmlElement option("option");
		SaveAttributeSmall(&option, "name", it->first);
		SaveAttributeSmall(&option, "value", it->second.ToString());
		me->InsertEndChild(option);
		++it;
	}
}

void EPTransport::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "type", GetType());
	SaveAttributeSmall(me, "format", GetFormat());
	SaveAttributeSmall(me, "address", GetAddress());
	SaveAttributeSmall(me, "framing", GetFraming());
	SaveAttributeSmall(me, "port", int(GetPort()));
}

/** EPState **/
EPState::~EPState() {
}

void EPState::SaveState(TiXmlElement* root) {
	std::map<String,Any> state;
	GetState(state);
	std::map< String, Any>::iterator it = state.begin();
	while(it!=state.end()) {
		TiXmlElement var("var");
		SaveAttributeSmall(&var, "key", it->first);
		it->second.Save(&var);
		root->InsertEndChild(var);
		++it;
	}
}