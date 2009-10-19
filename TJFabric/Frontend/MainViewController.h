#import <Cocoa/Cocoa.h>

@interface MainViewController : NSViewController {
	NSTextView* _logField;
	NSTextField* _fabricFileField;
	NSTask* _task;
	NSButton* _runningButton;
	NSButton* _stoppedButton;
	NSToolbarItem* _runItem, *_stopItem;
}

- (IBAction) chooseFabricFile: (id)sender;
- (IBAction) startServer: (id)sender;
- (IBAction) stopServer: (id)sender;

@property (nonatomic, retain) IBOutlet NSTextView* _logField;
@property (nonatomic, retain) IBOutlet NSTextField* _fabricFileField;
@property (nonatomic, retain) IBOutlet NSButton* _runningButton;
@property (nonatomic, retain) IBOutlet NSButton* _stoppedButton;
@property (nonatomic, retain) IBOutlet NSToolbarItem* _runItem;
@property (nonatomic, retain) IBOutlet NSToolbarItem* _stopItem;
@property (nonatomic, retain) NSTask* task;

@end
