#import <UIKit/UIKit.h>
#import "MWClient.h"

@class MWChooserController;
@class MWEndpointsTableViewController;

@interface MWAppDelegate : NSObject <UIApplicationDelegate, MWClientDelegate> {
    UIWindow *window;
	UIViewController* tabController;
	MWEndpointsTableViewController* endpointsController;
	UINavigationController* endpointsNavigationController;
	bool startingUp;
	MWChooserController* chooserController;
	
	
	// Splash screen items
	UIView* regularBackground;
	UIView* splashBackground;
	UIView* splashThrobber;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UIViewController* tabController;
@property (nonatomic, retain) IBOutlet UIView* regularBackground;
@property (nonatomic, retain) IBOutlet UIView* splashBackground;
@property (nonatomic, retain) IBOutlet UIView* splashThrobber;
@property (nonatomic, retain) IBOutlet MWEndpointsTableViewController* endpointsController;
@property (nonatomic, retain) IBOutlet UINavigationController* endpointsNavigationController;
@property (nonatomic, retain) IBOutlet MWChooserController* chooserController;

@end

