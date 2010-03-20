#import <UIKit/UIKit.h>
#import "MWMethodTableViewController.h"

@interface MWEndpointsTableViewController : UITableViewController {
	MWMethodTableViewController* _methodViewController;
	MWEndpoint* _selected;
	UINavigationController* _rightNavigationController;
	UIViewController* _favoritesController;
}

- (void)reload;

@property (nonatomic, retain) IBOutlet MWMethodTableViewController* methodViewController;
@property (nonatomic, assign, readonly) MWEndpoint* selected;
@property (nonatomic, retain) IBOutlet UINavigationController* rightNavigationController;
@property (nonatomic, retain) IBOutlet UIViewController* favoritesController;

@end
