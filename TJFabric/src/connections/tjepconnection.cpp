#include "tjepconnection.h"
#include "../../include/tjfabricserver.h"
using namespace tj::shared;
using namespace tj::np;
using namespace tj::fabric;
using namespace tj::scout;
using namespace tj::fabric::connections;

/** EPConnectionDefinition **/
EPConnectionDefinition::EPConnectionDefinition(): ConnectionDefinition(L"ep"), _port(7961) {
}

EPConnectionDefinition::~EPConnectionDefinition() {
}

void EPConnectionDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "port", _port);
	SaveAttributeSmall(me, "path", _customPath);
}

void EPConnectionDefinition::Load(TiXmlElement* me) {
	_port = LoadAttributeSmall(me, "port", _port);
	_customPath = LoadAttributeSmall<std::wstring>(me, "path", _customPath);
}

/** EPConnection **/
EPConnection::EPConnection() {
}

EPConnection::~EPConnection() {
}

void EPConnection::Create(tj::shared::strong<ConnectionDefinition> def, Direction d, strong<FabricEngine> fe) {
	if(def.IsCastableTo<EPConnectionDefinition>()) {
		ref<EPConnectionDefinition> epd = ref<ConnectionDefinition>(def);
		_server = WebServerManager::Instance()->CreateServer(epd->_port);
		
		if(_server) {
			strong<Fabric> fabric = fe->GetFabric();
			std::wstring pathPrefix = L"/ep/" + fabric->GetID();
			std::wstring definitionPath = pathPrefix + L"/definition";
			
			if(epd->_customPath.length()>0) {
				definitionPath = epd->_customPath;
			}
			
			std::map<std::wstring, std::wstring> attributes;
			attributes[L"EPDefinitionPath"] = definitionPath;
			_server->AddResolver(definitionPath, ref<FileRequestResolver>(GC::Hold(new FabricDefinitionResolver(fabric))));
			
			if((d & DirectionInbound)!=0) {
				std::wstring messagePath = pathPrefix + L"/message";
				attributes[L"EPMessagePath"] = messagePath;
				_server->AddResolver(messagePath, ref<FileRequestResolver>(GC::Hold(new FabricMessageResolver(fe, messagePath))));
			}
			
			// Advertise the service
			_serviceRegistration = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", fabric->GetTitle(), epd->_port, attributes);
			Log::Write(L"TJFabric/EPConnection", L"EP service active on port "+Stringify(epd->_port)+L" definitionPath="+definitionPath);
		}
	}
	else {
		Throw(L"Wrong definition type for EPConnection", ExceptionTypeError);
	}
}

void EPConnection::Send(tj::shared::strong<Message> msg) {
	Throw(L"Cannot send messages to an EP connection", ExceptionTypeWarning);
}