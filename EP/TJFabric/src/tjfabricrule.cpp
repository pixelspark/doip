#include "../include/tjfabricrule.h"
#include "../include/tjfabricutil.h"
using namespace tj::shared;
using namespace tj::fabric;

Rule::Rule(): _isEnabled(true), _isPublic(true) {
	Clone();
}

Rule::~Rule() {
}

void Rule::SaveEndpointMethodDefinition(TiXmlElement* me) {
	// Only the first pattern is put in the definition as the message path
	std::set<String>::const_iterator it = _patterns.begin();
	if(it!=_patterns.end()) {
		SaveAttributeSmall(me, "id", _id);
		SaveAttributeSmall(me, "friendly-name", _name);
		
		TiXmlElement pattern("path");
		pattern.InsertEndChild(TiXmlText(Mbs(*it).c_str()));
		me->InsertEndChild(pattern);
		
		std::deque< ref<Parameter> >::const_iterator pit = _parameters.begin();
		while(pit!=_parameters.end()) {
			ref<Parameter> param = *pit;
			if(param) {
				TiXmlElement tag("parameter");
				param->Save(&tag);
				me->InsertEndChild(tag);
			}
			++pit;
		}
	}
}

void Rule::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall<std::wstring>(me, "id", L"");
	_script = LoadAttribute<std::wstring>(me, "script", L"");
	_name = LoadAttributeSmall<std::wstring>(me, "name", L"");
	_isEnabled = Bool::FromString(LoadAttributeSmall<std::wstring>(me, "enabled", Bool::ToString(true)).c_str());
	_isPublic = Bool::FromString(LoadAttributeSmall<std::wstring>(me, "public", Bool::ToString(true)).c_str());
	
	TiXmlElement* pattern = me->FirstChildElement("pattern");
	while(pattern!=0) {
		TiXmlNode* nd = pattern->FirstChild();
		if(nd!=0) {
			_patterns.insert(Wcs(nd->Value()));
		}
		pattern = pattern->NextSiblingElement("pattern");
	}
	
	TiXmlElement* tag = me->FirstChildElement("parameter");
	while(tag!=0) {
		ref<Parameter> param = GC::Hold(new Parameter());
		param->Load(tag);
		_parameters.push_back(param);
		tag = tag->NextSiblingElement("parameter");
	}
}

void Rule::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", _id);
	SaveAttribute(me, "script", _script);
	SaveAttributeSmall<std::wstring>(me, "name", _name);
	SaveAttributeSmall<std::wstring>(me, "public", Bool::ToString(_isPublic));
	SaveAttributeSmall<std::wstring>(me, "enabled", Bool::ToString(_isEnabled));
	
	std::set<String>::const_iterator it = _patterns.begin();
	while(it!=_patterns.end()) {
		TiXmlElement pattern("pattern");
		pattern.InsertEndChild(TiXmlText(Mbs(*it).c_str()));
		me->InsertEndChild(pattern);
		++it;
	}
	
	std::deque< ref<Parameter> >::const_iterator pit = _parameters.begin();
	while(pit!=_parameters.end()) {
		ref<Parameter> param = *pit;
		if(param) {
			TiXmlElement tag("parameter");
			param->Save(&tag);
			me->InsertEndChild(tag);
		}
		
		++pit;
	}
}

String Rule::GetID() const {
	return _id;
}

void Rule::Clone() {
	_id = Util::RandomIdentifier(L'R');
}

bool Rule::IsEnabled() const {
	return _isEnabled;
}

bool Rule::IsPublic() const {
	return _isPublic;
}

String Rule::GetScriptSource() const {
	return _script;
}

String Rule::ToString() const {
	std::wostringstream wos;
	std::set<String>::const_iterator it = _patterns.begin();
	wos << _id << L": ";
	while(it!=_patterns.end()) {
		wos << *it;
		++it;
		if(it!=_patterns.end()) {
			wos << L',' << L' ';
		}
	}
	return wos.str();
}

bool Rule::Matches(const std::wstring& msg, const std::wstring& tags) const {
	if(Matches(msg)) {
		// Create the tag that belongs to these parameters
		std::wostringstream positiveTagStream, negativeTagStream;
		
		// TODO make this more efficient by simply iterating over both strings and bailing out when not equal
		std::deque< ref<Parameter> >::const_iterator it = _parameters.begin();
		while(it!=_parameters.end()) {
			ref<Parameter> param = *it;
			if(param) {
				if(param->GetType()==Parameter::KTypeBoolean) {
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

bool Rule::Matches(const std::wstring& msg) const {
	std::set<String>::const_iterator it = _patterns.begin();
	while(it!=_patterns.end()) {
		if(Pattern::Matches(*it, msg)) {
			return true;
		}
		++it;
	}
	return false;
}

/** Parameter **/
const wchar_t* Parameter::KTypeBoolean = L"bool";
const wchar_t* Parameter::KTypeString = L"string";
const wchar_t* Parameter::KTypeDouble = L"double";
const wchar_t* Parameter::KTypeInt32 = L"int32";
const wchar_t* Parameter::KTypeNull = L"null";

Parameter::Parameter() {
}

Parameter::~Parameter() {
}

void Parameter::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "friendly-name", _friendly);
	SaveAttributeSmall(me, "type", _type);
	SaveAttributeSmall(me, "min", _min);
	SaveAttributeSmall(me, "max", _max);
	SaveAttributeSmall(me, "default", _default);
}

void Parameter::Load(TiXmlElement* me) {
	_friendly = LoadAttributeSmall<std::wstring>(me, "friendly-name", _friendly);
	_type = LoadAttributeSmall<std::wstring>(me, "type", _type);
	_min = LoadAttributeSmall<std::wstring>(me, "min", _min);
	_max = LoadAttributeSmall<std::wstring>(me, "max", _max);
	_default = LoadAttributeSmall<std::wstring>(me, "default", _default);
}

std::wstring Parameter::GetFriendlyName() const {
	return _friendly;
}

std::wstring Parameter::GetType() const {
	return _type;
}

Any Parameter::GetMinimum() const {
	return Any(_min).Force(GetValueType());
}

Any Parameter::GetMaximum() const {
	return Any(_max).Force(GetValueType());
}

Any Parameter::GetDefault() const {
	return Any(_default).Force(GetValueType());
}

wchar_t Parameter::GetValueTypeTag() const {
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

Any::Type Parameter::GetValueType() const {
	if(_type==L"string") {
		return Any::TypeString;
	}
	else if(_type==L"bool") {
		return Any::TypeBool;
	}
	else if(_type==L"int32") {
		return Any::TypeInteger;
	}
	else if(_type==L"double") {
		return Any::TypeDouble;
	}
	else if(_type==L"null") {
		return Any::TypeNull;
	}
	else {
		return Any::TypeNull;
	}
}