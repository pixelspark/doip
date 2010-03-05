#import <UIKit/UIKit.h>
#include "MWEndpoint.h"

@class MWParameterTableViewController;

#ifdef TARGET_IPAD
	@interface MWMethodTableViewController : UITableViewController <UIPopoverControllerDelegate, UISplitViewControllerDelegate> {
#else
	@interface MWMethodTableViewController : UITableViewController {
#endif
	MWEndpoint* _endpoint;
	MWParameterTableViewController* _parameterViewController;
	
	#ifdef TARGET_IPAD
		UIPopoverController* _popOverController;
	#endif
}

@property (nonatomic, retain) MWEndpoint* endpoint;
@property (nonatomic, retain) IBOutlet MWParameterTableViewController* _parameterViewController;
#ifdef TARGET_IPAD
		@property (nonatomic, retain) UIPopoverController* popOverController;
#endif
		
@end
