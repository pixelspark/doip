#include "../include/epcondition.h"
#include "../../../TJNP/include/tjpattern.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np::pattern;

/** EPCondition **/
EPCondition::~EPCondition() {
}

/** EPSupportsCondition **/
EPSupportsCondition::EPSupportsCondition() {
}

void EPSupportsCondition::Load(TiXmlElement* me) {
	_pathPattern = LoadAttributeSmall(me, "path", _pathPattern);
}

void EPSupportsCondition::Save(TiXmlElement* parent) {
	TiXmlElement me("supports");
	SaveAttributeSmall(&me, "path", _pathPattern);
	parent->InsertEndChild(me);
}

bool EPSupportsCondition::Matches(strong<EPEndpoint> ep) {
	std::vector< tj::shared::ref<EPMethod> > methods;
	ep->GetMethods(methods);
	std::vector< ref<EPMethod> >::iterator it = methods.begin();
	while(it!=methods.end()) {
		ref<EPMethod> method = *it;
		if(method) {
			std::set<EPPath> paths;
			method->GetPaths(paths);
			std::set<EPPath>::const_iterator pit = paths.begin();
			while(pit!=paths.end()) {
				if(Pattern::Match(_pathPattern.c_str(), (*pit).c_str())) {
					return true;
				}
				++pit;
			}
		}
		++it;
	}
	return false;
}

/** EPConditionFactory **/
ref<EPCondition> EPConditionFactory::Load(TiXmlElement* root) {
	std::string elementName = root->Value();
	if(elementName=="requires" || elementName=="either") {
		ref<EPCondition> epc = GC::Hold(new EPLogicCondition());
		epc->Load(root);
		return epc;
	}
	else if(elementName=="supports") {
		ref<EPCondition> epc = GC::Hold(new EPSupportsCondition());
		epc->Load(root);
		return epc;
	}
	return null;
}

void Save(tj::shared::strong<EPCondition> ep, TiXmlElement* root) {
	ep->Save(root);
}

/** EPLogicCondition **/
EPLogicCondition::EPLogicCondition(): _op(EPLogicAnd) {
}

EPLogicCondition::~EPLogicCondition() {
}

bool EPLogicCondition::Matches(tj::shared::strong<EPEndpoint> ep) {
	std::deque< ref<EPCondition> >::iterator it = _conditions.begin();

	while(it!=_conditions.end()) {
		ref<EPCondition> kid = *it;
		if(kid) {
			bool r = kid->Matches(ep);
			if(!r && _op==EPLogicAnd) {
				return false;
			}
			
			if(r && _op==EPLogicOr) {
				return true;
			}
		}
		++it;
	}
	
	if(_op==EPLogicAnd) {
		return true;
	}
	return false;
}

void EPLogicCondition::Save(TiXmlElement* me) {
	std::string opName = "";
	switch(_op) {
		default:
		case EPLogicAnd:
			opName = "requires";
			break;
			
		case EPLogicOr:
			opName = "either";
			break;
	}
	
	TiXmlElement el(opName.c_str());
	std::deque< ref<EPCondition> >::iterator it = _conditions.begin();
	
	while(it!=_conditions.end()) {
		ref<EPCondition> kid = *it;
		if(kid) {
			kid->Save(&el);
		}
		++it;
	}
	me->InsertEndChild(el);
}

void EPLogicCondition::Load(TiXmlElement* me) {
	if(me->Value()=="either") {
		_op = EPLogicOr;
	}
	else {
		_op = EPLogicAnd;
	}
	
	TiXmlElement* child = me->FirstChildElement();
	while(child!=0) {
		_conditions.push_back(EPConditionFactory::Load(child));
		child = child->NextSiblingElement();
	}
}

void EPLogicCondition::AddCondition(tj::shared::strong<EPCondition> ep) {
	_conditions.push_back(ep);
}