#include "tjepdiscovery.h"
#include "../../include/tjfabricengine.h"
#include "../../include/tjfabricserver.h"

using namespace tj::shared;
using namespace tj::np;
using namespace tj::ep;
using namespace tj::fabric;
using namespace tj::scout;
using namespace tj::fabric::connections;

EPDiscoveryDefinition::EPDiscoveryDefinition(): DNSSDDiscoveryDefinition(L"ep", L"_ep._tcp") {
}

EPDiscoveryDefinition::~EPDiscoveryDefinition() {
}

void EPDiscoveryDefinition::Load(TiXmlElement* me) {
	TiXmlElement* requires = me->FirstChildElement("requires");
	if(requires!=0) {
		_condition = EPConditionFactory::Load(requires);
	}
	else {
		_condition = null;
	}
}

void EPDiscoveryDefinition::Save(TiXmlElement* me) {
	if(_condition) {
		TiXmlElement requires("requires");
		_condition->Save(&requires);
		me->InsertEndChild(requires);
	}
}

/** EPDiscovery **/
EPDiscovery::EPDiscovery() {
}

EPDiscovery::~EPDiscovery() {
}

void EPDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def) {
	DNSSDDiscovery::Create(def);
	
	if(def.IsCastableTo<EPDiscoveryDefinition>()) {
		ref<EPDiscoveryDefinition> edd = ref<DiscoveryDefinition>(def);
		if(edd) {
			_condition = edd->_condition;
		}
	}
}

void EPDiscovery::Notify(ref<Object> src, const EPDownloadedDefinition::EPDownloadNotification& data) {
	ref<EPDownloadedDefinition> edd = src;
	ref<EPEndpoint> epe = data.endpoint;
	
	if(epe) {
		if(_condition && !_condition->Matches(epe)) {
			//Log::Write(L"TJFabric/EPDiscovery", L"Endpoint does not meet requirements");
		}
		else {
			Log::Write(L"TJFabric/EPDiscovery", L"Found suitable endpoint; friendly name="+epe->GetFriendlyName()+L" fqdn="+epe->GetFullIdentifier());
			
			std::vector< ref<EPTransport> > transportsList;
			epe->GetTransports(transportsList);
			std::vector< ref<EPTransport> >::iterator it = transportsList.begin();
			while(it!=transportsList.end()) {
				ref<EPTransport> trp = *it;
				if(trp) {
					ref<Connection> con = ConnectionFactory::Instance()->CreateForTransport(trp, edd->GetAddress());
					if(con) {
						// we're done
						EventDiscovered.Fire(this, DiscoveryNotification(Timestamp(true), con, true));
						break;
					}
				}
				++it;
			}
			
			Log::Write(L"TJFabric/EPDiscovery", L"Endpoint found that met requirements, but did not have a compatible transport available");
		}
	}
	
	{
		ThreadLock lock(&_lock);
		_downloading.erase(ref<EPDownloadedDefinition>(src));
	}
}

void EPDiscovery::Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data) {
	ThreadLock lock(&_lock);
	
	if(data.online) {
		std::wstring defPath;
		std::wstring magicNumber;
		ref<Service> service = data.service;
		
		if(service->GetAttribute(L"EPMagicNumber", magicNumber) && magicNumber==FabricProcess::GetServerMagic()) {
			return;
		}
		
		if(service->GetAttribute(L"EPDefinitionPath", defPath)) {
			Log::Write(L"TJFabric/EPDiscovery", L"Found EP endpoint; will download definitions at http://"+service->GetHostName()+L":"+Stringify(service->GetPort())+defPath);
			ref<EPDownloadedDefinition> epd = GC::Hold(new EPDownloadedDefinition(NetworkAddress(service->GetHostName()), service->GetPort(), defPath));
			
			epd->EventDownloaded.AddListener(this);
			_downloading.insert(epd);
		}
	}
}