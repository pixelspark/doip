#import "MWFavoritesTableViewController.h"
#import "MWEndpoint.h"
#import "MWMethod.h"
#import "MWClient.h"

static UIImage* favoriteImage;
static UIImage* favoriteAllImage;

@implementation MWFavoritesTableViewController
@synthesize favorites = _favorites;

+ (void) initialize {
	NSString* pngPath = [[NSBundle mainBundle] pathForResource:@"Favorite" ofType:@"png"];
	favoriteImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
	
	pngPath = [[NSBundle mainBundle] pathForResource:@"FavoriteAll" ofType:@"png"];
	favoriteAllImage = [[UIImage imageWithContentsOfFile:pngPath] retain];
}

- (NSString*) persistentPath {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	NSString *documentsDirectoryPath = [paths objectAtIndex:0];
	NSString *databaseFile = [documentsDirectoryPath stringByAppendingPathComponent:@"favorites.plist"];
	//[databaseFile autorelease];
	return databaseFile;
}
	
- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
}

- (void) persist {
	if(![NSKeyedArchiver archiveRootObject:self.favorites toFile:[self persistentPath]]) {
		NSLog(@"Archival failed..");
	}
}

- (void) addFavorite:(MWFavorite *)fav {
	[_favorites addObject:fav];
	[self.tableView reloadData];
	[self persist];
}

- (void)viewWillAppear:(BOOL)animated {
}

- (void)viewDidLoad {
	[self.navigationItem setLeftBarButtonItem:[self editButtonItem] animated:NO];
	
	self.favorites = [NSKeyedUnarchiver unarchiveObjectWithFile:[self persistentPath]];
	if(!self.favorites) {
		self.favorites = [[[NSMutableArray alloc] init] autorelease];
	}
	[self.tableView setSeparatorColor:[UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:0.2f]];
	[super viewDidLoad];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [_favorites count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	MWFavorite* fav = [_favorites objectAtIndex:indexPath.row];
	cell.textLabel.text = [fav friendlyName];
	cell.textLabel.textColor = [UIColor whiteColor];
	if([fav specificDevice]) {
		cell.imageView.image = favoriteImage;
	}
	else {
		cell.imageView.image = favoriteAllImage;
	}
	
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if(indexPath.section==0) {
		MWFavorite* fav = [_favorites objectAtIndex:indexPath.row];
		MWClient* client = [MWClient sharedInstance];

		bool success = false;
		for(MWEndpoint* ep in [client resolvedEndpoints]) {
			if(!fav.specificDevice || [ep.endpointIdentifier isEqualToString:fav.specificDevice]) {
				success = [ep executeFavorite:fav] || success;
			}
		}
		
		if(!success) {
			NSString* msg;
			if(fav.specificDevice) {
				msg = @"The favorite action could not be executed, because the device for which it was created cannot be found.";
			}
			else {
				msg = @"The favorite action could not be executed, because there are no devices available that support the command.";
			}
			UIAlertView* av = [[UIAlertView alloc] initWithTitle:@"No supported devices found" message:msg delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK",nil];
			[av show];
			[av autorelease];			
		}
		
		[tableView deselectRowAtIndexPath:indexPath animated:YES];
	}
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
		[_favorites removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationRight];
		[self persist];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}

- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
	NSObject* to = [[_favorites objectAtIndex:toIndexPath.row] retain];
	[_favorites replaceObjectAtIndex:toIndexPath.row withObject:[_favorites objectAtIndex:fromIndexPath.row]];
	[_favorites replaceObjectAtIndex:fromIndexPath.row withObject:to];
	[to release];
	[self persist];
}

- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}

- (void)dealloc {
	[_favorites release];
    [super dealloc];
}

@end

