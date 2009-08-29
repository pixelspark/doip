#import "MWClient.h"

@implementation MWClient
@synthesize delegate;

+ (MWClient*)sharedInstance {
	static MWClient* instance = nil;
	
	@synchronized(self) {
		if(instance==nil) {
			instance = [[MWClient alloc] init];
		}
	}
	return instance;
}

- (id)init {
	if(self = [super init]) {
		// Initialize services array
		services = [[NSMutableSet alloc] init];
		resolvedServices = [[NSMutableArray alloc] init];
		
		// Start searching for services
		browser = [[NSNetServiceBrowser alloc] init];  //[[NSNetService alloc] initWithDomain:@"" type:KServiceType name:@"" port:7965];
		browser.delegate = self;
		[browser searchForServicesOfType:@"_osc._udp" inDomain:@""];
	}
	return self;
}

- (NSMutableArray*)resolvedServices {
	return resolvedServices;
}

- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)browser {
}

- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)browser {
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didNotSearch:(NSDictionary *)errorDict {
}

- (void)netServiceDidResolveAddress:(NSNetService *)netService {
	//NSLog(@"Service resolved name=%@ host=%@ port=%d", [netService name], [netService hostName], [netService port]);
	[resolvedServices addObject:netService];
	[services removeObject:netService];
	
	// Let the appdelegate know that we've resolved a service
	if(delegate) {
		[delegate client:self foundService:netService];
	}
}

- (void)netService:(NSNetService *)netService didNotResolve:(NSDictionary *)errorDict {
	//NSLog(@"Did not resolve");
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	//NSLog(@"Service found name=%@", [aNetService name]);
	[aNetService setDelegate:self];
	[aNetService resolveWithTimeout:10];
	[services addObject:aNetService];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	//NSLog(@"Service removed name=%@", [aNetService name]);
	[services removeObject:aNetService];
	[resolvedServices removeObject:aNetService];
	if(delegate!=nil) {
		[delegate client:self foundServiceRemoved:aNetService];
	}
}

- (void)dealloc {
	[services release];
	[resolvedServices release];
	[browser stop];
	[browser release];
	[super dealloc];
}

@end
