#import <UIKit/UIKit.h>
#import "MWLedDeviceViewController.h"

@interface MWServicesTableViewController : UITableViewController {
	MWLedDeviceViewController* ledController;
}

@property (nonatomic, retain) IBOutlet MWLedDeviceViewController* ledController;

@end
