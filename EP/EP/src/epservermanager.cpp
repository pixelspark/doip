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

EPDefinitionResolver::EPDefinitionResolver(ref<EPEndpoint> model): _endpoint(model) {
}

EPDefinitionResolver::~EPDefinitionResolver() {
}

FileRequestResolver::Resolution EPDefinitionResolver::Resolve(ref<FileRequest> frq, std::wstring& file, std::wstring& error, char** data, unsigned int& dataLength) {
	if(!_endpoint) {
		error = L"No endpoint in EPDefinitionResolver!";
		return FileRequestResolver::ResolutionNone;
	}
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "no");
	doc.InsertEndChild(decl);
	TiXmlElement dashboardElement("endpoint");
	_endpoint->Save(&dashboardElement);
	doc.InsertEndChild(dashboardElement);
	
	std::ostringstream xos;
	xos << doc;
	std::string dataString = xos.str();
	
	dataLength = dataString.length();
	char* nd = new char[dataLength+2];
	strncpy(nd, dataString.c_str(), dataLength);
	*data = nd;
	
	return FileRequestResolver::ResolutionData;
}