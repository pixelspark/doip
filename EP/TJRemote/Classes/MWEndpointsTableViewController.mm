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
 
 #import "MWEndpointsTableViewController.h"
#include "MWClient.h"
#import <UIKit/UITableViewController.h>
#import <UIKit/UITableViewCell.h>
#import "MWFavoritesTableViewController.h"
#import "MWHeaderView.h"

@implementation MWEndpointsTableViewController
@synthesize methodViewController = _methodViewController;
@synthesize selected = _selected;
@synthesize rightNavigationController = _rightNavigationController;
@synthesize favoritesController = _favoritesController;

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
}

- (void)viewWillAppear:(BOOL)a {
	NSIndexSet* nsi = [[NSIndexSet alloc] initWithIndex:1];
	[self.tableView reloadSections:nsi withRowAnimation:UITableViewRowAnimationNone];
	[nsi dealloc];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:0.2f]];
}

- (void) reload {
	NSIndexSet* nsi = [[NSIndexSet alloc] initWithIndex:1];
	[self.tableView reloadSections:nsi withRowAnimation:UITableViewRowAnimationFade];
	[nsi dealloc];
	[self.tableView setNeedsDisplay];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 2;
}

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return YES;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(section==0) {
		return 1;
	}
	else if(section==1) {
		MWClient* client = [MWClient sharedInstance];
		
		if(false) {
			int n = 0;
			for(MWEndpoint* ep in [client resolvedEndpoints]) {
				if([[ep methods] count] > 0) {
					++n;
				}
			}
			return n;
		}
		else {
			return [[client resolvedEndpoints] count];
		}
	}
	return 0;
}

- (MWEndpoint*) endpointForRow:(int)i {
	MWClient* client = [MWClient sharedInstance];
	if(false) {
		int n = 0;
		for(MWEndpoint* ep in [client resolvedEndpoints]) {
			if([[ep methods] count] > 0) {
				if(n==i) {
					return ep;
				}
				else {
					++n;
				}
			}
		}
		return nil;
	}
	else {
		return [[client resolvedEndpoints] objectAtIndex:i];
	}
}

- (CGFloat) tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	if(section==1) {
		return 27.0;
	}
	return 0.0;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	if(section==1) {
		return [[MWHeaderView alloc] initWithFrame:CGRectMake(10.0, 0.0, 300.0, 27.0) andTitle:@"Devices"];
	}
	return nil;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	if(indexPath.section==1) {
		MWEndpoint* ep = [self endpointForRow:indexPath.row];
		cell.textLabel.text = [ep name];
		cell.imageView.image = nil;
	}
	else {
		cell.textLabel.text = @"Favorite actions";
		cell.imageView.image = [MWFavoritesTableViewController favoriteImage];
	}
	cell.textLabel.textColor = [UIColor whiteColor];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section==0) {
		#ifdef TARGET_IPAD
			[self.rightNavigationController popToViewController:_favoritesController animated:YES];
		#else
			@try {
				[self.navigationController pushViewController:_favoritesController animated:YES];
			}
			@catch (NSException* e) {
			}
		#endif
	}
	else {
		MWEndpoint* endpoint = [self endpointForRow:indexPath.row];
		if(endpoint!=nil) {
			[_methodViewController setEndpoint:endpoint];
			_selected = endpoint;
			
			#ifndef TARGET_IPAD
				[self.navigationController pushViewController:_methodViewController animated:YES];
			#else
				if([self.rightNavigationController topViewController]!=_methodViewController) {
					@try {
						[self.rightNavigationController pushViewController:_methodViewController animated:YES];
					}
					@catch (NSException* e) {
					}
					[self.rightNavigationController popToViewController:_methodViewController animated:YES];
				}
				else {
					[self.rightNavigationController popToViewController:_methodViewController animated:YES];
				}
			#endif
		}
	}
	[tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)dealloc {
    [super dealloc];
	[_rightNavigationController release];
	[_methodViewController release];
	[_favoritesController release];
}


@end

