#include "../include/tjfabricmessage.h"
#include "../include/tjfabricconnection.h"
#include "connections/tjoscudpconnection.h"
using namespace tj::shared;
using namespace tj::fabric;

ref<ConnectionFactory> ConnectionFactory::_instance;

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
			std::map< ref<ConnectionDefinition> , ref<Connection> >::iterator it = _connections.begin();
			while(it!=_connections.end()) {
				ref<Connection> conn = it->second;
				if(conn) {
					conn->Send(m);
				}
				++it;
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

void ConnectedGroup::Connect(bool t) {
	if(t) {
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
	else {
		_connections.clear();
	}
}
