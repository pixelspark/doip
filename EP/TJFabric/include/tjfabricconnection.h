#ifndef _TJ_FABRIC_CONNECTION_H
#define _TJ_FABRIC_CONNECTION_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <EP/include/epconnection.h>
#include <EP/include/epmessage.h>
#include <TJNP/include/tjnetworkaddress.h>
#include "tjfabricgroup.h"

namespace tj {
	namespace fabric {
		class FabricEngine;
		
		struct DiscoveryScriptNotification {
			tj::shared::ref<tj::ep::DiscoveryDefinition> definition;
			tj::shared::ref<tj::ep::Connection> connection;
			tj::shared::String scriptSource;
		};
		
		class ConnectedGroup: public virtual tj::shared::Object, public tj::shared::Listener<tj::ep::MessageNotification>, public tj::shared::Listener<tj::ep::DiscoveryNotification>  {
			public:
				ConnectedGroup(tj::shared::strong<Group> g);
				virtual ~ConnectedGroup();
				virtual void Connect(bool t, tj::shared::strong<FabricEngine> fe);
				virtual void Send(tj::shared::strong<tj::ep::Message> m, tj::shared::strong<FabricEngine> fe, tj::shared::ref< tj::ep::ReplyHandler > rh);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const tj::ep::MessageNotification& data);
				virtual void Notify(tj::shared::ref<tj::shared::Object> source, const tj::ep::DiscoveryNotification& data);
			
				tj::shared::Listenable<tj::ep::MessageNotification> EventMessageReceived;
				tj::shared::Listenable<DiscoveryScriptNotification> EventDiscoveryScript;
			
			protected:
				virtual void CreateConnections(tj::shared::strong<FabricEngine> fe);
			
				tj::shared::CriticalSection _lock;
				tj::shared::strong<Group> _group;
				bool _shouldStillConnectOutbound;
				std::map< tj::shared::ref<tj::ep::ConnectionDefinition>, tj::shared::ref<tj::ep::Connection> > _connections;
				std::deque< tj::shared::ref<tj::ep::Connection> > _discoveredConnections;
				std::map< tj::shared::ref<tj::ep::DiscoveryDefinition>, tj::shared::ref<tj::ep::Discovery> > _discoveries;
		};
	}
}

#endif