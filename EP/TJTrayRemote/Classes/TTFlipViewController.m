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
 
 #import "TTFlipViewController.h"
#import <QuartzCore/CoreAnimation.h>

@implementation TTFlipViewController
@synthesize backView = _backView;
@synthesize frontView = _frontView;

- (void) setAnimationsEnabled:(BOOL)enabled forward:(BOOL)forward {
	NSView *contentView = self.view;
    [contentView setWantsLayer:YES];
	
    CATransition* transition = [CATransition animation];
	if(enabled) {
		[transition setType:@"cube"];
		[transition setSubtype:forward ? kCATransitionFromRight: kCATransitionFromLeft];
	}
	else {
		[transition setType:@""];
	}
	
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
		[self setAnimationsEnabled:YES forward:YES];
		[[self.view animator] replaceSubview:_frontView with:_backView];
		[self setAnimationsEnabled:NO forward:YES];
	}
	else {
		[self setAnimationsEnabled:YES forward:NO];
		[[self.view animator] replaceSubview:_backView with:_frontView];
		[self setAnimationsEnabled:NO forward:YES];
	}
}

- (void)setView:(NSView *)view {
	[super setView:view];
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
