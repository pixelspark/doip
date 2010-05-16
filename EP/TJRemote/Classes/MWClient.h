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
 
 #import <Foundation/Foundation.h>

@protocol MWClientDelegate;

@interface MWClient : NSObject {
	NSNetServiceBrowser* endpointsBrowser;
	NSMutableSet* services;
	NSMutableArray* resolvedEndpoints;
	id<MWClientDelegate> delegate;
}

+ (MWClient*)sharedInstance;
- (NSMutableArray*)resolvedEndpoints;

@property (nonatomic, retain) id<MWClientDelegate> delegate;

@end

@protocol MWClientDelegate
- (void)client:(MWClient*)c foundService:(NSNetService*)service;
- (void)client:(MWClient*)c foundServiceRemoved:(NSNetService*)service;
@end
