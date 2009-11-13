#include "../include/epconnection.h"
using namespace tj::shared;
using namespace tj::ep;

/** ReplyHandler **/
ReplyHandler::~ReplyHandler() {
}

void ReplyHandler::OnEndReply(strong<Message> orig) {
}

/** ConnectionDefinition **/
ConnectionDefinition::~ConnectionDefinition() {
}

ConnectionDefinition::ConnectionDefinition(const std::wstring& type): _type(type) {
}

std::wstring ConnectionDefinition::GetType() const {
	return _type;
}

/** ConnectionDefinitionFactory **/
ConnectionDefinitionFactory::~ConnectionDefinitionFactory() {
}

ref<ConnectionDefinition> ConnectionDefinitionFactory::Load(TiXmlElement* me) {
	std::wstring type = LoadAttributeSmall<std::wstring>(me, "type", L"");
	ref<ConnectionDefinition> cd = CreateObjectOfType(type);
	if(cd) {
		cd->LoadConnection(me);
	}
	return cd;
}

void ConnectionDefinitionFactory::Save(strong<ConnectionDefinition> c, TiXmlElement* me) {
	SaveAttributeSmall(me, "type", c->GetType());
	c->SaveConnection(me);
}

tj::shared::strong<ConnectionDefinitionFactory> ConnectionDefinitionFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ConnectionDefinitionFactory());
	}
	return _instance;
}

/** ConnectionChannel **/
ConnectionChannel::~ConnectionChannel() {
}

/** Connection **/
Connection::~Connection() {
}

MessageNotification::MessageNotification(const Timestamp& ts, strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc): when(ts), message(m), source(src), channel(cc) {
}

MessageNotification::~MessageNotification() {
}

/** ConnectionFactory **/
ConnectionFactory::~ConnectionFactory() {
}

ref<Connection> ConnectionFactory::CreateForTransport(strong<EPTransport> cd, const tj::np::NetworkAddress& address) {
	std::wstring type = cd->GetType();
	ref<Connection> conn = CreateObjectOfType(type);
	if(conn) {
		conn->CreateForTransport(cd, address);
	}
	return conn;
}

ref<Connection> ConnectionFactory::CreateFromDefinition(strong<ConnectionDefinition> cd, Direction d, ref<EPEndpoint> parent) {
	std::wstring type = cd->GetType();
	ref<Connection> conn = CreateObjectOfType(type);
	if(conn) {
		conn->Create(cd, d, parent);
	}
	return conn;
}

strong< ConnectionFactory > ConnectionFactory::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ConnectionFactory());
	}
	
	return _instance;
}
