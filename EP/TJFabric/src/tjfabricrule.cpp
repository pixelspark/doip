#include "../include/tjfabricrule.h"
#include "../include/tjfabricutil.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

Rule::Rule(): _isEnabled(true), _isPublic(true) {
	Clone();
}

Rule::~Rule() {
}

void Rule::GetReplies(std::vector< tj::shared::ref<tj::ep::EPReply> >& replyList) const {
	std::vector< ref<EPReply> >::const_iterator it = _replies.begin();
	while(it!=_replies.end()) {
		replyList.push_back(*it);
		++it;
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

	TiXmlElement* reply = me->FirstChildElement("reply");
	while(reply!=0) {
		ref<EPReplyDefinition> erp = GC::Hold(new EPReplyDefinition());
		erp->Load(reply);
		_replies.push_back(erp);
		reply = reply->NextSiblingElement("reply");
	}
}

String Rule::GetFriendlyName() const {
	return _name;
}

void Rule::Save(TiXmlElement* me) {
	EPMethod::Save(me);
}

void Rule::GetPaths(std::set<EPPath>& pathList) const {
	std::set<String>::const_iterator it = _patterns.begin();
	while(it!=_patterns.end()) {
		pathList.insert(*it);
		++it;
	}
}

void Rule::GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const {
	std::deque< ref<Parameter> >::const_iterator pit = _parameters.begin();
	while(pit!=_parameters.end()) {
		ref<Parameter> param = *pit;
		if(param) {
			parameterList.push_back(param);
		}
		++pit;
	}
}

void Rule::SaveRule(TiXmlElement* me) {
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

/** Parameter **/
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

Any Parameter::GetMinimumValue() const {
	return Any(_min).Force(GetValueType());
}

Any Parameter::GetMaximumValue() const {
	return Any(_max).Force(GetValueType());
}

Any Parameter::GetDefaultValue() const {
	return Any(_default).Force(GetValueType());
}
