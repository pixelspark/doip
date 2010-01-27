#import <Cocoa/Cocoa.h>

#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>
#include <TJScout/include/tjservice.h>

#import "TTDiscovery.h"

@class TTPreferencesWindowController;

@interface TJTrayRemoteAppDelegate : NSObject <NSApplicationDelegate, NSMenuDelegate> {
    NSWindow *window;
	NSStatusItem* _item;
	tj::shared::ref<TTDiscovery> _discovery;
	TTPreferencesWindowController* _prefsController;
}

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet TTPreferencesWindowController* prefsController;

@end
