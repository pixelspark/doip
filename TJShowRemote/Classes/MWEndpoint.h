#import <CoreFoundation/CoreFoundation.h>
#import "MWMethod.h"

typedef class UdpSocket;

@interface MWEndpoint : NSObject {
	NSNetService* _service;
	NSData* _definitionFileData;
	NSString* _transportAddress;
	NSString* _transportType;
	NSString* _transportFormat;
	int _transportPort;
	NSMutableSet* _methods;
	UdpSocket* _socket;
}

- (id) initWithService: (NSNetService*)service;
- (NSString*) name;
- (NSNetService*) service;
- (void) executeMethod: (MWMethod*)method;

@property (nonatomic, retain) NSString* transportAddress;
@property (nonatomic, retain) NSString* transportType;
@property (nonatomic, retain) NSString* transportFormat;
@property (nonatomic, readonly) NSMutableSet* methods;

@end
