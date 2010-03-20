#import "MWAppDelegate.h"
#import "MWClient.h"
#import "MWEndpointsTableViewController.h"
#import "MWFavoritesTableViewController.h"

@implementation MWAppDelegate
@synthesize window;
@synthesize endpointsController, favoritesController;
@synthesize endpointsNavigationController, chooserController;
@synthesize splashThrobber, splashBackground, regularBackground;

#ifdef TARGET_IPAD
	@synthesize splitViewController, splitViewDelegate;
#endif

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
	
	#ifdef TARGET_IPAD
		[splitViewController setDelegate:splitViewDelegate];
	#endif
	
	// Set time-out timer
	NSTimer* timer = [NSTimer timerWithTimeInterval:5.0 target:self selector:@selector(timeoutTimer:) userInfo:nil repeats:NO];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	[endpointsNavigationController popToRootViewControllerAnimated:YES];
}

- (void)client:(MWClient*)c foundServiceRemoved:(NSNetService*)s {
	[endpointsController reload];
	
	if([endpointsController.selected.service isEqual:s]) {
		[endpointsNavigationController popToRootViewControllerAnimated:YES];
	}
}

- (void)client:(MWClient*)c foundService:(NSNetService*)s {
	if(startingUp) {
		startingUp = NO;
		#ifdef TARGET_IPAD
			[splitViewController.view setOpaque:NO];
			[splitViewController.view setBackgroundColor:[UIColor clearColor]];
			[window addSubview:splitViewController.view];
			[splitViewController.view setAlpha:0.0f];
		#else 
			[window addSubview:endpointsNavigationController.view];
			[endpointsNavigationController.view setAlpha:0.0f];
		
			if([[favoritesController favorites] count]>0) {
				[endpointsNavigationController pushViewController:favoritesController animated:NO];
			}
		#endif
		
		[UIView beginAnimations:nil context:nil];
		#ifdef TARGET_IPAD
			[splitViewController.view setAlpha:1.0f];
		#else
			[endpointsNavigationController.view setAlpha:1.0f];
		#endif
		[regularBackground setAlpha:1.0f];
		[splashThrobber setAlpha:0.0f];
		[splashBackground setAlpha:0.0f];
		[UIView commitAnimations];
	}
	
	[endpointsController reload];
}

- (void)dealloc {
    [window release];
    [super dealloc];
	[endpointsController release];
	[endpointsNavigationController release];
}

@end
