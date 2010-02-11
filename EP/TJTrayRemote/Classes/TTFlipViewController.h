#import <Cocoa/Cocoa.h>

@interface TTFlipViewController : NSViewController {
	NSView* _backView;
	NSView* _frontView;
	bool _isBack;
}

- (IBAction) flip:(id)sender;
- (void) flipToFront;

@property (nonatomic, retain) IBOutlet NSView* backView;
@property (nonatomic, retain) IBOutlet NSView* frontView;

@end
