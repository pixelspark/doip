#import <Cocoa/Cocoa.h>
#import "FrontendAppDelegate.h"

@interface MainViewController : NSViewController {
	NSTextView* _logField;
	NSArrayController* _fabrics;
	NSButton* _runningButton;
	NSButton* _stoppedButton;
	NSButton* _addFabricButton;
	NSToolbarItem* _runItem, *_stopItem;
	NSTextField* _statusLabel;
	FrontendAppDelegate* _app;
	NSLock* _outLock;
	
}

- (IBAction) chooseFabricFile: (id)sender;
- (IBAction) startServer: (id)sender;
- (IBAction) stopServer: (id)sender;
- (IBAction) clearLog: (id)sender;

@property (nonatomic, retain) IBOutlet NSTextView* _logField;
@property (nonatomic, retain) IBOutlet NSButton* _addFabricButton;
@property (nonatomic, retain) IBOutlet NSButton* _runningButton;
@property (nonatomic, retain) IBOutlet NSButton* _stoppedButton;
@property (nonatomic, retain) IBOutlet NSToolbarItem* _runItem;
@property (nonatomic, retain) IBOutlet NSToolbarItem* _stopItem;
@property (nonatomic, retain) IBOutlet NSTextField* _statusLabel;
@property (nonatomic, retain) IBOutlet NSArrayController* _fabrics;
@property (nonatomic, assign) IBOutlet FrontendAppDelegate* _app;

@end
