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
 
 #include "../include/epcondition.h"
#include <TJNP/include/tjpattern.h>
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np::pattern;

/** EPCondition **/
EPCondition::~EPCondition() {
}

/** EPHasTagCondition **/
EPHasTagCondition::EPHasTagCondition(const EPTag& tag): _tag(tag) {
}

EPHasTagCondition::~EPHasTagCondition() {
}

void EPHasTagCondition::Load(TiXmlElement* you) {
	_tag = LoadAttributeSmall(you, "tag", _tag);
}

void EPHasTagCondition::Save(TiXmlElement* you) {
	SaveAttributeSmall(you,"tag", _tag);
}

bool EPHasTagCondition::Matches(strong<EPEndpoint> ep) {
	std::set<EPTag> tags;
	ep->GetTags(tags);
	std::set<EPTag>::const_iterator it = tags.find(_tag);
	return it!=tags.end();
}

/** EPSpecificCondition **/
EPSpecificCondition::EPSpecificCondition(const String& epid, const String& nsp): _epid(epid), _namespace(nsp) {
}

EPSpecificCondition::~EPSpecificCondition() {
}

void EPSpecificCondition::Load(TiXmlElement* you) {
	_epid = LoadAttributeSmall(you, "id", _epid);
	_namespace = LoadAttributeSmall(you, "namespace", _namespace);
}

void EPSpecificCondition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "id", _epid);
	SaveAttributeSmall(me, "namespace", _namespace);
}

bool EPSpecificCondition::Matches(tj::shared::strong<EPEndpoint> ep) {
	if(_namespace.length()>0) {
		if(ep->GetNamespace()!=_namespace) {
			return false;
		}
	}
	
	if(_epid.length()>0) {
		if(ep->GetID()!=_epid) {
			return false;
		}
	}
	
	return true;
}

/** EPSupportsCondition **/
EPSupportsCondition::EPSupportsCondition() {
}

EPSupportsCondition::~EPSupportsCondition() {
}

void EPSupportsCondition::Load(TiXmlElement* me) {
	_pathPattern = LoadAttributeSmall(me, "method", _pathPattern);
	_replyPattern = LoadAttributeSmall(me, "withReply", _replyPattern);
}

void EPSupportsCondition::Save(TiXmlElement* parent) {
	TiXmlElement me("supports");
	SaveAttributeSmall(&me, "method", _pathPattern);
	SaveAttributeSmall(&me, "withReply", _replyPattern);
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
				// Check whether this path matches with our path pattern
				if(Pattern::Match(_pathPattern.c_str(), (*pit).c_str())) {
					// We have a match; now check reply pattern, if there is one
					if(_replyPattern.length()>0) {
						std::vector< ref<EPReply> > replyList;
						method->GetReplies(replyList);
						std::vector< ref<EPReply> >::iterator rit = replyList.begin();
						while(rit!=replyList.end()) {
							ref<EPReply> reply = *it;
							if(reply && Pattern::Match(_replyPattern.c_str(), reply->GetPath().c_str())) {
								return true;
							}
							++rit;
						}

						// None of the replies supported by this method match with the reply pattern
					}
					else {
						// Don't care about supported replies
						return true;
					}
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
	else if(elementName=="specific") {
		ref<EPCondition> epc = GC::Hold(new EPSpecificCondition());
		epc->Load(root);
		return epc;
	}
	else if(elementName=="has-tag") {
		ref<EPCondition> epc = GC::Hold(new EPHasTagCondition());
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