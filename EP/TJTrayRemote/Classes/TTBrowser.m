#import "TTBrowser.h"

@implementation TTBrowser

- (id)initWithCoder:(NSCoder *)decoder {
	self = [super initWithCoder: decoder];
	//[self setMatrixClass: [Matrix class]];
	[self setCellClass: [TTBrowserCell class]];
	return self;
}

@end

@implementation TTBrowserCell

- (id)initTextCell:(NSString *)aString {
	if(self = [super initTextCell:aString]) {
		NSLog(@"initTextCell");
	}
	return self;
}

- (id) init {
	if(self = [super init]) {
		NSLog(@"init");
	}
	return self;
}

- (NSColor *)highlightColorInView:(NSView *)controlView {
    return [NSColor clearColor];
}

- (void)drawInteriorWithFrame:(NSRect)aRect inView:(NSView *)controlView {
    // Draw NSBrowserCell.
    //[super drawInteriorWithFrame:aRect inView:controlView];
}


@end

