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
 
 #import "MWSliderView.h"
#import "MWMethod.h"

@implementation MWSliderView

- (void) updateLabel {
	if(_parameter.discrete) {
		[_label setText:[NSString stringWithFormat:@"%d",[[_parameter value] intValue]]];
	}
	else {
		[_label setText:[NSString stringWithFormat:@"%0.2f",[[_parameter value] floatValue]]];
	}
};

- (void) parameterValueChanged:(NSNotification*)nt {
	float val = [_parameter.value floatValue];
	
	[UIView beginAnimations:NULL context:nil];
	[UIView setAnimationDuration:0.8];
	[UIView setAnimationCurve:UIViewAnimationCurveEaseInOut];
	[_slider setValue:val];
	[UIView commitAnimations];
	[self updateLabel];
}

- (id) initWithFrame:(CGRect)rect parameter:(MWParameter*)parameter immediate:(bool)imm {
	const static int KLabelWidth = 32;
	const static int KButtonWidth = 32;
	
	if(self = [super initWithFrame:rect]) {
		int x = 0;
		int w = rect.size.width;
		_parameter = parameter;
		[_parameter retain];
		
		// Add label and +/- buttons
		if([parameter discrete]) {
			_label = [[UILabel alloc] initWithFrame:CGRectMake(w-KLabelWidth, 0, KLabelWidth, rect.size.height)];
			w -= KLabelWidth;
			[_label setBackgroundColor:[UIColor clearColor]];
			[_label setOpaque:FALSE];
			[_label setTextColor:[UIColor whiteColor]];
			[_label setTextAlignment:UITextAlignmentRight];
			[_label setAutoresizingMask:UIViewAutoresizingFlexibleLeftMargin];
			[self addSubview:_label];
			[self updateLabel];
			
			_plusButton = [[UIButton alloc] initWithFrame:CGRectMake(w-KButtonWidth, 0, KButtonWidth, rect.size.height)];
			[_plusButton setTitle:@"+" forState:UIControlStateNormal];
			[_plusButton setAutoresizingMask:UIViewAutoresizingFlexibleLeftMargin];
			[_plusButton addTarget:self action:@selector(increment:event:) forControlEvents:UIControlEventTouchUpInside];
			[_plusButton setShowsTouchWhenHighlighted:YES];
			[self addSubview:_plusButton];
			w -= KButtonWidth;
			
			_minButton = [[UIButton alloc] initWithFrame:CGRectMake(x, 0, KButtonWidth, rect.size.height)];
			[_minButton setTitle:@"-" forState:UIControlStateNormal];
			[_minButton setAutoresizingMask:UIViewAutoresizingFlexibleLeftMargin];
			[_minButton addTarget:self action:@selector(decrement:event:) forControlEvents:UIControlEventTouchUpInside];
			[_minButton setShowsTouchWhenHighlighted:YES];
			[self addSubview:_minButton];
			x += KButtonWidth;
			w -= KButtonWidth;
		}
		
		_slider = [[UISlider alloc] initWithFrame:CGRectMake(x, 0, w, rect.size.height)];
		[_slider setMinimumValue:[[parameter minimumValue] floatValue]];
		[_slider setMaximumValue:[[parameter maximumValue] floatValue]];
		[_slider setValue:[[parameter value] floatValue]];
		[_slider addTarget:self action:@selector(sliderValueChanged:event:) forControlEvents:UIControlEventValueChanged];
		if(imm) {
			[_slider addTarget:parameter action:@selector(executeHandler:event:) forControlEvents:UIControlEventValueChanged];
		}
		[_slider setAutoresizingMask:UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
		[self addSubview:_slider];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(parameterValueChanged:) name:@"MWParameterValueChange" object:parameter];
		
	}
	return self;
}
- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt {
	[_parameter sliderValueChanged:slider event:evt];
	[self updateLabel];
};

- (void) increment: (UIView*)view event:(UIEvent*)evt {
	float val = [[_parameter value] floatValue];
	float maxVal = [[_parameter maximumValue] floatValue];
	val += 1.0f;
	if(val > maxVal) {
		val = maxVal;
	}
	[_parameter setValue:[NSString stringWithFormat:@"%f",val]];
	[_slider setValue:val];
	[self updateLabel];
}

- (void) decrement: (UIView*)view event:(UIEvent*)evt {
	float val = [[_parameter value] floatValue];
	float minVal = [[_parameter minimumValue] floatValue];
	val -= 1.0f;
	if(val < minVal) {
		val = minVal;
	}
	[_parameter setValue:[NSString stringWithFormat:@"%f",val]];
	[_slider setValue:val];
	[self updateLabel];
}

- (void) dealloc {
	[_parameter release];
	[_slider release];
	[_label release];
	[_minButton release];
	[_plusButton release];
	[super dealloc];
}

@end
