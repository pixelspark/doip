#import "MWMethodTableViewController.h"
#import "MWParameterTableViewController.h"

@implementation MWMethodTableViewController
@dynamic endpoint;
@synthesize _parameterViewController;

#ifdef TARGET_IPAD
	@synthesize popOverController = _popOverController;
#endif

- (MWEndpoint*) endpoint {
	return _endpoint;
}

- (void) setEndpoint:(MWEndpoint*)endpoint {
	[_endpoint release];
	_endpoint = endpoint;
	[_endpoint retain];
	[self.navigationItem setTitle:[[_endpoint service] name]];
	[self.tableView reloadData];
	
	#ifdef TARGET_IPAD
		if (_popOverController != nil) {
			[_popOverController dismissPopoverAnimated:YES];
		} 
	#endif
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewWillAppear:(BOOL)a {
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:0.2f]];
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


#pragma mark Table view methods

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

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
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
	
	MWMethod* method = [[self.endpoint methods] objectAtIndex:indexPath.row];
	[method setupCell:cell inController:self];
    return cell;
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row<[[self.endpoint methods] count]) {
		MWMethod* method = [[self.endpoint methods] objectAtIndex:[indexPath row]];
		_parameterViewController.method = method;
		[self.navigationController pushViewController:_parameterViewController animated:YES];
	}
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row<[[self.endpoint methods] count]) {
		MWMethod* method = [[self.endpoint methods] objectAtIndex:[indexPath row]];
		if([method.parameters count]==0) {
			[self.endpoint executeMethod:method];
		}
		else if([method parametersFitInCell]) {
			// Nothing, the control will start the execution
		}
		else {
			_parameterViewController.method = method;
			[self.navigationController pushViewController:_parameterViewController animated:YES];
		}
		[self.tableView deselectRowAtIndexPath:indexPath animated:YES];
	}
	else {
		[self.tableView reloadData];
	}
}

#ifdef TARGET_IPAD
- (void)splitViewController: (UISplitViewController*)svc willHideViewController:(UIViewController *)aViewController withBarButtonItem:(UIBarButtonItem*)barButtonItem forPopoverController: (UIPopoverController*)pc {
    UINavigationBar* navigationBar = [[self navigationController] navigationBar];
	barButtonItem.title = @"Master List";
    [navigationBar.topItem setLeftBarButtonItem:barButtonItem animated:YES];
	NSLog(@"willHide %@", navigationBar);
    self.popOverController = pc;
}


// Called when the view is shown again in the split view, invalidating the button and popover controller.
- (void)splitViewController: (UISplitViewController*)svc willShowViewController:(UIViewController *)aViewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem {
    UINavigationBar* navigationBar = [[self navigationController] navigationBar];
	[navigationBar.topItem setLeftBarButtonItem:nil animated:YES];
	NSLog(@"WillShow %@", navigationBar);
    self.popOverController = nil;
}
#endif

- (void)dealloc {
	[_endpoint release];
	[_parameterViewController release];
	
	#ifdef TARGET_IPAD
		[_popOverController release];
	#endif
    [super dealloc];
}


@end

