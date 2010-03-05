#import "MWEndpointsTableViewController.h"
#include "MWClient.h"
#import <UIKit/UITableViewController.h>
#import <UIKit/UITableViewCell.h>
#import "MWFavoritesTableViewController.h"

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
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:0.2f]];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return YES;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	MWClient* client = [MWClient sharedInstance];
    return [[client resolvedEndpoints] count]+1;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	if(indexPath.row>0) {
		MWClient* client = [MWClient sharedInstance];
		MWEndpoint* ep = [[client resolvedEndpoints] objectAtIndex:indexPath.row-1];
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
	if(indexPath.row==0) {
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
		MWClient* client = [MWClient sharedInstance];
		int r = indexPath.row-1;
		if(r<[[client resolvedEndpoints] count]) {
			MWEndpoint* endpoint = [[client resolvedEndpoints] objectAtIndex:r];
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
		else {
			[self.tableView reloadData];
		}
	}
}

- (void)dealloc {
    [super dealloc];
	[_rightNavigationController release];
	[_methodViewController release];
	[_favoritesController release];
}


@end

