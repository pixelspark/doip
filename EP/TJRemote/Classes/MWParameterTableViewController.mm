#import "MWParameterTableViewController.h"
#import "MWMethod.h"
#import "MWHeaderView.h"

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
    return 2;
}

- (void)viewWillAppear:(BOOL)a {
	[self.tableView setAllowsSelection:YES];
	[self.tableView reloadData];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:0.0f green:1.0f blue:0.0f alpha:0.4f]];
}

- (CGFloat) tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	return 27.0;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	if(section==0) {
		return [[MWHeaderView alloc] initWithFrame:CGRectMake(10.0, 0.0, 300.0, 27.0) andTitle:@"Parameters"];
	}
	else {
		return [[MWHeaderView alloc] initWithFrame:CGRectMake(10.0, 0.0, 300.0, 27.0) andTitle:@"Actions"];
	}
}

- (void)viewDidDisappear:(BOOL)animated {
	[self setMethod:nil];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(section==0) {
		return (_method!=nil) ? [_method.parameters count] : 0;
	}
	else {
		return (_method!=nil) ? 1 : 0;
	}
}

- (void) doExecute {
	[_method execute];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString* CellIdentifier = @"Cell";
	static NSString* ButtonCellIdentifier = @"Button";
	
	if(indexPath.section==0) {
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
		if (cell == nil) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		}
		
		cell.textLabel.textColor = [UIColor whiteColor];
		[[cell.contentView viewWithTag:1337] removeFromSuperview];
		
		MWParameter* parameter = [_method.parameters objectAtIndex:indexPath.row];
		if(parameter!=nil) {
			CGRect rect = CGRectMake(100, 8, 210, 28);
			UIView* pv = [parameter createView:rect immediate:FALSE];
			if(pv!=nil) {
				pv.tag = 1337;
				[cell.contentView addSubview:pv];
			}
			
			cell.textLabel.text = parameter.friendlyName;
			cell.textLabel.font = [UIFont boldSystemFontOfSize:15];
			[cell setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.9]];
		}
		
		return cell;
	}
	else if(indexPath.section==1) {
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:ButtonCellIdentifier];
		if (cell == nil) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		}
		cell.textLabel.textColor = [UIColor whiteColor];
		[[cell.contentView viewWithTag:1337] removeFromSuperview];
		cell.textLabel.text = @"Confirm";
		return cell;
	}
	return nil;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section==0) {
		[self.tableView deselectRowAtIndexPath:indexPath animated:NO];
	}
	else if(indexPath.section==1) {
		// Something else
		[self doExecute];
		[self.tableView deselectRowAtIndexPath:indexPath animated:YES];
	}
}

- (void)dealloc {
	[_method release];
    [super dealloc];
}

@end

