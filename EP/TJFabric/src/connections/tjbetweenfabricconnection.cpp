#include "tjbetweenfabricconnection.h"
#include "../../include/tjfabricregistry.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::fabric::connections;

/** BetweenConnectionDefinition **/
BetweenConnectionDefinition::BetweenConnectionDefinition(): ConnectionDefinition(L"fabric") {
}

BetweenConnectionDefinition::~BetweenConnectionDefinition() {
}

void BetweenConnectionDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "address", _pattern);
}

void BetweenConnectionDefinition::Load(TiXmlElement* me) {
	_pattern = LoadAttributeSmall(me, "address", _pattern);
}

/** BetweenConnection **/
BetweenConnection::BetweenConnection() {
}

BetweenConnection::~BetweenConnection() {
}

void BetweenConnection::Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::strong<FabricEngine> fe) {
	if(def.IsCastableTo<BetweenConnectionDefinition>()) {
		ref<BetweenConnectionDefinition> btd = ref<ConnectionDefinition>(def);
		if(btd) {
			_pattern = btd->_pattern;
			Log::Write(L"TJFabric/BetweenConnection", L"Connection created "+_pattern);
		}
	}
	else {
		Throw(L"Invalid definition type for a between-fabric connection!", ExceptionTypeError);
	}
}

void BetweenConnection::Send(strong<Message> msg) {
	Log::Write(L"TJFabric/BetweenConnection", L"Send "+msg->GetPath()+L" => "+_pattern);
	FabricRegistry::Instance()->Send(_pattern, msg);
}
