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

/** EPStateWebItem **/
EPStateWebItem::EPStateWebItem(ref<EPPublication> pub, const String& fn): WebItemResource(fn, fn, L"text/xml", 0), _publication(pub) {
}

EPStateWebItem::~EPStateWebItem() {
}

Resolution EPStateWebItem::Get(ref<WebRequest> frq, std::wstring& error, char** data, Bytes& dataLength) {
	ref<EPPublication> pub = _publication;
	if(!pub) {
		error = L"Publication doesn't exist anymore!";
		return ResolutionNone;
	}
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "no");
	doc.InsertEndChild(decl);
	TiXmlElement stateElement("state");
	pub->SaveState(&stateElement);
	doc.InsertEndChild(stateElement);
	
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

/** EPWebItem **/
const wchar_t* EPWebItem::KDefinitionPath = L"definition.xml";
const wchar_t* EPWebItem::KStatePath = L"state.xml";

EPWebItem::EPWebItem(ref<EPPublication> pub): WebItemResource(L"", pub->GetEndpoint()->GetFriendlyName(), L"text/xml", 0), _publication(pub) {
}

EPWebItem::~EPWebItem() {
}

ref<WebItem> EPWebItem::Resolve(const String& path) {
	bool hasSlash = path.at(0)==L'/';
	ref<EPPublication> pub = _publication;
	if(!pub) {
		return null;
	}
	
	if(path.compare(hasSlash ? 1 : 0, path.length()-(hasSlash?1:0), GetDefinitionPath())==0) {
		return this;
	}
	else if(path.compare(hasSlash ? 1 : 0, path.length()-(hasSlash?1:0), GetStatePath())==0) {
		if(!_stateItem) {
			_stateItem = GC::Hold(new EPStateWebItem(pub, GetStatePath()));
		}
		return _stateItem;
	}
	return null;
}

String EPWebItem::GetStatePath() const {
	return KStatePath;
}

String EPWebItem::GetDefinitionPath() const {
	return KDefinitionPath;
}

Resolution EPWebItem::Get(ref<WebRequest> frq, std::wstring& error, char** data, Bytes& dataLength) {
	ref<EPPublication> pub = _publication;
	if(!pub) {
		error = L"No publication in EPWebItem!";
		return ResolutionNone;
	}
	ref<EPEndpoint> endpoint = pub->GetEndpoint();
	if(!endpoint) {
		error = L"No endpoint in EPWebItem!";
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