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
			virtual void Send(tj::shared::strong<tj::ep::Message> m, tj::shared::strong<FabricEngine> fe, tj::shared::ref< tj::ep::ReplyHandler > rh, tj::ep::EPMediationLevel ourOwnLevel = tj::ep::EPMediationLevelIgnore);
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
				std::deque< std::pair<tj::ep::EPMediationLevel, tj::shared::ref<tj::ep::Connection> > > _discoveredConnections;
				std::map< tj::shared::ref<tj::ep::DiscoveryDefinition>, tj::shared::ref<tj::ep::Discovery> > _discoveries;
		};
	}
}

#endif