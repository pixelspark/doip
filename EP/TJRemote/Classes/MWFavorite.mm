#import "MWFavorite.h"

@implementation MWFavorite
@synthesize messagePath = _messagePath;
@synthesize messageArguments = _messageArguments;
@synthesize friendlyName = _friendlyName;
@synthesize specificDevice = _specific;

- (id) initWithPath:(NSString *)path andArguments:(NSMutableArray *)args {
	if(self = [super init]) {
		self.messagePath = path;
		self.messageArguments = args;
	}
	return self;
}

- (void) dealloc {
	[_messagePath release];
	[_messageArguments release];
	[_friendlyName release];
	[super dealloc];
}

- (id) initWithCoder:(NSCoder *)aDecoder {
	if(self = [super init]) {
		self.messagePath = [aDecoder decodeObjectForKey:@"Path"];
		self.messageArguments = [aDecoder decodeObjectForKey:@"Arguments"];
		self.friendlyName = [aDecoder decodeObjectForKey:@"FriendlyName"];
		self.specificDevice = [aDecoder decodeObjectForKey:@"SpecificDevice"];
	}
	return self;
}

- (void) encodeWithCoder:(NSCoder *)aCoder {
	[aCoder encodeObject:self.messagePath forKey:@"Path"];
	[aCoder encodeObject:self.messageArguments forKey:@"Arguments"];
	[aCoder encodeObject:self.friendlyName forKey:@"FriendlyName"];
	[aCoder encodeObject:self.specificDevice forKey:@"SpecificDevice"];
}

@end
