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
 
 #import <Cocoa/Cocoa.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>
#include <TJScout/include/tjservice.h>

using namespace tj::ep;
using namespace tj::shared;
using namespace tj::np;
using namespace tj::scout;

class TTDiscovery: public virtual Object, public Listener<DiscoveryNotification>, public Listener<EPStateChangeNotification> {
	public:
		TTDiscovery();
		virtual ~TTDiscovery();
		virtual void OnCreated();
		virtual void Notify(ref<Object> source, const DiscoveryNotification& dn);
		virtual void Notify(ref<Object> source, const EPStateChangeNotification& cn);
		virtual bool GetTagInPreferences(const EPTag& tag, bool& enabled);
		virtual void AddTagToPreferences(const EPTag& tag);
		virtual ref<Service> GetServiceForEndpoint(ref<EPEndpoint> ep);
		virtual ref<Connection> GetConnectionForEndpoint(ref<EPEndpoint> ep);
		virtual ref<EPRemoteState> GetStateForEndpoint(ref<EPEndpoint> ep);
		virtual void UpdateShownEndpoints();
		
		CriticalSection _lock;
		ref<Discovery> _discovery;
		ref<SocketListenerThread> _slt;
		std::multimap< ref<EPEndpoint>, ref<Connection> > _connections;
		std::map< ref<EPEndpoint>, ref<EPRemoteState> > _remoteStates;
		std::set< ref<EPEndpoint> > _endpoints;
		std::set< ref<EPEndpoint> > _shownEndpoints;
		std::map< ref<EPEndpoint>, ref<Service> > _services;
};