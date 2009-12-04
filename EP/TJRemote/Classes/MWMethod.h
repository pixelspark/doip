#ifndef MW_METHOD_H
#define MW_METHOD_H
#import <UIKit/UIKit.h>
#import "MWParameter.h"

@class MWEndpoint;
@class MWMethod;
@class MWFavorite;
@class MWSliderView;
class TiXmlElement;

@interface MWMethod : NSObject {
	NSString* _pattern;
	NSString* _friendly;
	NSString* _description;
	NSMutableArray* _parameters;
	MWEndpoint* _parent;
}

@property (nonatomic, retain) NSString* pattern;
@property (nonatomic, retain) NSMutableArray* parameters;
@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* friendlyDescription;
@property (nonatomic, assign) MWEndpoint* parent;

- (id) initWithPattern: (NSString*)pattern friendlyName:(NSString*)fn endpoint:(MWEndpoint*)e;
- (void) setupCell: (UITableViewCell*)cell inController:(UIViewController*)cs;
- (bool) parametersFitInCell;
- (void) execute;
- (MWFavorite*) createFavorite;

@end

#endif
