#import "MWEndpointsTableViewController.h"
#include "MWClient.h"
#import <UIKit/UITableViewController.h>
#import <UIKit/UITableViewCell.h>

@implementation MWEndpointsTableViewController
@synthesize methodViewController = _methodViewController;
@synthesize selected = _selected;

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
}

- (void)viewWillAppear:(BOOL)a {
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:0.0f green:1.0f blue:0.0f alpha:0.4f]];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	MWClient* client = [MWClient sharedInstance];
    return [[client resolvedEndpoints] count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
    
	MWClient* client = [MWClient sharedInstance];
	MWEndpoint* ep = [[client resolvedEndpoints] objectAtIndex:indexPath.row];
	cell.textLabel.text = [ep name];
	cell.textLabel.textColor = [UIColor whiteColor];
	
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	MWClient* client = [MWClient sharedInstance];
	if(indexPath.row<[[client resolvedEndpoints] count]) {
		MWEndpoint* endpoint = [[client resolvedEndpoints] objectAtIndex:indexPath.row];
		[_methodViewController setEndpoint:endpoint];
		_selected = endpoint;
		[self.navigationController pushViewController:_methodViewController animated:YES];
	}
	else {
		[self.tableView reloadData];
	}
}

- (void)dealloc {
    [super dealloc];
}


@end

