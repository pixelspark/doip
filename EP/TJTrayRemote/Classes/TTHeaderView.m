#import "TTHeaderView.h"

@implementation TTHeaderView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
    CGColorSpaceRef myColorspace=CGColorSpaceCreateDeviceRGB();
	size_t num_locations = 2;
	CGFloat locations[2] = { 1.0, 0.0 };
	CGFloat components[8] = { 1.0, 1.0, 1.0, 0.0,    0.7, 0.7, 0.7, 1.0 };
	
	CGGradientRef myGradient = CGGradientCreateWithColorComponents(myColorspace, components, locations, num_locations);
	
	CGPoint myStartPoint, myEndPoint;
	myStartPoint.x = 0;
	myStartPoint.y = 0;
	myEndPoint.x = 0;
	myEndPoint.y = dirtyRect.size.height;
	CGContextAddRect(context, NSRectToCGRect(dirtyRect));
	CGContextDrawLinearGradient(context, myGradient, myStartPoint, myEndPoint, 0);
	
	CGGradientRelease(myGradient);
	
}

@end
