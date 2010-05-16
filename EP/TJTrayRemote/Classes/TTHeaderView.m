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
