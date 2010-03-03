#include "../include/tjfabricconnection.h"
#include "../include/tjfabricengine.h"
#include <EP/include/epservermanager.h>
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

class SendMessageTask: public Task, public Recycleable {
	public:
		SendMessageTask(): i(0) {
		}
	
		virtual ~SendMessageTask() {
		}
		
		virtual void OnRecycle() {
			_connection = null;
			_rh = null;
			_message = null;
		}
	
		virtual void OnReuse() {
			Task::OnReuse();
		}
		
		virtual void Run() {
			_connection->Send(_message, _rh);
		}
	
		virtual void Dummy() {
			++i;
		}
	
		int i;
		ref<Connection> _connection;
		ref<ReplyHandler> _rh;
		ref<Message> _message;
};

/** ConnectedGroup **/
ConnectedGroup::ConnectedGroup(strong<Group> g): _group(g), _shouldStillConnectOutbound(false) {
}

ConnectedGroup::~ConnectedGroup() {
}

void ConnectedGroup::Send(strong<Message> m, strong<FabricEngine> fe, ref<ReplyHandler> rh, EPMediationLevel ourOwnMediationLevel) {
	
	if((_group->GetDirection() & DirectionOutbound) != 0) {
		if(_group->PassesFilter(m->GetPath())) {
			strong<Dispatcher> dispatcher = Dispatcher::CurrentOrDefaultInstance();
			ThreadLock lock(&_lock);
			
			// If we still haven't created our connections, do it now
			if(_shouldStillConnectOutbound && _group->IsLazy()) {
				CreateConnections(fe);
			}
			
			// Send to all fixed connections in this group
			std::map< ref<ConnectionDefinition> , ref<Connection> >::iterator it = _connections.begin();
			while(it!=_connections.end()) {
				ref<Connection> conn = it->second;
				if(conn) {
					// Send asynchronously (the former blocking version just did conn->Send(m,rh)
					ref<SendMessageTask> smt = Recycler<SendMessageTask>::Create();
					smt->_connection = conn;
					smt->_rh = rh;
					smt->_message = m;
					dispatcher->Dispatch(ref<Task>(smt));
				}
				++it;
			}
			
			// Send to all discovered connections in this group that have the highest
			// mediation level below ours
			bool ignoreMediationLevel = (ourOwnMediationLevel < 0);
			int highestMediationLevel = 0;
			std::deque< std::pair< EPMediationLevel, ref<Connection> > >::iterator cit = _discoveredConnections.begin();
			
			if(!ignoreMediationLevel) {
				while(cit!=_discoveredConnections.end()) {
					EPMediationLevel epm = cit->first;
					if(epm > highestMediationLevel && epm < ourOwnMediationLevel) {
						highestMediationLevel = epm;
					}
					++cit;
				}
				cit = _discoveredConnections.begin();
			}
			
			while(cit!=_discoveredConnections.end()) {
				if(ignoreMediationLevel || cit->first==highestMediationLevel) {
					ref<Connection> conn = cit->second;
					if(conn) {
						// Send asynchronously (the previous version simply did conn->Send(m,rh)
						ref<SendMessageTask> smt = Recycler<SendMessageTask>::Create();
						smt->_connection = conn;
						smt->_rh = rh;
						smt->_message = m;
						dispatcher->Dispatch(ref<Task>(smt));
					}
				}
				
				++cit;
			}
		}
	}
	else {
		Log::Write(L"TJFabric/ConnectedGroup", std::wstring(L"Cannot send messages to group '")+_group->GetID()+L"': direction is inbound-only.");
	}
}

void ConnectedGroup::Notify(ref<Object> source, const MessageNotification& data) {
	if(_group->PassesFilter(data.message->GetPath())) {
		ref<Message> message = data.message;
		
		// Add prefix
		message->SetPath(_group->GetPrefix()+message->GetPath());
		EventMessageReceived.Fire(this, data);
	}
}

