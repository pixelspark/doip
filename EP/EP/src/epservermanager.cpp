#include "../include/epservermanager.h"
#include "../include/epmessage.h"

using namespace tj::shared;
using namespace tj::np;
using namespace tj::ep;

ref<EPServerManager> EPServerManager::_instance;

String EPServerManager::GetServerMagic() {
	return _magic;
}

EPServerManager::~EPServerManager() {
}

strong<EPServerManager> EPServerManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new EPServerManager());
	}
	return _instance;
}

ref<WebServer> EPServerManager::CreateServer(unsigned short port) {
	ThreadLock lock(&_lock);
	std::map< unsigned short, ref<WebServer> >::iterator it = _servers.find(port);
	if(it!=_servers.end()) {
		return it->second;
	}
	else {
		unsigned short wsPort = (port==KPortDontCare) ? WebServer::KPortDontCare : port;
		ref<WebServer> ws = GC::Hold(new WebServer(wsPort));
		_servers[port] = ws;
		return ws;
	}
}

EPServerManager::EPServerManager() {
	_magic = Util::RandomIdentifier(L'P');
}

std::ostream& operator<< (std::ostream& out, const TiXmlNode& doc) {
	TiXmlPrinter printer;
	doc.Accept(&printer);
	out << printer.Str();
	return out;
}

EPDefinitionWebItem::EPDefinitionWebItem(ref<EPEndpoint> model): WebItemResource(L"", model->GetFriendlyName(), L"text/xml", 0), _endpoint(model) {
}

EPDefinitionWebItem::~EPDefinitionWebItem() {
}

Resolution EPDefinitionWebItem::Get(ref<WebRequest> frq, std::wstring& error, char** data, Bytes& dataLength) {
	ref<EPEndpoint> endpoint = _endpoint;
	if(!endpoint) {
		error = L"No endpoint in EPDefinitionResolver!";
		return ResolutionNone;
	}
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "no");
	doc.InsertEndChild(decl);
	TiXmlElement dashboardElement("endpoint");
	endpoint->Save(&dashboardElement);
	doc.InsertEndChild(dashboardElement);
	
	std::ostringstream xos;
	xos << doc;
	std::string dataString = xos.str();
	
	dataLength = dataString.length();
	char* nd = new char[(unsigned int)(dataLength+1)];
	const char* dataStringCstr = dataString.c_str();
	for(unsigned int a=0;a<dataLength+1;a++) {
		nd[a] = dataStringCstr[a];
	}
	nd[dataLength] = '\0';
	*data = nd;
	
	return ResolutionData;
}