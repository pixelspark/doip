#import "MWAppDelegate.h"
#import "MWClient.h"
#import "MWEndpointsTableViewController.h"

@implementation MWAppDelegate
@synthesize window;
@synthesize tabController, endpointsController;
@synthesize endpointsNavigationController;
@synthesize splashThrobber, splashBackground, regularBackground;

- (void) timeoutTimer:(NSTimer*)timer {
	if([[[MWClient sharedInstance] resolvedEndpoints] count]==0) {
		UIAlertView* av = [[UIAlertView alloc] initWithTitle:@"No devices found" message:@"No remotely controllable devices could be found on your network. Please check whether you are connected to the network. " delegate:nil cancelButtonTitle:nil otherButtonTitles:@"Keep looking",nil];
		[av show];
		[av autorelease];
	}
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	startingUp = YES;
	
    [window makeKeyAndVisible];
	MWClient* client = [MWClient sharedInstance];
	client.delegate = self;
	
	// Set time-out timer
	NSTimer* timer = [NSTimer timerWithTimeInterval:5.0 target:self selector:@selector(timeoutTimer:) userInfo:nil repeats:NO];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	[endpointsNavigationController popToRootViewControllerAnimated:YES];
}

- (void)client:(MWClient*)c foundServiceRemoved:(NSNetService*)s {
	[endpointsController.tableView reloadData];
	
	if([endpointsController.selected.service isEqual:s]) {
		[endpointsNavigationController popToRootViewControllerAnimated:YES];
	}
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
	
	[endpointsController.tableView reloadData];
}

- (void)dealloc {
    [window release];
    [super dealloc];
	[endpointsController release];
	[endpointsNavigationController release];
}

@end
