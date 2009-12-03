#import <UIKit/UIKit.h>

@class MWInputAlertView;

@protocol MWInputDelegate
	- (void)inputAlert:(MWInputAlertView*)alert didClose:(NSString*)value withContext:(NSObject*)context;
@end

@interface MWInputAlertView : UIAlertView <UIAlertViewDelegate> {
	UITextField* _field;
	NSObject* _icontext;
	id<MWInputDelegate> _idelegate;
}

- (id) initWithTitle:(NSString*)title placeholder:(NSString*)placeholder value:(NSString*)value withContext:(NSObject*)context delegate:(id<MWInputDelegate>)d;

@property (nonatomic, retain) UITextField* field;

@end
