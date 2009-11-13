#include "../include/epdnssddiscovery.h"
#include "../include/eposcipconnection.h"
#include "../include/epservermanager.h"
#include <TJScout/include/tjscout.h>
#include <TJNP/include/tjnetworkaddress.h>
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::scout;
using namespace tj::np;

/** DNSSDDiscoveryDefinition **/
DNSSDDiscoveryDefinition::DNSSDDiscoveryDefinition(): DiscoveryDefinition(L"dnssd"), _serviceType(L"_osc._udp") {
}

DNSSDDiscoveryDefinition::DNSSDDiscoveryDefinition(const std::wstring& type, const std::wstring& dst): DiscoveryDefinition(type), _serviceType(dst) {
}

DNSSDDiscoveryDefinition::~DNSSDDiscoveryDefinition() {
}

void DNSSDDiscoveryDefinition::Load(TiXmlElement* me) {
	_serviceType = LoadAttributeSmall<std::wstring>(me, "service-type", _serviceType);
}

void DNSSDDiscoveryDefinition::Save(TiXmlElement* me) {
	SaveAttributeSmall(me, "service-type", _serviceType);
}

/** DNSSDDiscovery **/
DNSSDDiscovery::DNSSDDiscovery() {
}

DNSSDDiscovery::~DNSSDDiscovery() {
}

void DNSSDDiscovery::Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data) {
	if(data.online) {
		ref<OSCOverUDPConnection> con = GC::Hold(new OSCOverUDPConnection());
		con->Create(data.service->GetHostName(), data.service->GetPort(), DirectionOutbound);
		EventDiscovered.Fire(this, DiscoveryNotification(Timestamp(true), ref<Connection>(con), true));
	}
}

void DNSSDDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def) {
	if(ref<DiscoveryDefinition>(def).IsCastableTo<DNSSDDiscoveryDefinition>()) {
		ServiceDescription sd;
		ref<DNSSDDiscoveryDefinition> dsd = ref<DiscoveryDefinition>(def);
		if(dsd) {
			sd.AddType(ServiceDiscoveryDNSSD, dsd->_serviceType);
			_resolver = GC::Hold(new ResolveRequest(sd));
			_resolver->EventService.AddListener(ref<DNSSDDiscovery>(this));
			Scout::Instance()->Resolve(_resolver);
		}
	}
}

/** EPDiscoveryDefinition **/
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
			Log::Write(L"TJFabric/EPDiscovery", L"Endpoint does not meet requirements");
		}
		else {
			Log::Write(L"TJFabric/EPDiscovery", L"Found suitable endpoint; friendly name="+epe->GetFriendlyName()+L" fqdn="+epe->GetFullIdentifier());
			
			ref<Connection> connection;
			std::vector< ref<EPTransport> > transportsList;
			epe->GetTransports(transportsList);
			std::vector< ref<EPTransport> >::iterator it = transportsList.begin();
			while(it!=transportsList.end()) {
				ref<EPTransport> trp = *it;
				if(trp) {
					connection = ConnectionFactory::Instance()->CreateForTransport(trp, edd->GetAddress());
					if(connection) {
						// we're done
						EventDiscovered.Fire(this, DiscoveryNotification(Timestamp(true), connection, true));
						break;
					}
				}
				++it;
			}
			
			if(!connection) {
				Log::Write(L"TJFabric/EPDiscovery", L"Endpoint found that met requirements, but did not have a compatible transport available");
			}
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
		std::wstring protocol;
		ref<Service> service = data.service;
		
		// Check whether this is a service that is published from this process
		if(service->GetAttribute(L"EPMagicNumber", magicNumber) && magicNumber==EPServerManager::Instance()->GetServerMagic()) {
			return;
		}
		
		// Check the protocol that is used in the negotiation phase (i.e. to download definitions)
		if(service->GetAttribute(L"EPProtocol", protocol)) {
			// If there is no protocol, assume HTTP; if there is something else in there, bail out
			if(protocol!=L"HTTP") {
				Log::Write(L"EPFramework/EPDiscovery", L"EP endpoint found, but negotiation protocol not supported ('"+protocol+L"')");
				return;
			}
		}
		
		if(service->GetAttribute(L"EPDefinitionPath", defPath)) {
			Log::Write(L"TJFabric/EPDiscovery", L"Found EP endpoint; will download definitions at http://"+service->GetHostName()+L":"+Stringify(service->GetPort())+defPath);
			ref<EPDownloadedDefinition> epd = GC::Hold(new EPDownloadedDefinition(NetworkAddress(service->GetHostName()), service->GetPort(), defPath));
			
			epd->EventDownloaded.AddListener(this);
			_downloading.insert(epd);
		}
	}
}
