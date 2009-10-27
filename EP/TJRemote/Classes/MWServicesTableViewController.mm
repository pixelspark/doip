#import "MWServicesTableViewController.h"
#import "MWClient.h"
#import <UIKit/UITableViewController.h>
#import <UIKit/UITableViewCell.h>

@implementation MWServicesTableViewController
@synthesize ledController;

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
    return [[client resolvedServices] count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	MWClient* client = [MWClient sharedInstance];
	NSNetService* service = [[client resolvedServices] objectAtIndex:indexPath.row];
	
	if(service!=nil) {
		cell.textLabel.text = [service name];
		cell.textLabel.textColor = [UIColor whiteColor];
	}
	
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	MWClient* client = [MWClient sharedInstance];
	if(indexPath.row<[[client resolvedServices] count]) {
		NSNetService* service = [[client resolvedServices] objectAtIndex:indexPath.row];
		[ledController setDeviceService:service];
		[self.navigationController pushViewController:ledController animated:YES];
	}
	else {
		[self.tableView reloadData];
	}
}

- (void)dealloc {
	[ledController release];
    [super dealloc];
}


@end