void ConnectedGroup::Notify(ref<Object> source, const DiscoveryNotification& data) {
	ThreadLock lock(&_lock);
	
	if(bool(data.added) && bool(data.connection)) {
		_discoveredConnections.push_back(std::pair<EPMediationLevel, ref<Connection> >(data.mediationLevel, data.connection));
	}
	else {
		// remove connection from _discoveredConnections
		std::deque< std::pair<EPMediationLevel, ref<Connection> > >::iterator it = _discoveredConnections.begin();
		ref<Connection> removedConnection = ref<Connection>(data.connection);
		if(removedConnection) {
			while(it!=_discoveredConnections.end()) {
				if(it->second == removedConnection) {
					_discoveredConnections.erase(it);
					break;
				}
				else {
					++it;
				}
			}
		}
	}
	
	// Queue discovery scripts
	if(source.IsCastableTo<Discovery>()) {
		String scriptSource;
		
		ref<Discovery> discovery = source;
		std::map< ref<DiscoveryDefinition>, ref<Discovery> >::iterator it = _discoveries.begin();
		while(it!=_discoveries.end()) {
			if(discovery == it->second) {
				ref<DiscoveryDefinition> def = it->first;
				if(_group->GetDiscoveryScript(def, scriptSource, data.added)) {
					DiscoveryScriptNotification dn;
					dn.definition = def;
					dn.connection = data.connection;
					dn.scriptSource = scriptSource; 
					EventDiscoveryScript.Fire(this, dn);
				}
				break;
			}
			++it;
		}
	}
}

void ConnectedGroup::CreateConnections(strong<FabricEngine> fe) {
	ThreadLock lock(&_lock);
	
	// Create connections from definitions in Group
	if(_shouldStillConnectOutbound) {
		std::map< ref<ConnectionDefinition> , ref<Connection> > newConnections;
		std::deque< ref<ConnectionDefinition > >::iterator it = _group->_connections.begin();
		while(it!=_group->_connections.end()) {
			ref<ConnectionDefinition> cd = *it;
			if(cd) {
				std::map< ref<ConnectionDefinition>, ref<Connection> >::iterator eit = _connections.find(cd);
				if(eit!=_connections.end()) {
					newConnections[cd] = eit->second;
				}
				else {
					ref<Connection> conn = ConnectionFactory::Instance()->CreateFromDefinition(cd, _group->GetDirection(), fe->GetFabric());
					
					if(conn) {
						conn->EventMessageReceived.AddListener(ref<ConnectedGroup>(this));
						newConnections[cd] = conn;
					}
					else {
						Log::Write(L"TJFabric/ConnectedGroup", L"Could not create connection: ConnectionFactory::CreateFromDefinition failed");
					}
				}
			}
			++it;
		}
		_connections = newConnections;
		_shouldStillConnectOutbound = false;
	}
}

void ConnectedGroup::Connect(bool t, strong<FabricEngine> fe) {
	ThreadLock lock(&_lock);

	if(t) {
		_shouldStillConnectOutbound = true;
		String serverMagic = EPServerManager::Instance()->GetServerMagic() + L"-" + fe->GetFabric()->GetID();
		
		if(!(_group->IsLazy() && _group->GetDirection()==DirectionOutbound)) {
			// Do not connect lazily, instead, do it now
			CreateConnections(fe);
		}
		
		{
			// Create discoveries based on definitions in Group
			std::map< ref<DiscoveryDefinition> , ref<Discovery> > newDiscoveries;
			std::deque< ref<DiscoveryDefinition > >::iterator it = _group->_discoveries.begin();
			while(it!=_group->_discoveries.end()) {
				ref<DiscoveryDefinition> cd = *it;
				if(cd) {
					std::map< ref<DiscoveryDefinition>, ref<Discovery> >::iterator eit = _discoveries.find(cd);
					if(eit!=_discoveries.end()) {
						newDiscoveries[cd] = eit->second;
					}
					else {
						ref<Discovery> conn = DiscoveryFactory::Instance()->CreateFromDefinition(cd, serverMagic);
						
						if(conn) {
							conn->EventDiscovered.AddListener(ref<ConnectedGroup>(this));
							newDiscoveries[cd] = conn;
						}
						else {
							Log::Write(L"TJFabric/ConnectedGroup", L"Could not create discovery: DiscoveryFactory::CreateFromDefinition failed");
						}
					}
				}
				++it;
			}
			_discoveries = newDiscoveries;
		}
		
	}
	else {
		_connections.clear();
		_discoveredConnections.clear();
		_discoveries.clear();
	}
}
