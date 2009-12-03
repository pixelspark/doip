#import <UIKit/UIKit.h>
@class MWParameter;

@interface MWSliderView: UIView {
	UISlider* _slider;
	UILabel* _label;
	UIButton* _plusButton;
	UIButton* _minButton;
	MWParameter* _parameter;
}

- (id) initWithFrame:(CGRect)rect parameter:(MWParameter*)p immediate:(bool)imm;

@end
