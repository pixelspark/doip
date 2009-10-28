#include "../include/tjfabric.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricgroup.h"
using namespace tj::shared;
using namespace tj::fabric;

Fabric::Fabric(): _version(0) {
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

String Fabric::GetFullIdentifier() const {
	std::wostringstream wos;
	if(_package.length()>0) {
		wos << _package << L'.';
	}
	wos << _id;
	return wos.str();
}

void Fabric::Load(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	_id = LoadAttributeSmall(me, "id", _id);
	_package = LoadAttributeSmall(me, "package", _package);
	
	TiXmlElement* info = me->FirstChildElement("info");
	if(info!=0) {
		_author = LoadAttribute<std::wstring>(info, "author", L"");
		_title = LoadAttribute<std::wstring>(info, "title", L"");
		_version = LoadAttribute<unsigned int>(info, "version", 0);
	}
	
	TiXmlElement* groups = me->FirstChildElement("groups");
	if(groups!=0) {
		TiXmlElement* group = groups->FirstChildElement("group");
		while(group!=0) {
			ref<Group> rr = GC::Hold(new Group());
			rr->Load(group);
			_groups.push_back(rr);
			group = group->NextSiblingElement("group");
		}
	}
	
	TiXmlElement* rules = me->FirstChildElement("rules");
	if(rules!=0) {
		TiXmlElement* rule = rules->FirstChildElement("rule");
		while(rule!=0) {
			ref<Rule> rr = GC::Hold(new Rule());
			rr->Load(rule);
			_rules.push_back(rr);
			rule = rule->NextSiblingElement("rule");
		}
	}
}

void Fabric::SaveEndpointDefinition(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	SaveAttributeSmall(me, "id", _id);
	SaveAttributeSmall(me, "namespace", _package);
	SaveAttributeSmall(me, "version", _version);
	SaveAttributeSmall(me, "dynamic", std::wstring(L"yes"));
	SaveAttributeSmall(me, "friendly-name", _title);
	
	if(_groups.size()>0) {
		TiXmlElement transports("transports");
		std::deque< ref<Group> >::iterator it = _groups.begin();
		while(it!=_groups.end()) {
			ref<Group> group = *it;
			if(group && (group->GetDirection() & DirectionInbound)!=0) {
				group->SaveEndpointDefinition(&transports);
			}
			++it;
		}
		me->InsertEndChild(transports);
	}

	if(_rules.size()>0) {
		TiXmlElement rules("methods");
		std::deque< ref<Rule> >::iterator it = _rules.begin();
		while(it!=_rules.end()) {
			ref<Rule> rule = *it;
			if(rule && rule->IsEnabled() && rule->IsPublic()) {
				TiXmlElement ruleElement("method");
				rule->SaveEndpointMethodDefinition(&ruleElement);
				rules.InsertEndChild(ruleElement);
			}
			++it;
		}
		me->InsertEndChild(rules);
	}
}

void Fabric::Save(TiXmlElement* me) {
	ThreadLock lock(&_lock);
	++_version;
	SaveAttributeSmall(me, "id", _id);

	TiXmlElement infoElement("info");
	SaveAttribute(&infoElement, "author", _author);
	SaveAttribute(&infoElement, "title", _title);
	SaveAttribute(&infoElement, "version", _version);
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
				rule->Save(&ruleElement);
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

unsigned int Fabric::GetVersion() const {
	return _version;
}

void Fabric::GetAllMatchingRules(const String& msg, const String& tags, std::deque< ref<Rule> >& results) {
	ThreadLock lock(&_lock);
	std::deque< ref<Rule> >::iterator it = _rules.begin();
	while(it!=_rules.end()) {
		ref<Rule> rule = *it;
		if(rule) {
			if(rule->IsEnabled() && rule->Matches(msg, tags)) {
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
		Log::Write(L"TJFabric/Main", L"Loaded fabric '"+f->GetTitle()+L"' v"+Stringify((int)f->GetVersion()));
	}
	else {
		Throw(L"Invalid fabric configuration file ", ExceptionTypeError);
	}
	delete doc;
	
}
