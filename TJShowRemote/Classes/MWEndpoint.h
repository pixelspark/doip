#import <CoreFoundation/CoreFoundation.h>

@interface MWEndpoint : NSObject {
	NSNetService* _service;
	NSData* _definitionFileData;
	NSString* _transportAddress;
	int _transportPort;
	NSMutableSet* _methods;
}

- (id)initWithService:(NSNetService*)service;
- (NSString*)name;
- (NSNetService*)service;

@property (nonatomic, retain) NSString* transportAddress;

@end
