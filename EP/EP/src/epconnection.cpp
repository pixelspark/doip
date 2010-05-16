/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
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
