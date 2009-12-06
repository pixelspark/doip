#import "MWDownload.h"

@implementation MWDownload
@synthesize data = _data;
@synthesize connection = _connection;
@synthesize delegate = _delegate;

- (id)initWithURL:(NSURL *)url delegate:(id<MWDownloadDelegate>)d {
	if(self = [super init]) {
		NSLog(@"MWDownload %@", [url absoluteURL]);
		NSURLRequest* request = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:5.0];
		_connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
		self.delegate = d;
	}
	return self;
}

- (void)connection:(NSURLConnection *)theConnection didReceiveData:(NSData *)incrementalData {
    if (_data==nil) {
		_data = [[NSMutableData alloc] initWithCapacity:2048];
    }
    [_data appendData:incrementalData];
}

- (void)connectionDidFinishLoading:(NSURLConnection*)theConnection {
    self.connection = nil;
	[_delegate download:self completed:self.data];
	self.data = nil;
}

- (void) dealloc {
	self.connection = nil;
	self.data = nil;
	self.delegate = nil;
	[super dealloc];
}

@end
