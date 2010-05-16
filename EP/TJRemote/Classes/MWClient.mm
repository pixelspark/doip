/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
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
	//NSLog(@"Service resolved name=%@ host=%@ port=%d type=%@", [netService name], [netService hostName], [netService port], [netService type]);
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
	//NSLog(@"Remove service: %@", [aNetService name]);
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
