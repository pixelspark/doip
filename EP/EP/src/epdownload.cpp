#include "../include/epdownload.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

EPDownloadedDefinition::EPDownloadNotification::EPDownloadNotification(ref<EPEndpoint> t, strong<Service> s): endpoint(t), service(s) {
}

EPDownloadedDefinition::EPDownloadedDefinition(strong<Service> service, const tj::shared::String& path):
	Download(NetworkAddress(service->GetHostName()), path, service->GetPort()),
	_service(service) {
		
}

strong<Service> EPDownloadedDefinition::GetService() {
	return _service;
}

EPDownloadedDefinition::~EPDownloadedDefinition() {
	Stop();
}

void EPDownloadedDefinition::OnCreated() {
	Download::OnCreated();
	Start();
}

void EPDownloadedDefinition::OnDownloadComplete(ref<CodeWriter> cw) {
	if(cw) {
		TiXmlDocument doc;
		std::string data((const char*)cw->GetBuffer(), 0, cw->GetSize());
		doc.Parse(data.c_str());
		TiXmlElement* root = doc.FirstChildElement("endpoint");
		if(root!=0) {
			ref<EPEndpointDefinition> epd = GC::Hold(new EPEndpointDefinition());
			epd->Load(root);
			EventDownloaded.Fire(this, EPDownloadNotification(epd, _service));
			return;
		}
		else {
			Log::Write(L"TJEP/EPDownloadedDefinition", L"No root element in this definition file!");
		}
	}

	EventDownloaded.Fire(this, EPDownloadNotification(null, _service));
}