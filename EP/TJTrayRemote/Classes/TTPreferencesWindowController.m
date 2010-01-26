#import "TTPreferencesWindowController.h"

@implementation TTPreferencesWindowController

- (IBAction) okButtonClicked:(id)sender {
	[self.window close];
	[NSApp deactivate];
}

@end