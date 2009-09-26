#ifndef _TJ_FABRIC_CONNECTION_H
#define _TJ_FABRIC_CONNECTION_H

#include "../../TJShared/include/tjshared.h"
#include "tjfabricgroup.h"

namespace tj {
	namespace fabric {
		class Message;
		
		struct MessageNotification {
			MessageNotification(const tj::shared::Timestamp& ts, tj::shared::strong<Message> m);
			
			tj::shared::Timestamp when;
			tj::shared::strong<Message> message;
		};
		
		class Connection: public virtual tj::shared::Object {
			public:
				virtual ~Connection();
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d) = 0;
				virtual void Send(tj::shared::strong< Message > msg) = 0;
			
				tj::shared::Listenable<MessageNotification> EventMessageReceived;
		};
		
		class ConnectionFactory: public virtual tj::shared::PrototypeBasedFactory< Connection > {
			public:
				virtual ~ConnectionFactory();
				virtual tj::shared::ref<Connection> CreateFromDefinition(tj::shared::strong<ConnectionDefinition> cd, Direction d);
				static tj::shared::strong< ConnectionFactory > Instance();
			
			protected:
				ConnectionFactory();
				static tj::shared::ref< ConnectionFactory > _instance;
		};
		
		class ConnectedGroup: public virtual tj::shared::Object, public tj::shared::Listener<MessageNotification> {
			public:
				ConnectedGroup(tj::shared::strong<Group> g);
				virtual ~ConnectedGroup();
				virtual void Connect(bool t);
				virtual void Send(tj::shared::strong<Message> m);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const MessageNotification& data);
				tj::shared::Listenable<MessageNotification> EventMessageReceived;
			
			protected:
				tj::shared::strong<Group> _group;
				std::map< tj::shared::ref<ConnectionDefinition>, tj::shared::ref<Connection> > _connections;
		};
	}
}

#endif