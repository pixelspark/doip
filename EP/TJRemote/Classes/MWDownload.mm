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
 
 #import "MWDownload.h"

@implementation MWDownload
@synthesize data = _data;
@synthesize connection = _connection;
@synthesize delegate = _delegate;

- (id)initWithURL:(NSURL *)url delegate:(id<MWDownloadDelegate>)d {
	if(self = [super init]) {
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
