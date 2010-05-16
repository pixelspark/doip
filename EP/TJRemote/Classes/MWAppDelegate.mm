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
