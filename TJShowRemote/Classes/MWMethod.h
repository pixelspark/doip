#ifndef MW_METHOD_H
#define MW_METHOD_H
#import <UIKit/UIKit.h>

@class MWEndpoint;
@class MWMethod;
class TiXmlElement;

@interface MWParameter: NSObject {
	NSString* _friendly;
	NSString* _id;
	NSString* _type;
	NSString* _min;
	NSString* _max;
	NSString* _default;
	NSObject* _value;
	MWMethod* _parent;
}

- (id) initFromDefinition:(TiXmlElement*)def inMethod:(MWMethod*)m;
- (UIView*) createView:(CGRect)rect;

@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* minimumValue;
@property (nonatomic, retain) NSString* maximumValue;
@property (nonatomic, retain) NSString* defaultValue;
@property (nonatomic, retain) NSString* type;
@property (nonatomic, assign) MWMethod* parent;
@property (nonatomic, retain) NSObject* value;
@property (nonatomic, retain) NSString* identifier;

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
- (void) execute;

@end

#endif
