#import "MWMethodTableViewController.h"

@implementation MWMethodTableViewController
@dynamic endpoint;

- (MWEndpoint*) endpoint {
	return _endpoint;
}

- (void) setEndpoint:(MWEndpoint*)endpoint {
	[_endpoint release];
	_endpoint = endpoint;
	[_endpoint retain];
	[self.navigationItem setTitle:[[_endpoint service] name]];
	[self.tableView reloadData];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewWillAppear:(BOOL)a {
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:0.0f green:1.0f blue:0.0f alpha:0.4f]];
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self.endpoint methods] count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	MWMethod* method = [[self.endpoint methods] objectAtIndex:indexPath.row];
	[method setupCell:cell];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row<[[self.endpoint methods] count]) {
		MWMethod* method = [[self.endpoint methods] objectAtIndex:[indexPath row]];
		[self.endpoint executeMethod:method];
	}
	else {
		[self.tableView reloadData];
	}
}

- (void)dealloc {
	[_endpoint release];
    [super dealloc];
}


@end

