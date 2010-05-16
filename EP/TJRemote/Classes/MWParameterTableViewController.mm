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
 
 #import "MWParameterTableViewController.h"
#import "MWMethod.h"
#import "MWHeaderView.h"
#import "MWFavoritesTableViewController.h"
#import "MWEndpoint.h"
#import "MWInputAlertView.h"

static UIImage* runImage;
static UIImage* favoriteImage;

@implementation MWParameterTableViewController
@dynamic method;
@synthesize favorites;

+ (void) initialize {
	NSString* pngPath = [[NSBundle mainBundle] pathForResource:@"Run" ofType:@"png"];
	runImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
	
	pngPath = [[NSBundle mainBundle] pathForResource:@"FavoriteAll" ofType:@"png"];
	favoriteImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
}

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
	[self.tableView setSeparatorStyle: UITableViewCellSeparatorStyleNone];
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

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(section==0) {
		return (_method!=nil) ? [_method.parameters count] : 0;
	}
	else {
		return (_method!=nil) ? 2 : 0;
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
		cell.imageView.image = nil;
		
		MWParameter* parameter = [_method.parameters objectAtIndex:indexPath.row];
		if(parameter!=nil) {
			CGRect rect = CGRectMake(100, 8, 210, 28);
			UIView* pv = [parameter createView:rect immediate:FALSE inController:self];
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
		if(indexPath.row==0) {
			cell.imageView.image = runImage;
			cell.textLabel.text = @"Run right now";
		}
		else if(indexPath.row==1) {
			cell.imageView.image = favoriteImage;
			cell.textLabel.text = @"Add to favorites";
		}
		return cell;
	}
	return nil;
}

- (void)inputAlert:(MWInputAlertView*)alert didClose:(NSString*)value withContext:(NSObject*)context {
	if(context!=nil) {
		MWFavorite* fav = (MWFavorite*)context;
		fav.friendlyName = value;
		[favorites addFavorite:fav];
		[fav release];
		_creatingFavorite = nil;
	}
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if(buttonIndex==1) {
		// Just this device; set specific device ID
		_creatingFavorite.specificDevice = [[_method parent] endpointIdentifier];
	}
	
	if(buttonIndex==2) {
		// Cancel
		[_creatingFavorite release];
		_creatingFavorite = nil;
		return;
	}
	
	// Continue with asking for a name
	MWInputAlertView* iv = [[MWInputAlertView alloc] initWithTitle:@"Add to favorites" placeholder:@"Enter name of favorite" value:[_method friendlyName] withContext:_creatingFavorite delegate:self];
	[iv show];
}

- (void) addFavorite {
	_creatingFavorite = [[_method createFavorite] retain];
	UIActionSheet* as = [[UIActionSheet alloc] initWithTitle:@"Do you want this favorite to run on all devices that support this command, or just this device?" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:@"All supported devices",@"Just this device",nil];
	[as setActionSheetStyle:UIActionSheetStyleBlackTranslucent];
	[as showInView:self.tableView];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section==0) {
		[self.tableView deselectRowAtIndexPath:indexPath animated:NO];
	}
	else if(indexPath.section==1) {
		if(indexPath.row==0) {
			[self doExecute];
		}
		else if(indexPath.row==1) {
			[self addFavorite];
		}
		[self.tableView deselectRowAtIndexPath:indexPath animated:YES];
	}
}

- (void)dealloc {
	[_creatingFavorite release];
	[_method release];
    [super dealloc];
}

@end

