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