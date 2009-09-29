#include "../include/tjfabricmessage.h"
#include "../include/tjfabricconnection.h"
#include "connections/tjoscudpconnection.h"
#include "connections/tjdnssddiscovery.h"
using namespace tj::shared;
using namespace tj::fabric;

ref<ConnectionFactory> ConnectionFactory::_instance;
ref<DiscoveryFactory> DiscoveryFactory::_instance;

/** Connection **/
Connection::~Connection() {
}

MessageNotification::MessageNotification(const Timestamp& ts, strong<Message> m): when(ts), message(m) {
}

/** ConnectionFactory **/
ConnectionFactory::ConnectionFactory() {
	RegisterPrototype(L"udp", GC::Hold(new SubclassedPrototype<connections::OSCOverUDPConnection, Connection>(L"OSC-over-UDP")));
}

ConnectionFactory::~ConnectionFactory() {
}

ref<Connection> ConnectionFactory::CreateFromDefinition(strong<ConnectionDefinition> cd, Direction d) {
	std::wstring type = cd->GetType();
	ref<Connection> conn = CreateObjectOfType(type);
	if(conn) {
		conn->Create(cd, d);
	}
	return conn;
}

strong< ConnectionFactory > ConnectionFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ConnectionFactory());
	}
	
	return _instance;
}

/** ConnectedGroup **/
ConnectedGroup::ConnectedGroup(strong<Group> g): _group(g) {
}

ConnectedGroup::~ConnectedGroup() {
}

void ConnectedGroup::Send(strong<Message> m) {
	if((_group->GetDirection() & DirectionOutbound) != 0) {
		if(_group->PassesFilter(m->GetPath())) {
			// Send to all fixed connections in this group
			std::map< ref<ConnectionDefinition> , ref<Connection> >::iterator it = _connections.begin();
			while(it!=_connections.end()) {
				ref<Connection> conn = it->second;
				if(conn) {
					conn->Send(m);
				}
				++it;
			}
			
			// Send to all discovered connections in this group
			std::deque< ref<Connection> >::iterator cit = _discoveredConnections.begin();
			while(cit!=_discoveredConnections.end()) {
				ref<Connection> conn = *cit;
				if(conn) {
					conn->Send(m);
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
		Log::Write(L"TJFabric/ConnectedGroup", std::wstring(L"Message received: ")+data.message->GetPath());
		EventMessageReceived.Fire(this, data);
	}
}

void ConnectedGroup::Notify(ref<Object> source, const DiscoveryNotification& data) {
	if(data.added) {
		Log::Write(L"TJFabric/ConnectedGroup", std::wstring(L"Connection discovered"));
		_discoveredConnections.push_back(data.connection);
	}
	else {
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

void ConnectedGroup::Connect(bool t) {
	if(t) {
		{
			// Create connections from definitions in Group
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
						ref<Connection> conn = ConnectionFactory::Instance()->CreateFromDefinition(cd, _group->GetDirection());
						
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

/** Discovery **/
Discovery::~Discovery() {
}

DiscoveryNotification::DiscoveryNotification(const Timestamp& ts, strong<Connection> m, bool a): when(ts), connection(m), added(a) {
}

/** DiscoveryFactory **/
DiscoveryFactory::DiscoveryFactory() {
	RegisterPrototype(L"dnssd", GC::Hold(new SubclassedPrototype<connections::DNSSDDiscovery, Discovery>(L"DNS-SD/mDNS discovery")));
}

DiscoveryFactory::~DiscoveryFactory() {
}

ref<Discovery> DiscoveryFactory::CreateFromDefinition(strong<DiscoveryDefinition> cd) {
	std::wstring type = cd->GetType();
	ref<Discovery> conn = CreateObjectOfType(type);
	if(conn) {
		conn->Create(cd);
	}
	return conn;
}

strong< DiscoveryFactory > DiscoveryFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new DiscoveryFactory());
	}
	
	return _instance;
}

