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
 
 #import "MWParameterView.h"
#import "MWParameter.h"
#import "MWSliderView.h"
#import "MWOptionView.h"

@implementation MWParameterView
@synthesize wrappedView = _wrapped;
@synthesize parameter = _parameter;

- (void) update:(NSString*)value {
	if([_parameter.type isEqualToString:@"bool"]) {
		UISwitch* sw = (UISwitch*)self.wrappedView;
		[sw setOn:[value isEqualToString:@"yes"] || [value isEqualToString:@"1"]];
	}
}

- (void) parameterValueChanged:(NSNotification*)ns {
	[self update:[_parameter value]];
}

- (id) initWithParameter:(MWParameter*)parameter inRect:(CGRect)rc immediate:(BOOL)imm inController:(UIViewController*)controller {
	if(self = [super initWithFrame:rc]) {
		CGRect rect = CGRectMake(0, 0, rc.size.width, rc.size.height);
		
		if([parameter.options count]>0) {
			// The values are restricted; create an MWOptionsView
			MWOptionView* mwo = [[MWOptionView alloc] initWithFrame:rect parameter:parameter immediate:imm];
			mwo.parentViewController = controller;
			self.wrappedView = mwo;
			[mwo autorelease];
		}
		else {
			if([parameter.type isEqualToString:@"int32"] || [parameter.type isEqualToString:@"double"]) {
				MWSliderView* sv = [[MWSliderView alloc] initWithFrame:rect parameter:parameter immediate:imm];
				self.wrappedView = sv;
				[sv release];
			}
			else if([parameter.type isEqualToString:@"bool"]) {
				UISwitch* sw = [[UISwitch alloc] initWithFrame:CGRectMake(rect.size.width+rect.origin.x-96, rect.origin.y, 96, rect.size.height)];
				[sw addTarget:parameter action:@selector(switchValueChanged:event:) forControlEvents:UIControlEventValueChanged];
				if(imm) {
					[sw addTarget:parameter action:@selector(executeHandler:event:) forControlEvents:UIControlEventValueChanged];
				}
				self.wrappedView = sw;
				[sw release];
			}
			else if([parameter.type isEqualToString:@"string"]) {
				UITextField* field = [[UITextField alloc] initWithFrame:rect];
				[field setBorderStyle:UITextBorderStyleRoundedRect];
				[field setDelegate:parameter];
				if(imm) {
					[field addTarget:parameter action:@selector(executeHandler:event:) forControlEvents:UIControlEventEditingDidEnd];
				}
				self.wrappedView = field;
				[field release];
			}
		}
		
		if(self.wrappedView) {
			[self setOpaque:FALSE];
			[self addSubview:self.wrappedView];
			self.parameter = parameter;
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(parameterValueChanged:) name:@"MWParameterValueChange" object:parameter];
			[self update:parameter.defaultValue];
		}
		else {
			@throw [NSException exceptionWithName:@"ViewNotImplementedException" reason:@"View for parameter could not be created, because a view for the parameter type is not implemented" userInfo:nil];
		}
	}
	
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[_parameter release];
	[_wrapped release];
    [super dealloc];
}

@end
