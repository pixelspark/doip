#import "TJTrayRemoteAppDelegate.h"
#import "TTFlipViewController.h"
#import "TTStatusItemView.h"

@implementation TJTrayRemoteAppDelegate

@synthesize window;
@synthesize prefsController = _prefsController;
@synthesize browserView = _browserView;
@synthesize flipper = _flipper;

+ (void)initialize {
	// load the default values for the user defaults
	NSString *userDefaultsValuesPath = [[NSBundle mainBundle] pathForResource:@"UserDefaults" ofType:@"plist"];
	NSDictionary *userDefaultsValuesDict = [NSDictionary dictionaryWithContentsOfFile:userDefaultsValuesPath];
	[[NSUserDefaults standardUserDefaults] registerDefaults:userDefaultsValuesDict];
	[[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:userDefaultsValuesDict];
}

- (IBAction) quitApplication:(id)sender {
	[NSApp terminate:nil];
}

- (ref<TTDiscovery>) discovery {
	return _discovery;
}

- (IBAction) showPreferences:(id)sender {
	[NSApp activateIgnoringOtherApps:YES];
	[_prefsController showWindow:sender];
	[[_prefsController window] makeKeyAndOrderFront:sender];
	[[_prefsController window] makeMainWindow];
}

- (void) executeMethod: (ref<EPMethod>)method onEndpoint:(ref<EPEndpoint>)enp {
	if(enp) {
		bool noConnection = true;
		ref<Connection> con = _discovery->GetConnectionForEndpoint(enp);
		if(con) {
			if(method) {
				std::set<EPPath> paths;
				method->GetPaths(paths);
				if(paths.size()==0) {
					noConnection = true;
				}
				else {
					noConnection = false;
					std::vector< tj::shared::ref<EPParameter> > parameterList;
					method->GetParameters(parameterList);
					ref<Message> message = GC::Hold(new Message(*(paths.begin())));
					
					// Put all the run-time changed default values in the message
					unsigned int i = 0;
					std::vector< ref<EPParameter> >::iterator it = parameterList.begin();
					while(it!=parameterList.end()) {
						ref<EPParameter> epp = *it;
						if(epp) {
							message->SetParameter(i, epp->GetDefaultValue());
						}
						++i;
						++it;
					}
					
					con->Send(message);
				}
			}
		}
		
		if(noConnection) {
			NSAlert* na = [NSAlert alertWithMessageText:@"The selected action cannot be performed" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"The device that provides the action does not supply enough information to be able to connect to it."];
			[na runModal];
		}
	}
}

- (void)toggleAttachedWindowAtPoint:(NSPoint)pt {
    // Attach/detach window.
    if (!_attachedWindow) {
		_discovery->UpdateShownEndpoints();
		[_flipper flipToFront];
		
        _attachedWindow = [[MAAttachedWindow alloc] initWithView:_browserView attachedToPoint:pt inWindow:nil onSide:MAPositionBottomRight atDistance:-5.0];
		[NSApp activateIgnoringOtherApps:YES];
		[_attachedWindow makeKeyAndOrderFront:self];
		
    } 
	else {
        [_attachedWindow orderOut:self];
        [_attachedWindow release];
        _attachedWindow = nil;
    }    
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Add the status item to the system status bar
	
	NSStatusBar* bar = [NSStatusBar systemStatusBar];
	_item = [[bar statusItemWithLength:NSSquareStatusItemLength] retain];
	_statusView = [[TTStatusItemView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0,0,16,16))];
	_statusView.app = self;
	
	[_item setView:_statusView];
	[_item setToolTip:@"HomeWeave Remote"];
	
	NSMenu* menu = [[NSMenu alloc] init];
	[menu setDelegate:self];
	[_item setMenu:menu];
	[menu release];
	
	// Set up device discovery
	_discovery = GC::Hold(new TTDiscovery());
}

- (void) dealloc {
	[_item release];
	_discovery->~TTDiscovery();
	[_statusView release];
	[super dealloc];
}

@end
