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
