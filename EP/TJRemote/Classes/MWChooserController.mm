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
 
 #import "MWChooserController.h"
#import "MWParameter.h"
#import "MWAppDelegate.h"

@implementation MWChooserController
@synthesize parameter = _parameter;
@synthesize tableView = _tableView;
@synthesize choiceDelegate = _choiceDelegate;
@synthesize titleLabel = _titleLabel;

- (IBAction) onDone:(id)sender {
	[self dismiss];
}

- (void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
	[self.view removeFromSuperview];
}

- (void) dismiss {
	[self.parentViewController dismissModalViewControllerAnimated:YES];
}

- (void) showInView:(UIViewController*)parent {
	[_titleLabel setTitle:_parameter.friendlyName];
	[parent presentModalViewController:self animated:YES];
}

- (void)viewWillAppear:(BOOL)animated {
	[_tableView setSeparatorColor:[UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.3]];
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
	[super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(section==0) {
		return [_parameter.options count];
	}
	return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section==0) {
		static NSString *CellIdentifier = @"Cell";
		
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
		if (cell == nil) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		}
		
		NSString* key = [[_parameter.options allKeys] objectAtIndex:indexPath.row];
		cell.textLabel.text = key;
		cell.textLabel.textColor = [UIColor whiteColor];
		return cell;
	}
	return nil;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	// Set parameter value
	if(indexPath.section==0) {
		NSString* key = [[_parameter.options allKeys] objectAtIndex:indexPath.row];
		NSString* value = [_parameter.options objectForKey:key];
		[_parameter setValue:value];
		[_choiceDelegate chooser:self hasChangedParameter:_parameter];
	}
}

- (void)dealloc {
	[_parameter release];
	[_tableView release];
    [super dealloc];
}


@end

