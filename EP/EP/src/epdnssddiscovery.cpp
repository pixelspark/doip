#include "../include/epdnssddiscovery.h"
#include "../include/eposcipconnection.h"
#include "../include/epservermanager.h"
#include <TJScout/include/tjscout.h>
#include <TJNP/include/tjnetworkaddress.h>
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::scout;
using namespace tj::np;

/** DNSSDRemoteState **/
class DNSSDRemoteState: public virtual Object, public EPRemoteState, 
	public Listener<Service::UpdateNotification>,
	public Listener<EPDownloadedState::EPStateDownloadNotification> {
		
	public:
		DNSSDRemoteState(ref<EPEndpoint> ep, ref<Service> sd);
		virtual ~DNSSDRemoteState();
		virtual void OnCreated();
		virtual void Download();
		virtual void Notify(ref<Object> src, const EPDownloadedState::EPStateDownloadNotification& dn);
		virtual void Notify(ref<Object> src, const Service::UpdateNotification& dn);
		virtual void GetState(EPState::ValueMap& vals);
		virtual tj::shared::Any GetValue(const tj::shared::String& key);
	
	protected:
		CriticalSection _lock;
		String _lastVersion;
		ref<Service> _service;
		EPState::ValueMap _values;
		ref<EPDownloadedState> _state;
};

DNSSDRemoteState::DNSSDRemoteState(ref<EPEndpoint> ep, ref<Service> sd): EPRemoteState(ep), _service(sd) {
}

DNSSDRemoteState::~DNSSDRemoteState() {
}

void DNSSDRemoteState::OnCreated() {
	if(_service) {
		_service->EventUpdate.AddListener(this);
		Download();
	}
}

void DNSSDRemoteState::Download() {
	ThreadLock lock(&_lock);
	
	if(!_state) {
		String currentVersion;
		if(!_service->GetAttribute(L"EPStateVersion", currentVersion)) {
			Throw(L"DNSSDRemoteState: remote service doesn't supply a state version", ExceptionTypeError);
		}
		
		if(_lastVersion!=currentVersion) {
			_lastVersion = currentVersion;
			String remoteStatePath;
			if(!_service->GetAttribute(L"EPStatePath", remoteStatePath)) {
				Throw(L"DNSSDRemoteState: remote service doesn't supply a state path", ExceptionTypeError);
			}
		
			_state = GC::Hold(new EPDownloadedState(_service, remoteStatePath));
			_state->EventDownloaded.AddListener(this);
			_state->Start();
		}
	}
}

void DNSSDRemoteState::Notify(ref<Object> src, const Service::UpdateNotification& dn) {
	Download();
}

void DNSSDRemoteState::Notify(ref<Object> src, const EPDownloadedState::EPStateDownloadNotification& dn) {
	ThreadLock lock(&_lock);
	if(_state) {
		_state->GetState(_values);
		_state = null;
		EPStateChangeNotification en;
		en.remoteState = ref<EPRemoteState>(this);
		EPRemoteState::EventStateChanged.Fire(this, en);
	}
}

void DNSSDRemoteState::GetState(EPState::ValueMap& vm) {
	ThreadLock lock(&_lock);
	vm = _values;
}

Any DNSSDRemoteState::GetValue(const tj::shared::String& key) {
	EPState::ValueMap::const_iterator it = _values.find(key);
	if(it!=_values.end()) {
		return it->second;
	}
	return Any();
}

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
		ref<tj::scout::Service> service = data.service;
		con->Create(service->GetHostName(), service->GetPort(), DirectionOutbound);

		bool GetAttribute(const std::wstring& key, std::wstring& value);
		EPMediationLevel mediationLevel = 0;
		std::wstring mlString;
		if(service->GetAttribute(L"EPMediationLevel", mlString)) {
			mediationLevel = StringTo<EPMediationLevel>(mlString, mediationLevel);
		}
		
		DiscoveryNotification dn(Timestamp(true), ref<Connection>(con), true, mediationLevel);
		EventDiscovered.Fire(this, dn);
	}
}

void DNSSDDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def, const tj::shared::String& ownMagic) {
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

void EPDiscovery::Create(tj::shared::strong<DiscoveryDefinition> def, const String& ownMagic) {
	DNSSDDiscovery::Create(def, ownMagic);
	_ownMagic = ownMagic;
	
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
			// Endpoint does not meet requirements, ignore
		}
		else {
			{
				ThreadLock lock(&_lock);
				_discoveredEndpoints.insert(std::pair< ref<Service>, weak<EPEndpoint> >(data.service, epe));
			}
			
			ref<Connection> connection;
			std::vector< ref<EPTransport> > transportsList;
			epe->GetTransports(transportsList);
			std::vector< ref<EPTransport> >::iterator it = transportsList.begin();
			while(it!=transportsList.end()) {
				ref<EPTransport> trp = *it;
				if(trp) {
					connection = ConnectionFactory::Instance()->CreateForTransport(trp, edd->GetAddress());
					if(connection) {
						{
							ThreadLock lock(&_lock);
							_discovered.insert(std::pair< ref<Service>, weak<Connection> >(data.service, connection));
							_discoveredIDs.insert(data.service->GetID());
						}
						DiscoveryNotification dn(Timestamp(true), connection, true, epe->GetMediationLevel());
						dn.endpoint = epe;
						dn.service = data.service;
						
						// If the remote service supports states, start a download and add a remote state to the notification
						ref<Service> service = edd->GetService();
						if(service) {
							String remoteStatePath;
							String remoteStateVersion;
							if(service->GetAttribute(L"EPStatePath", remoteStatePath) && service->GetAttribute(L"EPStateVersion", remoteStateVersion)) {
								ref<DNSSDRemoteState> dsd = GC::Hold(new DNSSDRemoteState(epe, service));
								dn.remoteState = dsd;
							}
						}
						
						EventDiscovered.Fire(this, dn);
						break;
					}
					else {
						Log::Write(L"EPFramework/EPDiscovery", L"Could not create connection to endpoint; not notifying upstream");
					}
				}
				++it;
			}
			
			if(!connection) {
				Log::Write(L"TJFabric/EPDiscovery", L"Endpoint found that met requirements ("+epe->GetFullIdentifier()+L"), but did not have a compatible transport available");
			}
		}
	}
	
	{
		ThreadLock lock(&_lock);
		_downloading.erase(ref<EPDownloadedDefinition>(src));
	}
}

void EPDiscovery::Notify(tj::shared::ref<Object> src, const tj::scout::ResolveRequest::ServiceNotification& data) {
	ref<Service> service = data.service;
	
	if(!service) {
		Throw(L"EP discovery notification was received, but without a service", ExceptionTypeError);
	}
	
	if(data.online) {
		std::wstring defPath;
		std::wstring magicNumber;
		std::wstring protocol;
		
		// Check whether this is a service that is published from this process
		if(service->GetAttribute(L"EPMagicNumber", magicNumber) && magicNumber==_ownMagic) {
			return;
		}
		
		// Check the protocol that is used in the negotiation phase (i.e. to download definitions)
		if(service->GetAttribute(L"EPProtocol", protocol)) {
			// If there is no protocol, assume HTTP; if there is something else in there, bail out
			if(protocol!=L"HTTP") {
				Log::Write(L"EPFramework/EPDiscovery", L"EP endpoint found ("+service->GetFriendlyName()+L"), but negotiation protocol not supported ('"+protocol+L"')");
				return;
			}
		}
		
		if(service->GetAttribute(L"EPDefinitionPath", defPath)) {
			ThreadLock lock(&_lock);
			Log::Write(L"TJFabric/EPDiscovery", L"Found EP endpoint; definition at http://"+service->GetHostName()+L":"+Stringify(service->GetPort())+defPath);
			ref<EPDownloadedDefinition> epd = GC::Hold(new EPDownloadedDefinition(service, defPath));
			
			epd->EventDownloaded.AddListener(this);
			_downloading.insert(epd);
			epd->Start();
		}
	}
	else {
		DiscoveryNotification dn;
		bool sendNotification = false;
		{
			ThreadLock lock(&_lock);
			// Remove any pending download action
			std::set< ref<EPDownloadedDefinition> >::iterator it = _downloading.begin();
			while(it!=_downloading.end()) {
				ref<EPDownloadedDefinition> edd = *it;
				if(edd && edd->GetService()->GetID()==service->GetID()) {
					edd->Stop();
					edd->EventDownloaded.RemoveListener(this);
					_downloading.erase(it);
					break;
				}
				++it;
			}
			
			// Remove discovered endpoint
			ref<EPEndpoint> removedEndpoint;
			std::map< ref<Service>, weak<EPEndpoint> >::iterator deit = _discoveredEndpoints.begin();
			while(deit!=_discoveredEndpoints.end()) {
				ref<Service> s = deit->first;
				if(s && s->GetID()==service->GetID()) {
					removedEndpoint = deit->second;
					_discoveredEndpoints.erase(deit);
					break;
				}
				
				deit++;
			}
			
			// Remove discovered connection
			ref<Connection> removedConnection;
			std::map< ref<Service>, weak<Connection> >::iterator dit = _discovered.begin();
			while(dit!=_discovered.end()) {
				ref<Service> s = dit->first;
				if(s && s->GetID()==service->GetID()) {
					removedConnection = dit->second;
					_discovered.erase(dit);
					break;
				}
				
				dit++;
			}
			
			// Fire a notification that contains the removed connection (if any) and/or the removed endpoint
			// Both can be null if the definition file was still being downloaded.
			dn = DiscoveryNotification(Timestamp(true), removedConnection, false, EPMediationLevelIgnore);
			dn.endpoint = removedEndpoint;
			std::set<String>::iterator idit = _discoveredIDs.find(service->GetID());
			if(idit!=_discoveredIDs.end()) {
				_discoveredIDs.erase(idit);
				sendNotification = true;
			}
			else {
				Log::Write(L"EPFramework/EPDNSSDDiscovery", L"A service (id="+service->GetID()+L") was removed, but it was never discovered at all; not sending notification");
			}
		}
		
		if(sendNotification) {
			EventDiscovered.Fire(this, dn);
		}
	}
}
