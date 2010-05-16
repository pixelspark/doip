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
 
 #include "../include/tjfabricrule.h"
#include "../include/tjfabricutil.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

/** Variable **/
Variable::Variable() {
}

Variable::~Variable() {
}

void Variable::Load(TiXmlElement* me) {
	String type = LoadAttributeSmall<String>(me, "type", L"");
	String value = LoadAttributeSmall<String>(me, "value", L"");
	_id = LoadAttributeSmall<String>(me, "id", _id);
	_defaultValue = Any(value).Force(Any::TypeFromString(type));
}

void Variable::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", _id);
	SaveAttributeSmall(me, "type", Any::StringFromType(_defaultValue.GetType()));
	SaveAttributeSmall(me, "value", _defaultValue.ToString());
}

void Variable::Clone() {
	_id = Util::RandomIdentifier(L'V');
}

String Variable::GetID() const {
	return _id;
}

Any Variable::GetDefaultValue() const {
	return _defaultValue;
}

/** Rule **/
Rule::Rule(): _isEnabled(true), _isPublic(true) {
	Clone();
}

Rule::~Rule() {
}

String Rule::GetEnabledBinding() const {
	return _enabledBinding;
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
	_description = LoadAttribute<std::wstring>(me,"description", L"");
	_enabledBinding = LoadAttributeSmall(me, "bind-enabled", _enabledBinding);
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
		ref<EPParameterDefinition> param = GC::Hold(new EPParameterDefinition());
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

String Rule::GetDescription() const {
	return _description;
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
	std::deque< ref<EPParameterDefinition> >::const_iterator pit = _parameters.begin();
	while(pit!=_parameters.end()) {
		ref<EPParameterDefinition> param = *pit;
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
	
	std::deque< ref<EPParameterDefinition> >::const_iterator pit = _parameters.begin();
	while(pit!=_parameters.end()) {
		ref<EPParameterDefinition> param = *pit;
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