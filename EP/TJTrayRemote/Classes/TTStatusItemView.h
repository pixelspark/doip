#import <Cocoa/Cocoa.h>
#import "../TJTrayRemoteAppDelegate.h"

@interface TTStatusItemView : NSView {
	NSImage* _statusImage;
	NSImage* _statusImageHigh;
	BOOL _clicked;
	TJTrayRemoteAppDelegate* _app;
}

@property (nonatomic, assign) TJTrayRemoteAppDelegate* app;

@end
