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
		
		CriticalSection _lock;
		ref<Discovery> _discovery;
		ref<SocketListenerThread> _slt;
		std::multimap< ref<EPEndpoint>, ref<Connection> > _connections;
		std::map< ref<EPEndpoint>, ref<EPRemoteState> > _remoteStates;
		std::set< ref<EPEndpoint> > _endpoints;
		std::map< ref<EPEndpoint>, ref<Service> > _services;
		NSMutableArray* _methodMenuItems;
};