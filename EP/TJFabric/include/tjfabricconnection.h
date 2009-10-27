#ifndef _TJ_FABRIC_CONNECTION_H
#define _TJ_FABRIC_CONNECTION_H

#include "../../../TJShared/include/tjshared.h"
#include "tjfabricgroup.h"
#include "tjfabricmessage.h"

namespace tj {
	namespace fabric {
		class Message;
		class FabricEngine;
		
		struct MessageNotification {
			MessageNotification(const tj::shared::Timestamp& ts, tj::shared::strong<Message> m);
			
			tj::shared::Timestamp when;
			tj::shared::strong<Message> message;
		};
		
		class Connection: public virtual tj::shared::Object {
			public:
				virtual ~Connection();
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::strong<FabricEngine> engine) = 0;
				virtual void Send(tj::shared::strong< Message > msg) = 0;
			
				tj::shared::Listenable<MessageNotification> EventMessageReceived;
		};
		
		class ConnectionFactory: public virtual tj::shared::PrototypeBasedFactory< Connection > {
			public:
				virtual ~ConnectionFactory();
				virtual tj::shared::ref<Connection> CreateFromDefinition(tj::shared::strong<ConnectionDefinition> cd, Direction d, tj::shared::strong<FabricEngine> engine);
				static tj::shared::strong< ConnectionFactory > Instance();
			
			protected:
				ConnectionFactory();
				static tj::shared::ref< ConnectionFactory > _instance;
		};
		
		struct DiscoveryNotification {
			DiscoveryNotification(const tj::shared::Timestamp& ts, tj::shared::strong<Connection> m, bool add);
			
			tj::shared::Timestamp when;
			tj::shared::strong<Connection> connection;
			bool added;
		};
		
		class Discovery: public virtual tj::shared::Object {
			public:
				virtual ~Discovery();
				virtual void Create(tj::shared::strong<DiscoveryDefinition> def) = 0;
				
				tj::shared::Listenable<DiscoveryNotification> EventDiscovered;
		};
		
		class DiscoveryFactory: public virtual tj::shared::PrototypeBasedFactory< Discovery > {
			public:
				virtual ~DiscoveryFactory();
				virtual tj::shared::ref<Discovery> CreateFromDefinition(tj::shared::strong<DiscoveryDefinition> cd);
				static tj::shared::strong< DiscoveryFactory > Instance();
				
			protected:
				DiscoveryFactory();
				static tj::shared::ref< DiscoveryFactory > _instance;
		};
		
		class ConnectedGroup: public virtual tj::shared::Object, public tj::shared::Listener<MessageNotification>, public tj::shared::Listener<DiscoveryNotification>  {
			public:
				ConnectedGroup(tj::shared::strong<Group> g);
				virtual ~ConnectedGroup();
				virtual void Connect(bool t, tj::shared::strong<FabricEngine> fe);
				virtual void Send(tj::shared::strong<Message> m, tj::shared::strong<FabricEngine> fe);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const MessageNotification& data);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const DiscoveryNotification& data);
				tj::shared::Listenable<MessageNotification> EventMessageReceived;
			
			protected:
				virtual void CreateConnections(tj::shared::strong<FabricEngine> fe);
			
				tj::shared::CriticalSection _lock;
				tj::shared::strong<Group> _group;
				bool _shouldStillConnectOutbound;
				std::map< tj::shared::ref<ConnectionDefinition>, tj::shared::ref<Connection> > _connections;
				std::deque< tj::shared::ref<Connection> > _discoveredConnections;
				std::map< tj::shared::ref<DiscoveryDefinition>, tj::shared::ref<Discovery> > _discoveries;
		};
	}
}

#endif