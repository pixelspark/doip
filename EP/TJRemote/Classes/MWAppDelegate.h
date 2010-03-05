#import <UIKit/UIKit.h>
#import "MWClient.h"

@class MWChooserController;
@class MWEndpointsTableViewController;
@class MWFavoritesTableViewController;

@interface MWAppDelegate : NSObject <UIApplicationDelegate, MWClientDelegate> {
    UIWindow *window;
	MWEndpointsTableViewController* endpointsController;
	UINavigationController* endpointsNavigationController;
	
	#ifdef TARGET_IPAD
		id<UISplitViewControllerDelegate> splitViewDelegate;
		UISplitViewController* splitViewController;
	#endif
	
	bool startingUp;
	MWChooserController* chooserController;
	MWFavoritesTableViewController* favoritesController;
	
	// Splash screen items
	UIView* regularBackground;
	UIView* splashBackground;
	UIView* splashThrobber;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UIView* regularBackground;
@property (nonatomic, retain) IBOutlet UIView* splashBackground;
@property (nonatomic, retain) IBOutlet UIView* splashThrobber;
@property (nonatomic, retain) IBOutlet MWEndpointsTableViewController* endpointsController;
@property (nonatomic, retain) IBOutlet UINavigationController* endpointsNavigationController;
@property (nonatomic, retain) IBOutlet MWChooserController* chooserController;
@property (nonatomic, retain) IBOutlet MWFavoritesTableViewController* favoritesController;

#ifdef TARGET_IPAD
	@property (nonatomic, retain) IBOutlet UISplitViewController* splitViewController;
	@property (nonatomic, retain) IBOutlet id<UISplitViewControllerDelegate> splitViewDelegate;
#endif

@end

