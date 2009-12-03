#import <UIKit/UIKit.h>
#import "MWInputAlertView.h"

@class MWMethod;
@class MWFavoritesTableViewController;
@class MWFavorite;

@interface MWParameterTableViewController : UITableViewController <UIActionSheetDelegate, MWInputDelegate> {
	MWMethod* _method;
	MWFavoritesTableViewController* favorites;
	MWFavorite* _creatingFavorite;
}

@property (nonatomic, retain) MWMethod* method;
@property (nonatomic, assign) IBOutlet MWFavoritesTableViewController* favorites;

- (IBAction) doExecute;

@end
