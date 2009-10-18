#import <UIKit/UIKit.h>
#import "MWMethodTableViewController.h"

@interface MWEndpointsTableViewController : UITableViewController {
	MWMethodTableViewController* _methodViewController;
	MWEndpoint* _selected;
}

@property (nonatomic, retain) IBOutlet MWMethodTableViewController* methodViewController;
@property (nonatomic, assign, readonly) MWEndpoint* selected;

@end
