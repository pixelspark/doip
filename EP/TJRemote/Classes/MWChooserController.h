#import <UIKit/UIKit.h>

@class MWParameter;
@class MWChooserController;

@protocol MWChooserDelegate
- (void) chooser:(MWChooserController*)ch hasChangedParameter:(MWParameter*)param;
@end


@interface MWChooserController : UITableViewController {
	MWParameter* _parameter;
	UITableView* _tableView;
	UIBarButtonItem* _titleLabel;
	id<MWChooserDelegate> _choiceDelegate;
}

- (IBAction) onDone:(id)sender;
- (void) dismiss;
- (void) showInView:(UIViewController*)view;

@property (nonatomic, retain) MWParameter* parameter;
@property (nonatomic, retain) IBOutlet UITableView* tableView;
@property (nonatomic, retain) IBOutlet UIBarButtonItem* titleLabel;
@property (nonatomic, assign) id<MWChooserDelegate> choiceDelegate;

@end
