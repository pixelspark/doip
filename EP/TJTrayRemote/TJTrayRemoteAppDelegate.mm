#import "TJTrayRemoteAppDelegate.h"
#import "TTMethodMenuItem.h"
#include <EP/include/epdnssddiscovery.h>

TTDiscovery::TTDiscovery() {
	_slt = SocketListenerThread::DefaultInstance();
}

void TTDiscovery::Notify(ref<Object> source, const DiscoveryNotification& dn) {
	ThreadLock lock(&_lock);
	
	ref<EPEndpoint> enp = dn.endpoint;
	if(enp) {
		_endpoints.insert(enp);
		_connections.insert(std::pair<ref<EPEndpoint>, ref<Connection> >(enp, dn.connection));
	}
}

void TTDiscovery::OnCreated() {
	ref<EPDiscoveryDefinition> dd = GC::Hold(new EPDiscoveryDefinition());
	_discovery = GC::Hold(new EPDiscovery());
	_discovery->Create(ref<DiscoveryDefinition>(dd), L"");
	_discovery->EventDiscovered.AddListener(this);
}

TTDiscovery::~TTDiscovery() {
}

@implementation TJTrayRemoteAppDelegate

@synthesize window;

- (void) quitApplication:(id)sender {
	[NSApp terminate:nil];
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
	
	/* Add endpoints as items */
	{
		std::set< ref<EPEndpoint> >::iterator it = _discovery->_endpoints.begin();
		while(it!=_discovery->_endpoints.end()) {
			ref<EPEndpoint> enp = *it;
			if(enp) {
				std::string name = Mbs(enp->GetFriendlyName());
				NSMenuItem* endpointItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:name.c_str()] action:nil keyEquivalent:@""];
				
				// Enumerate methods and add them
				std::vector< ref<EPMethod> > methods;
				enp->GetMethods(methods);
				
				if(methods.size()>0) {
					NSMenu* methodsMenu = [[NSMenu alloc] initWithTitle:@""];
					std::vector< ref<EPMethod> >::iterator mit = methods.begin();
					while(mit!=methods.end()) {
						ref<EPMethod> method = *mit;
						if(method) {
							ref<Connection> connection = [self connectionForEndpoint:enp];
							TTMethodMenuItem* methodItem = [[TTMethodMenuItem alloc] initWithMethod:method endpoint:enp connection:connection];
							[methodItem setTarget:self];
							[methodItem setAction:@selector(execute:)];
							[methodsMenu addItem:methodItem];
							[methodItem release];
						}
						++mit;
					}
					[endpointItem setSubmenu:methodsMenu];
					[methodsMenu release];
				}
					
				[menu addItem:endpointItem];
				[endpointItem release];
				
			}	
			++it;
		}
	}
	
	/* Quit item*/
	[menu addItem:[NSMenuItem separatorItem]];
	NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(quitApplication:) keyEquivalent:@""];
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
