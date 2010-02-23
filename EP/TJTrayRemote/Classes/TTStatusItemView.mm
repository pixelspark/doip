#import "TTStatusItemView.h"

@implementation TTStatusItemView
@synthesize app = _app;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        NSBundle *bundle = [NSBundle mainBundle];
		_statusImage = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"icon" ofType:@"png"]];
		_statusImageHigh = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"icon-high" ofType:@"png"]];
		_clicked = NO;
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
	NSRect iconRect = dirtyRect;
	iconRect.origin.x += 3;
	iconRect.origin.y += 1;
	
	if (_clicked) {
        [[NSColor selectedMenuItemColor] set];
        NSRectFill(dirtyRect);
		[_statusImageHigh drawInRect:iconRect fromRect:dirtyRect operation:NSCompositeSourceOver fraction:1.0];
    }
	else {
		[_statusImage drawInRect:iconRect fromRect:dirtyRect operation:NSCompositeCopy fraction:1.0];
	}
}

- (void)dealloc {
	[_statusImage release];
	[_statusImageHigh release];
	[super dealloc];
}

- (void)mouseDown:(NSEvent *)event {
    NSRect frame = [[self window] frame];
    NSPoint pt = NSMakePoint(NSMidX(frame)-5, NSMinY(frame));
    [_app toggleAttachedWindowAtPoint:pt];
    _clicked = !_clicked;
    [self setNeedsDisplay:YES];
}

@end
