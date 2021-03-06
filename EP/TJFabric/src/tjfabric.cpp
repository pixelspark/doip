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
 
 #include "../include/tjfabric.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricgroup.h"
#include <EP/include/epconnection.h>
#include <EP/include/ependpoint.h>
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

Fabric::Fabric(): _version(0), _mediationLevel(1) {
	Clone();
}

Fabric::~Fabric() {
}

void Fabric::Clear() {
	Clone();
	_rules.clear();
}

void Fabric::Clone() {
	_id = Util::RandomIdentifier(L'F');
}

String Fabric::GetPackage() const {
	return _package;
}

void Fabric::GetTags(std::set<EPTag>& tagList) const {
	std::set<EPTag>::const_iterator it = _tags.begin();
	while(it!=_tags.end()) {
		tagList.insert(*it);
		++it;
	}
}

ref<EPStateDefinition> Fabric::CreateDefaultState() const {
	ref<EPStateDefinition> epd = GC::Hold(new EPStateDefinition());
	std::deque< ref<Variable> >::const_iterator it = _variables.begin();
	while(it!=_variables.end()) {
		ref<Variable> var = *it;
		if(var) {
			epd->SetValue(var->GetID(), var->GetDefaultValue());
		}
		++it;
	}
	return epd;
}

void Fabric::Load(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	_id = LoadAttributeSmall(me, "id", _id);
	_package = LoadAttributeSmall(me, "package", _package);
	_mediationLevel = LoadAttributeSmall(me, "mediation-level", _mediationLevel);
	_author = LoadAttributeSmall<std::wstring>(me, "author", L"");
	_title = LoadAttributeSmall<std::wstring>(me, "title", L"???");
	_version = LoadAttributeSmall<unsigned int>(me, "version", 0);
		
	TiXmlElement* tag = me->FirstChildElement("tag");
	while(tag!=0) {
		TiXmlNode* text = tag->FirstChild();
		if(text!=0) {
			_tags.insert(Wcs(text->Value() == 0 ? "" : std::string(text->Value())));
		}
		tag = tag->NextSiblingElement("tag");
	}
	
	TiXmlElement* group = me->FirstChildElement("group");
	while(group!=0) {
		ref<Group> rr = GC::Hold(new Group());
		rr->Load(group);
		_groups.push_back(rr);
		group = group->NextSiblingElement("group");
	}
	
	TiXmlElement* variable = me->FirstChildElement("variable");
	while(variable!=0) {
		ref<Variable> var = GC::Hold(new Variable());
		var->Load(variable);
		_variables.push_back(var);
		variable = variable->NextSiblingElement("variable");
	}
		
	TiXmlElement* rule = me->FirstChildElement("rule");
	while(rule!=0) {
		ref<Rule> rr = GC::Hold(new Rule());
		rr->Load(rule);
		_rules.push_back(rr);
		rule = rule->NextSiblingElement("rule");
	}
}

void Fabric::Save(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	EPEndpoint::Save(me);
	
	// TODO save rule contents, variable tags, info block, etc.
}

EPMediationLevel Fabric::GetMediationLevel() const {
	return _mediationLevel;
}

void Fabric::GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const {
	std::deque< ref<Rule> >::const_iterator it = _rules.begin();
	while(it!=_rules.end()) {
		ref<Rule> rule = *it;
		if(rule && rule->IsPublic() && rule->IsEnabled()) {
			methodList.push_back(rule);
		}
		++it;
	}
}

void Fabric::GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const {
	std::deque< ref<Group> >::const_iterator it = _groups.begin();
	while(it!=_groups.end()) {
		ref<Group> g = *it;
		if(g && (g->GetDirection() & DirectionInbound)!=0) {
			g->GetTransports(transportsList);
		}
		++it;
	}
}

