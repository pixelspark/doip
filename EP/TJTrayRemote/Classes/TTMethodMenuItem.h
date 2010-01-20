#import <Cocoa/Cocoa.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>

using namespace tj::ep;
using namespace tj::shared;
using namespace tj::np;

@interface TTMethodMenuItem: NSMenuItem {
	ref<EPMethod> _method;
	ref<EPEndpoint> _endpoint;
	ref<Connection> _connection;
	NSMutableArray* _parameterItems;
}

- (id) initWithMethod:(ref<EPMethod>)method endpoint:(ref<EPEndpoint>)ep connection:(ref<Connection>)c state:(ref<EPRemoteState>)rs;
- (ref<EPEndpoint>) endpoint;
- (ref<EPMethod>) method;
- (void) update:(ref<EPRemoteState>)epr;

@end
