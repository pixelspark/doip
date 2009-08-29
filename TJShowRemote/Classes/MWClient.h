#import <Foundation/Foundation.h>

@protocol MWClientDelegate;

@interface MWClient : NSObject {
	NSNetServiceBrowser* browser;
	NSMutableSet* services;
	NSMutableArray* resolvedServices;
	id<MWClientDelegate> delegate;
}

+ (MWClient*)sharedInstance;
- (NSMutableArray*) resolvedServices;
@property (nonatomic, retain) id<MWClientDelegate> delegate;

@end

@protocol MWClientDelegate
- (void)client:(MWClient*)c foundService:(NSNetService*)service;
- (void)client:(MWClient*)c foundServiceRemoved:(NSNetService*)service;
@end
