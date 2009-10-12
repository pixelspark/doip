#import "MWMethod.h"

@implementation MWMethod

@synthesize pattern = _pattern;
@synthesize parameters = _parameters;
@synthesize friendlyName = _friendly;

- (id) initWithPattern:(NSString*)pattern parameters:(NSString*)ps friendlyName:(NSString*)fn {
	if(self = [super init]) {
		self.pattern = pattern;
		self.parameters = ps;
		self.friendlyName = fn;
	}
	return self;
}

- (void) dealloc {
	[_pattern release];
	[_parameters release];
	[_friendly release];
	[super dealloc];
}

@end
