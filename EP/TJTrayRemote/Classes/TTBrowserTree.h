/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
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