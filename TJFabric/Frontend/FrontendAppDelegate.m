#import "FrontendAppDelegate.h"

@implementation FrontendAppDelegate

@synthesize window;
@synthesize task = _task;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
	if(_task!=nil) {
		[_task interrupt];
		[_task waitUntilExit];
	}
	return NSTerminateNow;
}

- (void) dealloc {
	if(_task!=nil) {
		[_task release];
	}
	[super dealloc];
}

@end
