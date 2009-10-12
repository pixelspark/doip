#import <UIKit/UIKit.h>
#import "MWMethodTableViewController.h"

@interface MWEndpointsTableViewController : UITableViewController {
	MWMethodTableViewController* _methodViewController;
}

@property (nonatomic, retain) IBOutlet MWMethodTableViewController* methodViewController;

@end
