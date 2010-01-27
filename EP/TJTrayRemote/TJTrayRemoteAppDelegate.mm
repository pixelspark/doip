#import "TJTrayRemoteAppDelegate.h"
#import "TTMethodMenuItem.h"
#import "TTURLMenuItem.h"

@implementation TJTrayRemoteAppDelegate

@synthesize window;
@synthesize prefsController = _prefsController;

+ (void)initialize {
	// load the default values for the user defaults
	NSString *userDefaultsValuesPath = [[NSBundle mainBundle] pathForResource:@"UserDefaults" ofType:@"plist"];
	NSDictionary *userDefaultsValuesDict = [NSDictionary dictionaryWithContentsOfFile:userDefaultsValuesPath];
	[[NSUserDefaults standardUserDefaults] registerDefaults:userDefaultsValuesDict];
	[[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:userDefaultsValuesDict];
}

- (void) quitApplication:(id)sender {
	[NSApp terminate:nil];
}

- (void) showPreferences:(id)sender {
	[NSApp activateIgnoringOtherApps:YES];
	[_prefsController showWindow:sender];
	[[_prefsController window] makeKeyAndOrderFront:sender];
	[[_prefsController window] makeMainWindow];
}

- (ref<Connection>) connectionForEndpoint: (ref<EPEndpoint>) enp {
	std::multimap< ref<EPEndpoint>, ref<Connection> >& cons = _discovery->_connections;
	if(enp) {
		std::multimap< ref<EPEndpoint>, ref<Connection> >::iterator it = cons.find(enp);
		if(it!=cons.end()) {
			std::pair< ref<EPEndpoint>, ref<Connection> > data = *it;
			ref<Connection> con = it->second;
			if(con) {
				return con;
			}
		}
	}
	return ref<Connection>(0);
}

- (void) execute: (id)sender {
	TTMethodMenuItem* mi = sender;
	if(mi!=0) {
		// Find the first connection for the endpoint
		ref<EPEndpoint> enp = [mi endpoint];
		if(enp) {
			bool noConnection = true;
			ref<Connection> con = [self connectionForEndpoint:enp];
			if(con) {
				ref<EPMethod> method = [mi method];
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
						if(parameterList.size()==0) {
							ref<Message> msg = GC::Hold(new Message(*(paths.begin())));
							con->Send(msg);
						}
					}
				}
			}
			
			if(noConnection) {
				NSAlert* na = [NSAlert alertWithMessageText:@"The selected action cannot be performed" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"The device that provides the action does not supply enough information to be able to connect to it."];
				[na runModal];
			}
		}
	}
}

