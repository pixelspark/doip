#include "../include/tjfabricserver.h" 
#include "../include/tjfabricmessage.h"
#include "../include/tjfabricqueue.h"
using namespace tj::shared;
using namespace tj::np;
using namespace tj::fabric;

ref<WebServerManager> WebServerManager::_instance;
String FabricProcess::_magic;

String FabricProcess::GetServerMagic() {
	if(_magic.length()==0) {
		_magic = Util::RandomIdentifier(L'P');
	}
	return _magic;
}

WebServerManager::~WebServerManager() {
}

strong<WebServerManager> WebServerManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new WebServerManager());
	}
	return _instance;
}

ref<WebServer> WebServerManager::CreateServer(unsigned short port) {
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

WebServerManager::WebServerManager() {
}

std::ostream& operator<< (std::ostream& out, const TiXmlNode& doc) {
	TiXmlPrinter printer;
	doc.Accept(&printer);
	out << printer.Str();
	return out;
}

FabricDefinitionResolver::FabricDefinitionResolver(ref<Fabric> model): _fabric(model) {
}

FabricDefinitionResolver::~FabricDefinitionResolver() {
}

FileRequestResolver::Resolution FabricDefinitionResolver::Resolve(ref<FileRequest> frq, std::wstring& file, std::wstring& error, char** data, unsigned int& dataLength) {
	if(!_fabric) {
		error = L"No fabric in FabricDefinitionResolver!";
		return FileRequestResolver::ResolutionNone;
	}
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "no");
	doc.InsertEndChild(decl);
	TiXmlElement dashboardElement("endpoint");
	_fabric->SaveEndpointDefinition(&dashboardElement);
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

FabricMessageResolver::FabricMessageResolver(ref<FabricEngine> model, const std::wstring& prefix): _fabric(model), _prefix(prefix) {
}

FabricMessageResolver::~FabricMessageResolver() {
}

FileRequestResolver::Resolution FabricMessageResolver::Resolve(ref<FileRequest> frq, std::wstring& file, std::wstring& error, char** data, unsigned int& dataLength) {
	if(!_fabric) {
		error = L"No fabric engine in FabricMessageResolver!";
		return FileRequestResolver::ResolutionNone;
	}

	std::wstring path = frq->GetPath();
	strong<Message> msg = GC::Hold(new Message(path.substr(_prefix.length())));
	
	// Parse parameters, if this is an HTTP request
	if(frq.IsCastableTo<HTTPRequest>()) {
		ref<HTTPRequest> hrp = frq;
		std::wstring queryString = hrp->GetQueryString();
		if(queryString.length()>0) {
			std::wstring typeTag(queryString, 0, queryString.find_first_of(L','));
			std::wstring parameters(queryString, queryString.find_first_of(L',')+1);
			
			unsigned int idx = 0;
			while(idx < typeTag.length() && parameters.length()>0) {
				wchar_t typeChar = typeTag.at(idx);
				
				if(typeChar==L'T') {
					msg->SetParameter(idx, Any(true));
				}
				else if(typeChar==L'F') {
					msg->SetParameter(idx, Any(false));
				}
				else {
					Any::Type anyType = Any::TypeNull;
					switch(typeChar) {
						case L'i':
							anyType = Any::TypeInteger;
							break;
							
						case L's':
							anyType= Any::TypeString;
							break;
							
						case L'd':
							anyType = Any::TypeDouble;
							break;
							
						default:
						case L'N':
							break;
					}
					
					unsigned int end =  parameters.find_first_of(L',');
					std::wstring parameter(parameters, 0, end);
					msg->SetParameter(idx, Any(parameter).Force(anyType));
					parameters = std::wstring(parameters, end+1);
				}
				++idx;
			}
		}
	}
	
	_fabric->GetQueue()->Add(msg);
	return FileRequestResolver::ResolutionEmpty;
}