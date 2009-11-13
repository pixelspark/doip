#include "../include/epserverconnection.h"
#include "../include/epservermanager.h"
#include <TJNP/include/tjnetworkaddress.h>

using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

/** EPConnectionDefinition **/
EPServerDefinition::EPServerDefinition(): ConnectionDefinition(L"epserver"), _port(WebServer::KPortDontCare) {
}

EPServerDefinition::~EPServerDefinition() {
}

String EPServerDefinition::GetAddress() const {
	return _customPath;
}

String EPServerDefinition::GetFormat() const {
	return L"xml";
}

String EPServerDefinition::GetFraming() const {
	return L"http";
}

unsigned short EPServerDefinition::GetPort() const {
	return _port;
}

void EPServerDefinition::SaveConnection(TiXmlElement* me) {
	SaveAttributeSmall(me, "port", _port);
	SaveAttributeSmall(me, "path", _customPath);
}

void EPServerDefinition::LoadConnection(TiXmlElement* me) {
	_port = LoadAttributeSmall(me, "port", _port);
	_customPath = LoadAttributeSmall<std::wstring>(me, "path", _customPath);
}

/** EPConnection **/
EPServerConnection::EPServerConnection() {
}

EPServerConnection::~EPServerConnection() {
}

void EPServerConnection::CreateForTransport(strong<EPTransport> ept, const NetworkAddress& na) {
	Throw(L"EP server cannot be created as transport", ExceptionTypeError);
}

void EPServerConnection::Create(tj::shared::strong<ConnectionDefinition> def, Direction d, ref<EPEndpoint> parent) {
	if(!parent) {
		Throw(L"EP server cannot be created, no fabric engine was given (an EP server can only be created directly from a fabric definition)", ExceptionTypeError);
	}
	
	if(def.IsCastableTo<EPServerDefinition>()) {
		ref<EPServerDefinition> epd = ref<ConnectionDefinition>(def);
		_server = EPServerManager::Instance()->CreateServer(epd->_port);
		
		if(_server) {
			std::wstring pathPrefix = L"/ep/" + parent->GetFullIdentifier();
			std::wstring definitionPath = pathPrefix + L"/definition";
			
			if(epd->_customPath.length()>0) {
				definitionPath = epd->_customPath;
			}
			
			std::map<std::wstring, std::wstring> attributes;
			attributes[L"EPDefinitionPath"] = definitionPath;
			attributes[L"EPMagicNumber"] = EPServerManager::Instance()->GetServerMagic();
			attributes[L"EPProtocol"] = L"HTTP";
			_server->AddResolver(definitionPath, ref<FileRequestResolver>(GC::Hold(new EPDefinitionResolver(parent))));
			
			// Advertise the service
			unsigned short actualPort = _server->GetActualPort();
			_serviceRegistration = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", parent->GetFriendlyName(), actualPort, attributes);
			Log::Write(L"TJEP/EPConnection", L"EP service active on port "+Stringify(actualPort)+L" definitionPath="+definitionPath);
		}
	}
	else {
		Throw(L"Wrong definition type for EPConnection", ExceptionTypeError);
	}
}

void EPServerConnection::Send(tj::shared::strong<Message> msg, ref<ReplyHandler> rh, ref<ConnectionChannel> cc) {
	Throw(L"Cannot send messages to an EP connection", ExceptionTypeWarning);
}