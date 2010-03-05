#import "MWClient.h"
#include "MWEndpoint.h"

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
		resolvedEndpoints = [[NSMutableArray alloc] init];
		
		// Start searching for endpoints
		endpointsBrowser = [[NSNetServiceBrowser alloc] init];
		endpointsBrowser.delegate = self;
		[endpointsBrowser searchForServicesOfType:@"_ep._tcp" inDomain:@""];

	}
	return self;
}

- (NSMutableArray*)resolvedEndpoints {
	return resolvedEndpoints;
}

- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)browser {
}

- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)browser {
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didNotSearch:(NSDictionary *)errorDict {
}

- (void)netServiceDidResolveAddress:(NSNetService *)netService {
	NSLog(@"Service resolved name=%@ host=%@ port=%d type=%@", [netService name], [netService hostName], [netService port], [netService type]);
	if([[netService type] isEqualToString:@"_ep._tcp."] && [[netService name] length]>0) {
		MWEndpoint* ep = [[MWEndpoint alloc] initWithService:netService];
		[resolvedEndpoints addObject:ep];
		[resolvedEndpoints sortUsingSelector:@selector(compareToEndpoint:)];
		[ep release];
	}
	[services removeObject:netService];
	
	// Let the appdelegate know that we've resolved a service
	if(delegate) {
		[delegate client:self foundService:netService];
	}
}

- (void)netService:(NSNetService *)netService didNotResolve:(NSDictionary *)errorDict {
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	[aNetService setDelegate:self];
	[aNetService resolveWithTimeout:10];
	[services addObject:aNetService];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	NSLog(@"Remove service: %@", [aNetService name]);
	[services removeObject:aNetService];
	
	for (MWEndpoint* endpoint in resolvedEndpoints) {
		if([[endpoint service] isEqual:aNetService]) {
			[resolvedEndpoints removeObject:endpoint];
			break;
		}
	}
	
	if(delegate!=nil) {
		[delegate client:self foundServiceRemoved:aNetService];
	}
}

- (void)dealloc {
	[services release];
	[endpointsBrowser stop];
	[endpointsBrowser release];
	[resolvedEndpoints release];
	[super dealloc];
}

@end
