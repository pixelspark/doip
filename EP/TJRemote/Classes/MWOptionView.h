#import <UIKit/UIKit.h>
#import "MWChooserController.h"

@class MWParameter;

@interface MWOptionView : UIButton <MWChooserDelegate> {
	MWParameter* _parameter;
	BOOL _immediate;
	UIViewController* _parentViewController;
}

- (id) initWithFrame:(CGRect)frame parameter:(MWParameter*)param immediate:(BOOL)imm;
- (void) updateLabel;
- (void) chooser:(MWChooserController*)ch hasChangedParameter:(MWParameter*)param;

@property (nonatomic, retain) MWParameter* parameter;
@property (nonatomic, assign) UIViewController* parentViewController;


@end
