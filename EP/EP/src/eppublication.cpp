#include "../include/eppublication.h"
#include "../include/epservermanager.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

EPPublication::EPPublication(strong<EPEndpoint> ep, const std::wstring& magicPostfix) {
	_ws = EPServerManager::Instance()->CreateServer(EPServerManager::KPortDontCare);
	ref<EPWebItem> resolver = GC::Hold(new EPWebItem(ep));
	std::wstring basePath = L"/ep/" + ep->GetFullIdentifier();
	_ws->AddResolver(basePath,ref<WebItem>(resolver));
	
	// Create a list of service attributes
	std::map<std::wstring, std::wstring> attributes;
	attributes[L"EPDefinitionPath"] = basePath + L"/" + resolver->GetDefinitionPath();
	attributes[L"EPStatePath"] = basePath + L"/" + resolver->GetStatePath();
	attributes[L"EPProtocol"] = L"HTTP";
	
	// Calculate magic number
	std::wstring serverMagic = EPServerManager::Instance()->GetServerMagic();
	if(magicPostfix.length()>0) {
		serverMagic = serverMagic + L"-" + magicPostfix;
	}
	attributes[L"EPMagicNumber"] = serverMagic;
	
	// Get the port we're running on and publish the service
	unsigned short actualPort = _ws->GetActualPort();
	_reg = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", ep->GetFriendlyName(), actualPort, attributes);
	Log::Write(L"EPFramework/EPPublication", L"EP service active http://localhost:"+Stringify(actualPort)+basePath);
}

EPPublication::~EPPublication() {
}
