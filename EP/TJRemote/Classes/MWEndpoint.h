#ifndef MW_ENDPOINT_H
#define MW_ENDPOINT_H

#import <CoreFoundation/CoreFoundation.h>
#import "MWMethod.h"
#include "MWDownload.h"

typedef class UdpSocket;

@interface MWEndpoint : NSObject <MWDownloadDelegate> {
	NSNetService* _service;
	MWDownload* _download;
	NSData* _definitionFileData;
	NSString* _id;
	NSString* _transportAddress;
	NSString* _transportType;
	NSString* _transportFormat;
	int _transportPort;
	NSMutableArray* _methods;
	UdpSocket* _socket;
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
@property (nonatomic, retain) MWDownload* download;
@property (nonatomic, readonly) NSString* endpointIdentifier;

@end

#endif