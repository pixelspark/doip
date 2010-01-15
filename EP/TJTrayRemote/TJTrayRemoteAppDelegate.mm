#import "TJTrayRemoteAppDelegate.h"
#include <EP/include/epdnssddiscovery.h>

@interface MethodMenuItem: NSMenuItem {
	ref<EPMethod> _method;
	ref<EPEndpoint> _endpoint;
}


- (id) initWithMethod:(ref<EPMethod>)method endpoint:(ref<EPEndpoint>)ep;
- (ref<EPEndpoint>) endpoint;
- (ref<EPMethod>) method;

@end

@implementation MethodMenuItem

- (id) initWithMethod:(ref<EPMethod>)m endpoint:(ref<EPEndpoint>)ep {
	std::string methodName = Mbs(m->GetFriendlyName());
	if(self = [super initWithTitle:[NSString stringWithUTF8String:methodName.c_str()] action:nil keyEquivalent:@""]) {
		_method = m;
		_endpoint = ep;
	}
	
	return self;
}

- (ref<EPEndpoint>) endpoint {
	return _endpoint;
}

- (ref<EPMethod>) method {
	return _method;
}

@end


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

- (void) execute: (id)sender {
	MethodMenuItem* mi = sender;
	if(mi!=0) {
		// Find the first connection for the endpoint
		std::multimap< ref<EPEndpoint>, ref<Connection> >& cons = _discovery->_connections;
		ref<EPEndpoint> enp = [mi endpoint];
		if(enp) {
			bool noConnection = true;
			std::multimap< ref<EPEndpoint>, ref<Connection> >::iterator it = cons.find(enp);
			if(it!=cons.end()) {
				std::pair< ref<EPEndpoint>, ref<Connection> > data = *it;
				ref<Connection> con = it->second;
				if(con) {
					ref<EPMethod> method = [mi method];
					if(method) {
						std::set<EPPath> paths;
						method->GetPaths(paths);
						if(paths.size()==0) {
							noConnection = true;
						}
						else {
							std::vector< tj::shared::ref<EPParameter> > parameterList;
							method->GetParameters(parameterList);
							if(parameterList.size()==0) {
								ref<Message> msg = GC::Hold(new Message(*(paths.begin())));
								con->Send(msg);
								noConnection = false;
							}
							else {
								// Some form or a warning
								NSAlert* na = [NSAlert alertWithMessageText:@"In the current version, it is not yet possible to execute methods that require parameters." defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"The method will not be executed right now."];
								[na runModal];
								noConnection = false;
							}
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
							MethodMenuItem* methodItem = [[MethodMenuItem alloc] initWithMethod:method endpoint:enp];
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
