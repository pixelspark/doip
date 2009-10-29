#include "tjepconnection.h"
#include "../../include/tjfabricserver.h"
#include "../../../../TJNP/include/tjnetworkaddress.h"
using namespace tj::shared;
using namespace tj::np;
using namespace tj::ep;
using namespace tj::fabric;
using namespace tj::scout;
using namespace tj::fabric::connections;

/** EPConnectionDefinition **/
EPConnectionDefinition::EPConnectionDefinition(): ConnectionDefinition(L"epserver"), _port(WebServer::KPortDontCare) {
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

void EPConnection::CreateForTransport(strong<EPTransport> ept, const NetworkAddress& na) {
	Throw(L"EP server cannot be created as transport", ExceptionTypeError);
}

void EPConnection::Create(tj::shared::strong<ConnectionDefinition> def, Direction d, ref<FabricEngine> fe) {
	if(!fe) {
		Throw(L"EP server cannot be created, no fabric engine was given (an EP server can only be created directly from a fabric definition)", ExceptionTypeError);
	}
	
	if(def.IsCastableTo<EPConnectionDefinition>()) {
		ref<EPConnectionDefinition> epd = ref<ConnectionDefinition>(def);
		_server = WebServerManager::Instance()->CreateServer(epd->_port);
		
		if(_server) {
			strong<Fabric> fabric = fe->GetFabric();
			std::wstring pathPrefix = L"/ep/" + fabric->GetFullIdentifier();
			std::wstring definitionPath = pathPrefix + L"/definition";
			
			if(epd->_customPath.length()>0) {
				definitionPath = epd->_customPath;
			}
			
			std::map<std::wstring, std::wstring> attributes;
			attributes[L"EPDefinitionPath"] = definitionPath;
			attributes[L"EPMagicNumber"] = FabricProcess::GetServerMagic();
			_server->AddResolver(definitionPath, ref<FileRequestResolver>(GC::Hold(new FabricDefinitionResolver(fabric))));
			
			if((d & DirectionInbound)!=0) {
				std::wstring messagePath = pathPrefix + L"/message";
				///attributes[L"EPMessagePath"] = messagePath;
				_server->AddResolver(messagePath, ref<FileRequestResolver>(GC::Hold(new FabricMessageResolver(fe, messagePath))));
			}
			
			// Advertise the service
			unsigned short actualPort = _server->GetActualPort();
			_serviceRegistration = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", fabric->GetTitle(), actualPort, attributes);
			Log::Write(L"TJFabric/EPConnection", L"EP service active on port "+Stringify(actualPort)+L" definitionPath="+definitionPath);
		}
	}
	else {
		Throw(L"Wrong definition type for EPConnection", ExceptionTypeError);
	}
}

void EPConnection::Send(tj::shared::strong<Message> msg) {
	Throw(L"Cannot send messages to an EP connection", ExceptionTypeWarning);
}