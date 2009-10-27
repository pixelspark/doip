#import <Foundation/Foundation.h>

@protocol MWDownloadDelegate;

@interface MWDownload : NSObject {
	NSMutableData* _data;
	NSURLConnection* _connection;
	id<MWDownloadDelegate> _delegate;
}

- (id) initWithURL: (NSURL*)url delegate:(id<MWDownloadDelegate>)d;

@property (nonatomic, retain) NSMutableData* data;
@property (nonatomic, retain) NSURLConnection* connection;
@property (nonatomic, assign) id<MWDownloadDelegate> delegate;

@end

@protocol MWDownloadDelegate
- (void) download: (MWDownload*)d completed:(NSData*)data;
@end

