#import <Cocoa/Cocoa.h>

@interface FrontendAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	NSTask* _task;
}

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic, retain) NSTask* task;

@end
