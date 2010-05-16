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
#include <EP/include/ependpoint.h>
#include <EP/include/epdiscovery.h>
#include <TJNP/include/tjsocket.h>

using namespace tj::ep;
using namespace tj::shared;
using namespace tj::np;

@interface TTParametersDataSource: NSObject <NSTableViewDelegate, NSTableViewDataSource> {
	ref<EPMethod> _method;
	ref<EPRemoteState> _state;
}

- (void) setMethod: (ref<EPMethod>)method withRemoteState:(ref<EPRemoteState>)epr;

@end

@interface TTMethodViewController : NSViewController {
	NSTextField* _methodNameLabel;
	TTParametersDataSource* _parametersSource;
	NSTableView* _parametersTable;
	ref<EPEndpoint> _endpoint;
	ref<EPMethod> _method;
}

- (void) setMethod:(ref<EPMethod>)method endpoint:(ref<EPEndpoint>)ep connection:(ref<Connection>)c state:(ref<EPRemoteState>)rs;
- (IBAction) runMethod:(id)sender;

@property (nonatomic, retain) IBOutlet NSTextField* methodNameLabel;
@property (nonatomic, retain) IBOutlet TTParametersDataSource* parametersSource;
@property (nonatomic, retain) IBOutlet NSTableView* parametersTable;

@end
