#import <UIKit/UIKit.h>
#include "MWEndpoint.h"

@interface MWMethodTableViewController : UITableViewController {
	MWEndpoint* _endpoint;
}

@property (nonatomic, retain) MWEndpoint* endpoint;

@end
