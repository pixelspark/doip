#import "MWHeaderView.h"

@implementation MWHeaderView


- (id)initWithFrame:(CGRect)frame andTitle:(NSString*)title {
    if (self = [super initWithFrame:frame]) {
		// create the button object
		self.opaque = NO;
		UILabel * headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
		headerLabel.backgroundColor = [UIColor clearColor];
		headerLabel.opaque = NO;
		headerLabel.textColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.5];
		headerLabel.shadowColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.8];
		headerLabel.shadowOffset = CGSizeMake(-0.5, 0.5);
		headerLabel.highlightedTextColor = [UIColor whiteColor];
		headerLabel.font = [UIFont boldSystemFontOfSize:15];
		headerLabel.frame = CGRectMake(10.0, 0.0, 300.0, 27.0);
		headerLabel.text = title;
		[self addSubview:headerLabel];
    }
    return self;
}

- (void)drawRect:(CGRect)rect {
    CGContextRef currentContext = UIGraphicsGetCurrentContext();
	
    CGGradientRef glossGradient;
    CGColorSpaceRef rgbColorspace;
    size_t num_locations = 2;
    CGFloat locations[2] = { 0.0, 1.0 };
    CGFloat components[8] = { 1.0, 1.0, 1.0, 0.35,  // Start color
		1.0, 1.0, 1.0, 0.06 }; // End color
	
    rgbColorspace = CGColorSpaceCreateDeviceRGB();
    glossGradient = CGGradientCreateWithColorComponents(rgbColorspace, components, locations, num_locations);
	
    CGRect currentBounds = rect;
    CGPoint topCenter = CGPointMake(CGRectGetMidX(currentBounds), 0.0f);
    CGPoint midCenter = CGPointMake(CGRectGetMidX(currentBounds), CGRectGetMidY(currentBounds));
    CGContextDrawLinearGradient(currentContext, glossGradient, topCenter, midCenter, 0);
	
    CGGradientRelease(glossGradient);
    CGColorSpaceRelease(rgbColorspace); 
}


- (void)dealloc {
    [super dealloc];
}


@end