- (void) menuNeedsUpdate:(NSMenu *)menu {
	[menu removeAllItems];
	
	// Do we have to show items without any methods?
	NSNumber* val = [[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"hideEndpointsWithoutActions"];
	bool hideEndpointsWithoutMethods = [val boolValue];
	bool enableDeveloperExtensions = [(NSNumber*)[[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"enableDeveloperExtensions"] boolValue];
	
	[_discovery->_methodMenuItems release];
	_discovery->_methodMenuItems = [[NSMutableArray alloc] init];
	
	/* Add endpoints as items */
	{
		std::set< ref<EPEndpoint> >::iterator it = _discovery->_endpoints.begin();
		while(it!=_discovery->_endpoints.end()) {
			ref<EPEndpoint> enp = *it;
			if(enp) {
				std::string name = Mbs(enp->GetFriendlyName());
				
				// Enumerate methods and add them
				std::vector< ref<EPMethod> > methods;
				enp->GetMethods(methods);
				
				bool show = false;
				std::set<EPTag> tags;
				enp->GetTags(tags);
				if(tags.size()==0) {
					_discovery->GetTagInPreferences(L"", show);
				}
				else {
					std::set<EPTag>::const_iterator tagIt = tags.begin();
					while(tagIt!=tags.end()) {
						bool enabled = false;
						if(_discovery->GetTagInPreferences(*tagIt, enabled)) {
							show = enabled | show;
							if(show) {
								break;
							}
						}
						
						++tagIt;
					}
				}
				
				if(show) {
					NSMenuItem* endpointItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:name.c_str()] action:nil keyEquivalent:@""];
					if(methods.size()>0) {
						NSMenu* methodsMenu = [[NSMenu alloc] initWithTitle:@""];
						std::vector< ref<EPMethod> >::iterator mit = methods.begin();
						while(mit!=methods.end()) {
							ref<EPMethod> method = *mit;
							if(method) {
								ref<Connection> connection = [self connectionForEndpoint:enp];
								ref<EPRemoteState> rs;
								std::map< ref<EPEndpoint>, ref<EPRemoteState> >::iterator sit = _discovery->_remoteStates.find(enp);
								if(sit!=_discovery->_remoteStates.end()) {
									rs = sit->second;
								}
								
								TTMethodMenuItem* methodItem = [[TTMethodMenuItem alloc] initWithMethod:method endpoint:enp connection:connection state:rs];
								[methodItem setTarget:self];
								[methodItem setAction:@selector(execute:)];
								[methodsMenu addItem:methodItem];
								[_discovery->_methodMenuItems addObject:methodItem];
								[methodItem release];
							}
							++mit;
						}
						
						if(enableDeveloperExtensions) {
							[methodsMenu addItem:[NSMenuItem separatorItem]];
							ref<Service> svc = _discovery->GetServiceForEndpoint(enp);
							std::wstring defPath;
							
							if(svc && svc->GetAttribute(L"EPDefinitionPath", defPath)) {
								NSString* url = [NSString stringWithFormat:@"http://%s:%d%s",Mbs(svc->GetHostName()).c_str(), svc->GetPort(), Mbs(defPath).c_str(), nil];
								TTURLMenuItem* definitionItem = [[TTURLMenuItem alloc] initWithTitle:NSLocalizedString(@"Inspect definition...",@"") url:url];
								[methodsMenu addItem:definitionItem];
							}
							
							std::wstring statePath;
							if(svc && svc->GetAttribute(L"EPStatePath", statePath)) {
								NSString* url = [NSString stringWithFormat:@"http://%s:%d%s",Mbs(svc->GetHostName()).c_str(), svc->GetPort(), Mbs(statePath).c_str(), nil];
								TTURLMenuItem* definitionItem = [[TTURLMenuItem alloc] initWithTitle:NSLocalizedString(@"Inspect state...",@"") url:url];
								[methodsMenu addItem:definitionItem];
							}
						}
						
						[endpointItem setSubmenu:methodsMenu];
						[methodsMenu release];
					}
					
					if(!hideEndpointsWithoutMethods || methods.size()>0) {
						[menu addItem:endpointItem];
					}
					[endpointItem release];
				}
			}	
			++it;
		}
	}
	
	/* Quit/prefs item */
	[menu addItem:[NSMenuItem separatorItem]];
	
	NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Preferences...", @"Show preferences window") action:@selector(showPreferences:) keyEquivalent:@""];
	[prefsItem setTarget:self];
	[menu addItem:prefsItem];
	[prefsItem release];
	
	NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quit", @"Quit application") action:@selector(quitApplication:) keyEquivalent:@""];
	[quitItem setTarget:self];
	[menu addItem:quitItem];
	[quitItem release];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Add the status item to the system status bar
	
	NSStatusBar* bar = [NSStatusBar systemStatusBar];
	_item = [[bar statusItemWithLength:NSSquareStatusItemLength] retain];
	
	NSBundle *bundle = [NSBundle mainBundle];
	NSImage* statusImage = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"icon" ofType:@"png"]];
	NSImage* statusImageHigh = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"icon-high" ofType:@"png"]];
	
	[_item setImage:statusImage];
	[_item setAlternateImage:statusImageHigh];
	[_item setToolTip:@"HomeWeave Remote"];
	[_item setHighlightMode:TRUE];
	[statusImage release];
	
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
	[super dealloc];
}

@end
