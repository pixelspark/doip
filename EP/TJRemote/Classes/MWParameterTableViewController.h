#import <UIKit/UIKit.h>

@class MWMethod;

@interface MWParameterTableViewController : UITableViewController {
	MWMethod* _method;
}

@property (nonatomic, retain) MWMethod* method;
- (IBAction) doExecute;

@end
