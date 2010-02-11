#import <Cocoa/Cocoa.h>
#import "TTDiscovery.h"

using namespace tj::shared;
using namespace tj::ep;

@class TTFlipViewController;
@class TJTrayRemoteAppDelegate;
@class TTMethodViewController;

@interface TTEndpointsDataSource: NSObject <NSTableViewDelegate, NSTableViewDataSource> {
	TJTrayRemoteAppDelegate* _app;
}

- (ref<EPEndpoint>) endpointWithIndex:(int)idx;

@property (nonatomic, assign) IBOutlet TJTrayRemoteAppDelegate* app;

@end

@interface TTMethodsDataSource: NSObject <NSTableViewDelegate, NSTableViewDataSource> {
	ref<EPEndpoint> _endpoint;
}

- (ref<EPMethod>) methodWithIndex:(int)idx;
- (void) setEndpoint: (ref<EPEndpoint>)ep;

@end

@interface TTBrowserController: NSWindowController {
	NSTableView* _endpointsTable;
	NSTableView* _methodsTable;
	TTMethodViewController* _methodsView;
	TTEndpointsDataSource* _endpointsSource;
	TTMethodsDataSource* _methodsSource;
	TTFlipViewController* _flipper;
	NSView* _parametersView;
	NSView* _parametersWindow;
	NSButton* _developerButton;
}

- (IBAction) endpointSelected:(id)sender;
- (IBAction) methodSelected:(id)sender;
- (IBAction) developerButtonClicked:(id)sender;

@property (nonatomic, retain) IBOutlet NSTableView* endpointsTable;
@property (nonatomic, retain) IBOutlet NSTableView* methodsTable;
@property (nonatomic, retain) IBOutlet TTEndpointsDataSource* endpointsSource;
@property (nonatomic, retain) IBOutlet TTMethodsDataSource* methodsSource;
@property (nonatomic, retain) IBOutlet NSView* parametersView;
@property (nonatomic, retain) IBOutlet TTMethodViewController* methodsView;
@property (nonatomic, retain) IBOutlet TTFlipViewController* flipper;
@property (nonatomic, retain) IBOutlet NSButton* developerButton;

@end