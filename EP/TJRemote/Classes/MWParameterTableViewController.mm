#import "MWParameterTableViewController.h"
#import "MWMethod.h"

@implementation MWParameterTableViewController
@dynamic method;

- (MWMethod*) method {
	return _method;
}

- (void) setMethod: (MWMethod*)m {
	[_method release];
	_method = m;
	[_method retain];
	[self.navigationItem setTitle:_method.friendlyName];
	[self.tableView reloadData];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (void)viewWillAppear:(BOOL)a {
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:0.0f green:1.0f blue:0.0f alpha:0.4f]];
}

- (void)viewDidDisappear:(BOOL)animated {
	[self setMethod:nil];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return (_method!=nil) ? [_method.parameters count] : 0;
}

- (void) doExecute {
	[_method execute];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
	
	cell.textLabel.textColor = [UIColor whiteColor];
	[[cell.contentView viewWithTag:1337] removeFromSuperview];
    
	MWParameter* parameter = [_method.parameters objectAtIndex:indexPath.row];
	if(parameter!=nil) {
		CGRect rect = CGRectMake(160, 8, 150, 28);
		UIView* pv = [parameter createView:rect immediate:FALSE];
		if(pv!=nil) {
			pv.tag = 1337;
			[cell.contentView addSubview:pv];
		}
		
		cell.textLabel.text = parameter.friendlyName;
	}
	
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[self.tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)dealloc {
	[_method release];
    [super dealloc];
}

@end

