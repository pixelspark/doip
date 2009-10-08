#include "../include/tjfabricserver.h" 
using namespace tj::shared;
using namespace tj::np;
using namespace tj::fabric;

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
	TiXmlElement dashboardElement("device");
	_fabric->Save(&dashboardElement);
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