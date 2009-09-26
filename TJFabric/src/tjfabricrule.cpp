#include "../include/tjfabricrule.h"
#include "../include/tjfabricutil.h"
using namespace tj::shared;
using namespace tj::fabric;

Rule::Rule(): _isEnabled(true) {
	Clone();
}

Rule::~Rule() {
}

void Rule::Load(TiXmlElement* me) {
	_id = LoadAttributeSmall<std::wstring>(me, "id", L"");
	_script = LoadAttribute<std::wstring>(me, "script", L"");
	_isEnabled = LoadAttributeSmall<std::wstring>(me, "enabled", Bool::KTrue)==Bool::KTrue;
	
	TiXmlElement* pattern = me->FirstChildElement("pattern");
	while(pattern!=0) {
		TiXmlNode* nd = pattern->FirstChild();
		if(nd!=0) {
			_patterns.insert(Wcs(nd->Value()));
		}
		pattern = pattern->NextSiblingElement("pattern");
	}
}

void Rule::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", _id);
	SaveAttribute(me, "script", _script);
	SaveAttributeSmall<std::wstring>(me, "enabled", _isEnabled ? Bool::KTrue : Bool::KFalse);
	
	std::set<String>::const_iterator it = _patterns.begin();
	while(it!=_patterns.end()) {
		TiXmlElement pattern("pattern");
		pattern.InsertEndChild(TiXmlText(Mbs(*it).c_str()));
		me->InsertEndChild(pattern);
		++it;
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