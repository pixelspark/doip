#include "../include/tjfabricconnection.h"
#include "../include/tjfabricengine.h"
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::ep;

/** ConnectedGroup **/
ConnectedGroup::ConnectedGroup(strong<Group> g): _group(g), _shouldStillConnectOutbound(false) {
}

ConnectedGroup::~ConnectedGroup() {
}

void ConnectedGroup::Send(strong<Message> m, strong<FabricEngine> fe, ref<ReplyHandler> rh) {
	if((_group->GetDirection() & DirectionOutbound) != 0) {
		if(_group->PassesFilter(m->GetPath())) {
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
					conn->Send(m, rh);
				}
				++it;
			}
			
			// Send to all discovered connections in this group
			std::deque< ref<Connection> >::iterator cit = _discoveredConnections.begin();
			while(cit!=_discoveredConnections.end()) {
				ref<Connection> conn = *cit;
				if(conn) {
					conn->Send(m, rh);
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
	if(data.added) {
		_discoveredConnections.push_back(data.connection);
		
		// If there was a discovery script, run it
		if(source.IsCastableTo<Discovery>()) {
			ThreadLock lock(&_lock);
			String scriptSource;
			
			ref<Discovery> discovery = source;
			std::map< ref<DiscoveryDefinition>, ref<Discovery> >::iterator it = _discoveries.begin();
			while(it!=_discoveries.end()) {
				if(discovery == it->second) {
					ref<DiscoveryDefinition> def = it->first;
					if(_group->GetDiscoveryScript(def, scriptSource)) {
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
	else {
		ThreadLock lock(&_lock);
		
		// remove connection from _discoveredConnections
		std::deque< ref<Connection> >::iterator it = _discoveredConnections.begin();
		while(it!=_discoveredConnections.end()) {
			if(*it == ref<Connection>(data.connection)) {
				_discoveredConnections.erase(it++);
			}
			else {
				++it;
			}
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
						ref<Discovery> conn = DiscoveryFactory::Instance()->CreateFromDefinition(cd);
						
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
