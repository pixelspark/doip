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
 
 #ifndef MW_ENDPOINT_H
#define MW_ENDPOINT_H

#import <CoreFoundation/CoreFoundation.h>
#import "MWMethod.h"
#include "MWDownload.h"

typedef class UdpSocket;

@interface MWEndpoint : NSObject <MWDownloadDelegate> {
	NSNetService* _service;
	MWDownload* _definitionDownload;
	NSData* _definitionFileData;
	NSString* _id;
	NSString* _transportAddress;
	NSString* _transportType;
	NSString* _transportFormat;
	int _transportPort;
	NSMutableArray* _methods;
	UdpSocket* _socket;
	NSString* _stateID;
	MWDownload* _stateDownload;
	bool _needStateUpdate;
	NSURL* _currentStateURL;
	NSMutableDictionary* _currentState;
}

- (id) initWithService: (NSNetService*)service;
- (NSString*) name;
- (NSNetService*) service;
- (void) executeMethod: (MWMethod*)method;
- (bool) executeFavorite:(MWFavorite*)favorite;

@property (nonatomic, retain) NSString* transportAddress;
@property (nonatomic, retain) NSString* transportType;
@property (nonatomic, retain) NSString* transportFormat;
@property (nonatomic, readonly) NSMutableArray* methods;
@property (nonatomic, retain) MWDownload* definitionDownload;
@property (nonatomic, retain) NSString* stateID;
@property (nonatomic, readonly) NSString* endpointIdentifier;
@property (nonatomic, assign) bool needStateUpdate;
@property (nonatomic, retain) NSURL* currentStateURL;
@property (nonatomic, retain) MWDownload* stateDownload;
@property (nonatomic, retain) NSMutableDictionary* currentState;

@end

@interface MWStateChange: NSObject {
	MWEndpoint* _endpoint;
	NSString* _key;
	NSString* _value;
}

- (id) initWithEndpoint:(MWEndpoint*)ep key:(NSString*)k value:(NSString*)v;

@property (nonatomic, retain) MWEndpoint* endpoint;
@property (nonatomic, retain) NSString* key;
@property (nonatomic, retain) NSString* value;

@end

#endif