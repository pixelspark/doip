#import <UIKit/UIKit.h>
#include "MWEndpoint.h"

@class MWParameterTableViewController;

@interface MWMethodTableViewController : UITableViewController {
	MWEndpoint* _endpoint;
	MWParameterTableViewController* _parameterViewController;
}

@property (nonatomic, retain) MWEndpoint* endpoint;
@property (nonatomic, retain) IBOutlet MWParameterTableViewController* _parameterViewController;

@end
