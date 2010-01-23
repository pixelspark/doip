#include "../include/epdownload.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

/** EPDownloadedState **/
EPDownloadedState::EPDownloadedState(strong<Service> service, const String& path): 
	Download(NetworkAddress(service->GetHostName()), path, service->GetPort()),
	_service(service) {
}

EPDownloadedState::~EPDownloadedState() {
	Stop();
}

strong<Service> EPDownloadedState::GetService() {
	return _service;
}

void EPDownloadedState::GetState(ValueMap& vals) {
	ThreadLock lock(&_lock);
	vals = _state;
}

Any EPDownloadedState::GetValue(const String& key) {
	EPState::ValueMap::const_iterator it = _state.find(key);
	if(it!=_state.end()) {
		return it->second;
	}
	return Any();
}

void EPDownloadedState::LoadState(TiXmlElement* root) {
	ThreadLock lock(&_lock);
	
	TiXmlElement* var = root->FirstChildElement("var");
	while(var!=0) {
		String key = LoadAttributeSmall(var, "key", String(L""));
		String val = LoadAttributeSmall(var, "value", String(L"???"));
		Any value;
		value.Load(var);
		_state[key] = value;
		var = var->NextSiblingElement("var");
	}	
}

void EPDownloadedState::OnDownloadComplete(ref<DataWriter> cw) {
	Download::OnDownloadComplete(cw);
	if(cw) {
		TiXmlDocument doc;
		std::string data((const char*)cw->GetBuffer(), 0, (unsigned int)cw->GetSize());
		doc.Parse(data.c_str());
		TiXmlElement* root = doc.FirstChildElement("state");
		if(root!=0) {
			LoadState(root);
			EventDownloaded.Fire(this, EPStateDownloadNotification());
			return;
		}
		else {
			Log::Write(L"TJEP/EPDownloadedState", L"No root element in this definition file! (path="+_path+L")");
		}
	}
	
	EventDownloaded.Fire(this, EPStateDownloadNotification());
}


/** EPDownloadedDefinition **/
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

ref<EPEndpoint> EPDownloadedDefinition::GetEndpoint() {
	return _createdEndpoint;
}

void EPDownloadedDefinition::OnCreated() {
	Download::OnCreated();
	Start();
}

void EPDownloadedDefinition::OnDownloadComplete(ref<DataWriter> cw) {
	Download::OnDownloadComplete(cw);
	if(cw) {
		TiXmlDocument doc;
		std::string data((const char*)cw->GetBuffer(), 0, (unsigned int)cw->GetSize());
		doc.Parse(data.c_str());
		TiXmlElement* root = doc.FirstChildElement("endpoint");
		if(root!=0) {
			ref<EPEndpointDefinition> epd = GC::Hold(new EPEndpointDefinition());
			epd->Load(root);
			_createdEndpoint = epd;
			EventDownloaded.Fire(this, EPDownloadNotification(_createdEndpoint, _service));
			return;
		}
		else {
			Log::Write(L"TJEP/EPDownloadedDefinition", L"No root element in this definition file!");
		}
	}

	EventDownloaded.Fire(this, EPDownloadNotification(null, _service));
}
