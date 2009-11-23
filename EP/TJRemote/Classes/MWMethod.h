#ifndef MW_METHOD_H
#define MW_METHOD_H
#import <UIKit/UIKit.h>

@class MWEndpoint;
@class MWMethod;
@class MWParameter;
class TiXmlElement;

@interface MWSliderView: UIView {
	UISlider* _slider;
	UILabel* _label;
	UIButton* _plusButton;
	UIButton* _minButton;
	MWParameter* _parameter;
}

- (id) initWithFrame:(CGRect)rect parameter:(MWParameter*)p immediate:(bool)imm;

@end;

@interface MWParameter: NSObject <UITextFieldDelegate> {
	NSString* _friendly;
	NSString* _id;
	NSString* _type;
	NSString* _min;
	NSString* _max;
	NSString* _default;
	NSString* _value;
	bool _discrete;
	MWMethod* _parent;
}

- (id) initFromDefinition:(TiXmlElement*)def inMethod:(MWMethod*)m;
- (UIView*) createView:(CGRect)rect immediate:(BOOL)imm;
- (void) textValueChanged: (UIView*)view event:(UIEvent*)evt;
- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt;
- (void) executeHandler: (UIView*)vw event:(UIEvent*)evt;
- (void) switchValueChanged: (UISwitch*)sw event:(UIEvent*)evt;

@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* minimumValue;
@property (nonatomic, retain) NSString* maximumValue;
@property (nonatomic, retain) NSString* defaultValue;
@property (nonatomic, retain) NSString* type;
@property (nonatomic, assign) MWMethod* parent;
@property (nonatomic, retain) NSString* value;
@property (nonatomic, retain) NSString* identifier;
@property (nonatomic, readonly) bool discrete;

@end

@interface MWMethod : NSObject {
	NSString* _pattern;
	NSString* _friendly;
	NSMutableArray* _parameters;
	MWEndpoint* _parent;
}

@property (nonatomic, retain) NSString* pattern;
@property (nonatomic, retain) NSMutableArray* parameters;
@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, assign) MWEndpoint* parent;

- (id) initWithPattern: (NSString*)pattern friendlyName:(NSString*)fn endpoint:(MWEndpoint*)e;
- (void) setupCell: (UITableViewCell*)cell;
- (bool) parametersFitInCell;
- (void) execute;

@end

#endif