void Fabric::SaveFabric(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	++_version;
	SaveAttributeSmall(me, "id", _id);

	TiXmlElement infoElement("info");
	SaveAttribute(&infoElement, "author", _author);
	SaveAttribute(&infoElement, "title", _title);
	SaveAttribute(&infoElement, "version", _version);
	
	std::set<EPTag>::const_iterator it = _tags.begin();
	while(it!=_tags.end()) {
		TiXmlElement tag("tag");
		TiXmlText tagName(Mbs(*it));
		tag.InsertEndChild(tagName);
		infoElement.InsertEndChild(tag);
		++it;
	}
	
	me->InsertEndChild(infoElement);
	
	if(_groups.size()>0) {
		TiXmlElement groups("groups");
		std::deque< ref<Group> >::iterator it = _groups.begin();
		while(it!=_groups.end()) {
			ref<Group> g = *it;
			if(g) {
				TiXmlElement group("group");
				g->Save(&group);
				groups.InsertEndChild(group);
			}
			++it;
		}
		me->InsertEndChild(groups);
	}
	
	if(_rules.size()>0) {
		TiXmlElement rules("rules");
		std::deque< ref<Rule> >::iterator it = _rules.begin();
		while(it!=_rules.end()) {
			ref<Rule> rule = *it;
			if(rule) {
				TiXmlElement ruleElement("rule");
				rule->SaveRule(&ruleElement);
				rules.InsertEndChild(ruleElement);
			}
			++it;
		}
		me->InsertEndChild(rules);
	}
}

tj::shared::String Fabric::GetTitle() const {
	return _title;
}

tj::shared::String Fabric::GetAuthor() const {
	return _author;
}

tj::shared::String Fabric::GetID() const {
	return _id;
}

String Fabric::GetVersion() const {
	return Stringify(_version);
}

void Fabric::GetAllMatchingRules(strong<Message> msg, std::deque< ref<Rule> >& results) {
	ThreadLock lock(&_lock);
	
	std::deque< ref<Rule> >::iterator it = _rules.begin();
	while(it!=_rules.end()) {
		ref<Rule> rule = *it;
		if(rule) {
			if(rule->IsEnabled() && rule->Matches(msg)) {
				results.push_back(rule);
			}
		}
		++it;
	}
}

ref<Rule> Fabric::GetFirstMatchingRule(const tj::shared::String& msg) {
	ThreadLock lock(&_lock);
	std::deque< ref<Rule> >::iterator it = _rules.begin();
	while(it!=_rules.end()) {
		ref<Rule> rule = *it;
		if(rule) {
			if(rule->IsEnabled() && rule->Matches(msg)) {
				return rule;
			}
		}
		++it;
	}
	
	return null;
}

tj::shared::String Fabric::GetFriendlyName() const {
	return GetTitle();
}

tj::shared::String Fabric::GetNamespace() const {
	return GetPackage();
}

bool Fabric::IsDynamic() const {
	return true;
}

/** This loads fabric files, while also (recursively) processing <include file="someotherfile.fabric"/> tags.
 FIXME: Add some kind of way to check whether an include file is already loaded (i.e. by passing an 
 std::set<std::string>& as parameter). **/
void Fabric::LoadRecursive(const std::string& path, strong<Fabric> f) {
	std::string dir = Mbs(File::GetDirectory(Wcs(path)));
	TiXmlDocument* doc = new TiXmlDocument(path.c_str());
	doc->LoadFile(TIXML_ENCODING_UTF8);
	TiXmlElement* root = doc->FirstChildElement("fabric");
	if(root!=0) {
		// Check to see if there are include files; if so, load them first
		TiXmlElement* include = root->FirstChildElement("include");
		while(include!=NULL) {
			std::string file = Mbs(LoadAttributeSmall<std::wstring>(include, "file", L""));
			std::ostringstream wos;
			wos << dir << (char)File::GetPathSeparator() << file;
			std::string ipath = wos.str();
			Log::Write(L"TJFabric/Fabric", std::wstring(L"LoadRecursive: loading ")+Wcs(ipath));
			LoadRecursive(ipath, f);
			include = include->NextSiblingElement("include");
		}
		
		/** Includes are loaded first; so, the fabric will have the title and author of the root fabric
		Of course, when saving this fabric, it will become one big XML file, which is probably not the
		cleanest solution. Instead, we should probably implement sub-fabrics or something like that, or
		fabrics that extend eachother. **/
		f->Load(root);
		Log::Write(L"TJFabric/Main", L"Loaded fabric '"+f->GetTitle()+L"' version="+f->GetVersion());
	}
	else {
		Log::Write(L"TJFabric/Main", L"Invalid fabric: '"+Wcs(path)+L"' has no root element <fabric>");
	}
	delete doc;
	
}
