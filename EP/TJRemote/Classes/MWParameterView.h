#import <UIKit/UIKit.h>

@class MWParameter;

@interface MWParameterView : UIView {
	UIView* _wrapped;
	MWParameter* _parameter;
}

- (id) initWithParameter:(MWParameter*)p inRect:(CGRect)rect immediate:(BOOL)imm inController:(UIViewController*)controller;

@property (nonatomic, retain) UIView* wrappedView;
@property (nonatomic, retain) MWParameter* parameter;

@end
