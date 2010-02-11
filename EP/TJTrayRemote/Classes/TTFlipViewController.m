#import "TTFlipViewController.h"
#import <QuartzCore/CoreAnimation.h>

@implementation TTFlipViewController
@synthesize backView = _backView;
@synthesize frontView = _frontView;

- (void) setAnimations:(BOOL)forward {
	NSView *contentView = self.view;
    [contentView setWantsLayer:YES];
	
    CATransition* transition = [CATransition animation];
    [transition setType:@"cube"];
    [transition setSubtype:forward ? kCATransitionFromRight: kCATransitionFromLeft];
	
    NSDictionary *ani = [NSDictionary dictionaryWithObject:transition forKey:@"subviews"];
    [contentView setAnimations:ani];
}

- (void)awakeFromNib {
}

- (void) flipToFront {
	[self.view replaceSubview:_backView with:_frontView];
}

- (IBAction)flip:(id)sender {
	if([_frontView superview]!=nil) {
		[self setAnimations:YES];
		[[self.view animator] replaceSubview:_frontView with:_backView];
	}
	else {
		[self setAnimations:NO];
		[[self.view animator] replaceSubview:_backView with:_frontView];
	}
}

- (void)setView:(NSView *)view {
	[super setView:view];
	NSLog(@"f=%@ %@ b=%@ %@", _frontView, _frontView.layer, _backView, _backView.layer);
	[view addSubview:_frontView];
	[_frontView setWantsLayer:YES];
	[_backView setWantsLayer:YES];
}	

- (void) dealloc {
	[_frontView release];
	[_backView release];
	[super dealloc];
}

@end
