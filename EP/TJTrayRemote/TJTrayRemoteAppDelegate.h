#import <Cocoa/Cocoa.h>

#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>
#include <TJScout/include/tjservice.h>

#import "TTDiscovery.h"
#import "MAAttachedWindow.h"

@class TTStatusItemView;
@class TTPreferencesWindowController;
@class TTFlipViewController;

@interface TJTrayRemoteAppDelegate : NSObject <NSApplicationDelegate, NSMenuDelegate> {
    NSWindow *window;
	NSStatusItem* _item;
	tj::shared::ref<TTDiscovery> _discovery;
	TTPreferencesWindowController* _prefsController;
	TTFlipViewController* _flipper;
	NSView* _browserView;
	TTStatusItemView* _statusView;
	MAAttachedWindow* _attachedWindow;
}

- (ref<TTDiscovery>)discovery;
- (IBAction) showPreferences:(id)sender;
- (void) executeMethod: (ref<EPMethod>)method onEndpoint:(ref<EPEndpoint>)enp;
- (void)toggleAttachedWindowAtPoint:(NSPoint)pt;
- (IBAction) quitApplication:(id)sender;

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet TTPreferencesWindowController* prefsController;
@property (nonatomic, retain) IBOutlet NSView* browserView;
@property (nonatomic, retain) IBOutlet TTFlipViewController* flipper;

@end
