#import <UIKit/UIKit.h>

@interface MWMethod : NSObject {
	NSString* _pattern;
	NSString* _parameters;
	NSString* _friendly;
}

@property (nonatomic, retain) NSString* pattern;
@property (nonatomic, retain) NSString* parameters;
@property (nonatomic, retain) NSString* friendlyName;

- (id) initWithPattern: (NSString*)pattern parameters:(NSString*)ps friendlyName:(NSString*)fn;

@end
