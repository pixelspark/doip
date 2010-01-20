#import <Cocoa/Cocoa.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>

using namespace tj::ep;
using namespace tj::shared;
using namespace tj::np;

@interface TTParameterMenuItem: NSMenuItem {
	ref<EPParameter> _parameter;
	NSView* _dataView;
	NSTextField* _valueLabel;
}

- (void) update:(ref<EPRemoteState>)epr onlyState:(BOOL)t;
- (id) initWithParameter:(ref<EPParameter>)method state:(ref<EPRemoteState>)rs;

@end
