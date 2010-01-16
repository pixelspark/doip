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
}

- (id) initWithMethod:(ref<EPMethod>)method endpoint:(ref<EPEndpoint>)ep connection:(ref<Connection>)c;
- (ref<EPEndpoint>) endpoint;
- (ref<EPMethod>) method;

@end
