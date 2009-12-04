#import <UIKit/UIKit.h>

@class MWMethod;
class TiXmlElement;

@interface MWParameter: NSObject <UITextFieldDelegate, NSCoding> {
	NSString* _friendly;
	NSString* _id;
	NSString* _type;
	NSString* _min;
	NSString* _max;
	NSString* _default;
	NSString* _value;
	NSString* _nature;
	MWMethod* _parent;
	NSMutableDictionary* _options;
}

- (id) initFromDefinition:(TiXmlElement*)def inMethod:(MWMethod*)m;
- (UIView*) createView:(CGRect)rect immediate:(BOOL)imm inController:(UIViewController*)c;
- (void) textValueChanged: (UIView*)view event:(UIEvent*)evt;
- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt;
- (void) executeHandler: (UIView*)vw event:(UIEvent*)evt;
- (void) switchValueChanged: (UISwitch*)sw event:(UIEvent*)evt;
- (MWParameter*) clone;

- (id) initWithCoder:(NSCoder *)aDecoder;
- (void) encodeWithCoder:(NSCoder *)aCoder;

@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* minimumValue;
@property (nonatomic, retain) NSString* maximumValue;
@property (nonatomic, retain) NSString* defaultValue;
@property (nonatomic, retain) NSString* type;
@property (nonatomic, assign) MWMethod* parent;
@property (nonatomic, retain) NSString* value;
@property (nonatomic, retain) NSString* identifier;
@property (nonatomic, readonly) bool discrete;
@property (nonatomic, retain) NSString* nature;
@property (nonatomic, retain) NSDictionary* options;

@end
