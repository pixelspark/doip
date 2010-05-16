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
 
 #import "MWInputAlertView.h"

@implementation MWInputAlertView
@synthesize field = _field;

- (id) initWithTitle:(NSString*)title placeholder:(NSString*)placeholder value:(NSString*)value  withContext:(NSObject*)context delegate:(id<MWInputDelegate>)d {
	if(self = [super initWithTitle:title message:@"This gets covered" delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK",nil]) {
		_icontext = context;
		[_icontext retain];
		super.delegate = self;
		_idelegate = d;
		self.field = [[UITextField alloc] initWithFrame:CGRectMake(12.0, 45.0, 260.0, 25.0)];
		//[myTextField setBackgroundColor:[UIColor whiteColor]];
		[self.field setOpaque:FALSE];
		[self.field setKeyboardType:UIKeyboardTypeDefault];
		[self.field setKeyboardAppearance:UIKeyboardAppearanceAlert];
		[self.field setAutocorrectionType:UITextAutocorrectionTypeNo];
		[self.field setTag:1337];
		[self.field setText:value];
		[self.field setBorderStyle:UITextBorderStyleRoundedRect];
		[self.field setPlaceholder:placeholder];
		[self addSubview:self.field];
		
		CGAffineTransform myTransform = CGAffineTransformMakeTranslation(0.0, 110.0);
		[self setTransform:myTransform];
	}
	return self;
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
	[_idelegate inputAlert:self didClose:self.field.text withContext:_icontext];
}

- (void) show {
	[super show];
	[self.field becomeFirstResponder];
}

- (void) dealloc {
	[_field release];
	[_icontext release];
	[super dealloc];
}

@end
