#import "TTURLMenuItem.h"


@implementation TTURLMenuItem

- (void) itemClicked:(id)sender {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:_url]];
}

- (id) initWithTitle:(NSString*)title url:(NSString*)url {
	if(self = [super initWithTitle:title action:@selector(itemClicked:) keyEquivalent:@""]) {
		[self setTarget:self];
		_url = url;
		[_url retain];
	}
	return self;
}

- (void) dealloc {
	[_url release];
	[super dealloc];
}

@end
