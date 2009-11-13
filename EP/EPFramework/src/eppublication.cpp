#include "../include/eppublication.h"
#include "../include/epservermanager.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

EPPublication::EPPublication(strong<EPEndpoint> ep) {
	_ws = EPServerManager::Instance()->CreateServer(EPServerManager::KPortDontCare);
	ref<EPDefinitionResolver> resolver = GC::Hold(new EPDefinitionResolver(ep));
	std::wstring definitionPath = L"/ep/" + ep->GetFullIdentifier() + L"/definition.xml";
	_ws->AddResolver(definitionPath,ref<FileRequestResolver>(resolver));
	
	// Advertise the service
	std::map<std::wstring, std::wstring> attributes;
	attributes[L"EPDefinitionPath"] = definitionPath;
	attributes[L"EPProtocol"] = L"HTTP";
	attributes[L"EPMagicNumber"] = EPServerManager::Instance()->GetServerMagic();
	unsigned short actualPort = _ws->GetActualPort();
	_reg = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", ep->GetFriendlyName(), actualPort, attributes);
	Log::Write(L"EPFramework/EPPublication", L"EP service active http://localhost:"+Stringify(actualPort)+definitionPath);
}

EPPublication::~EPPublication() {
}
