#import "TJTrayRemoteAppDelegate.h"
#import "TTMethodMenuItem.h"
#include <EP/include/epdnssddiscovery.h>

TTDiscovery::TTDiscovery() {
	_slt = SocketListenerThread::DefaultInstance();
	_methodMenuItems = nil;
}

void TTDiscovery::Notify(ref<Object> source, const EPStateChangeNotification& cn) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	ref<EPRemoteState> epr = cn.remoteState;
	
	if(epr && _methodMenuItems!=nil) {
		// Which endpoint was this for, again?
		ref<EPEndpoint> forEndpoint;
		std::map< ref<EPEndpoint>, ref<EPRemoteState> >::iterator it = _remoteStates.begin();
		while(it!=_remoteStates.end()) {
			if(it->second==epr) {
				forEndpoint = it->first;
				break;
			}
			++it;
		}
		
		if(forEndpoint) {
			for(TTMethodMenuItem* mi in _methodMenuItems) {
				if([mi endpoint]==forEndpoint) {
					[mi update:epr];
				}
			}
		}
	}
	
	[pool release];
}

bool TTDiscovery::GetTagInPreferences(const EPTag& tag, bool& enabled) {
	NSMutableArray* dict = [[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"tags"];
	NSString* tagName = [NSString stringWithUTF8String:Mbs(tag).c_str()];
	for (NSMutableDictionary* row in dict) {
		NSString* name = [row valueForKey:@"name"];
		if(name!=nil && [name isEqualToString:tagName]) {
			NSNumber* enabledNr = [row valueForKey:@"enabled"];
			enabled = [enabledNr boolValue];
			return true;
		}
	}
	return false;
}

void TTDiscovery::AddTagToPreferences(const EPTag& tag) {
	NSString* tagName = [NSString stringWithUTF8String:Mbs(tag).c_str()];
	NSMutableArray* dict = [[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"tags"];
	bool enabled = false;
	if(!GetTagInPreferences(tag,enabled)) {
		NSMutableDictionary* row = [[NSMutableDictionary alloc] init];
		[row setValue:tagName forKey:@"name"];
		[row setValue:[NSNumber numberWithBool:YES] forKey:@"enabled"];
		[dict addObject:row];
		[row release];
	}
	
	[[[NSUserDefaultsController sharedUserDefaultsController] values] setValue:dict forKey:@"tags"];
}

void TTDiscovery::Notify(ref<Object> source, const DiscoveryNotification& dn) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	ThreadLock lock(&_lock);
	
	ref<EPEndpoint> enp = dn.endpoint;
	if(dn.added) {
		if(enp && dn.connection) {
			_endpoints.insert(enp);
			_connections.insert(std::pair<ref<EPEndpoint>, ref<Connection> >(enp, dn.connection));
			
			std::set<EPTag> tags;
			enp->GetTags(tags);
			
			if(tags.size()==0) {
				AddTagToPreferences(L"");
			}
			else {
				// Add each tag to the set of tags for user defaults
				std::set<EPTag>::const_iterator tit = tags.begin();
				while(tit!=tags.end()) {
					AddTagToPreferences(*tit);
					++tit;
				}
			}
		}
		
		if(enp && dn.remoteState) {
			_remoteStates[enp] = dn.remoteState;
			ref<EPRemoteState>(dn.remoteState)->EventStateChanged.AddListener(this);
		}
	}
	else {
		if(enp) {
			std::set<ref<EPEndpoint> >::iterator eit = _endpoints.find(enp);
			if(eit!=_endpoints.end()) {
				_endpoints.erase(eit);
			}
			
			std::map< ref<EPEndpoint>, ref<EPRemoteState> >::iterator sit = _remoteStates.find(enp);
			if(sit!=_remoteStates.end()) {
				_remoteStates.erase(sit);
			}
		}
		else {
			Log::Write(L"TJTrayRemote/TTDiscovery", L"Endpoint leaving, but no endpoint referenced");
		}
		
		std::multimap< ref<EPEndpoint>, ref<Connection> >::iterator it = _connections.begin();
		while(it!=_connections.end()) {
			ref<EPEndpoint> itep = it->first;
			ref<Connection> itcon = it->second;
			if(itep==enp || itcon==dn.connection) {
				_connections.erase(it);
				it = _connections.begin();
			}
			else {
				++it;
			}
		}
	}
	[pool release];
}

void TTDiscovery::OnCreated() {
	ref<EPDiscoveryDefinition> dd = GC::Hold(new EPDiscoveryDefinition());
	_discovery = GC::Hold(new EPDiscovery());
	_discovery->Create(ref<DiscoveryDefinition>(dd), L"");
	_discovery->EventDiscovered.AddListener(this);
}

TTDiscovery::~TTDiscovery() {
	[_methodMenuItems release];
}

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
