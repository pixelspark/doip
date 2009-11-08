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
