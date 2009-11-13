#include "../include/epdownload.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;

EPDownloadedDefinition::EPDownloadNotification::EPDownloadNotification(ref<EPEndpoint> t): endpoint(t) {
}

EPDownloadedDefinition::EPDownloadedDefinition(const tj::np::NetworkAddress& host, unsigned short port, const tj::shared::String& path):
	Download(host, path, port) {
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
			EventDownloaded.Fire(this, EPDownloadNotification(epd));
			return;
		}
		else {
			Log::Write(L"TJEP/EPDownloadedDefinition", L"No root element in this definition file!");
		}
	}

	EventDownloaded.Fire(this, EPDownloadNotification(null));
}