#import "MWAppDelegate.h"
#import "MWClient.h"

@implementation MWAppDelegate
@synthesize window;
@synthesize tabController, servicesController;
@synthesize splashThrobber, splashBackground, regularBackground;

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    // Override point for customization after application launch
	startingUp = YES;
	
    [window makeKeyAndVisible];
	MWClient* client = [MWClient sharedInstance];
	client.delegate = self;
}

- (void)client:(MWClient*)c foundServiceRemoved:(NSNetService*)s {
	[servicesController.tableView reloadData];
}

- (void)client:(MWClient*)c foundService:(NSNetService*)s {
	if(startingUp) {
		startingUp = NO;
		[window addSubview:tabController.view];
		[tabController.view setAlpha:0.0f];
		[UIView beginAnimations:nil context:nil];
		[tabController.view setAlpha:1.0f];
		[regularBackground setAlpha:1.0f];
		[splashThrobber setAlpha:0.0f];
		[splashBackground setAlpha:0.0f];
		[UIView commitAnimations];
	}
	
	[servicesController.tableView reloadData];
}

- (void)dealloc {
    [window release];
    [super dealloc];
}

@end
