#import <UIKit/UIKit.h>
#import "MWClient.h"

@interface MWAppDelegate : NSObject <UIApplicationDelegate, MWClientDelegate> {
    UIWindow *window;
	UIViewController* tabController;
	UITableViewController* servicesController;
	UITableViewController* endpointsController;
	bool startingUp;
	
	
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
@property (nonatomic, retain) IBOutlet UITableViewController* servicesController;
@property (nonatomic, retain) IBOutlet UITableViewController* endpointsController;

@end

