#import <Cocoa/Cocoa.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>

using namespace tj::ep;
using namespace tj::shared;
using namespace tj::np;

class TTDiscovery: public virtual Object, public Listener<DiscoveryNotification> {
	public:
		TTDiscovery();
		virtual ~TTDiscovery();
		virtual void OnCreated();
		virtual void Notify(ref<Object> source, const DiscoveryNotification& dn);
	
		CriticalSection _lock;
		ref<Discovery> _discovery;
		ref<SocketListenerThread> _slt;
		std::multimap< ref<EPEndpoint>, ref<Connection> > _connections;
		std::set< ref<EPEndpoint> > _endpoints;
};

@interface TJTrayRemoteAppDelegate : NSObject <NSApplicationDelegate, NSMenuDelegate> {
    NSWindow *window;
	NSStatusItem* _item;
	tj::shared::ref<TTDiscovery> _discovery;
}

@property (assign) IBOutlet NSWindow *window;

@end
