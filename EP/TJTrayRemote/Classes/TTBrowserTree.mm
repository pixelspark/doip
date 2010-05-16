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
 
 #import "TTBrowserTree.h"
#import "TJTrayRemoteAppDelegate.h"
#import "TTFlipViewController.h"
#import "TTMethodViewController.h"

@implementation TTBrowserController
@synthesize methodsTable = _methodsTable;
@synthesize endpointsTable = _endpointsTable;
@synthesize methodsSource = _methodsSource;
@synthesize endpointsSource = _endpointsSource;
@synthesize parametersView = _parametersView;
@synthesize flipper = _flipper;
@synthesize methodsView = _methodsView;
@synthesize developerButton = _developerButton;

- (IBAction) endpointSelected:(id)sender {
	// Find out with endpoint was selected
	ref<EPEndpoint> enp = [_endpointsSource endpointWithIndex:[_endpointsTable selectedRow]];
	if(enp) {
		[_methodsSource setEndpoint:enp];
		[_methodsTable reloadData];
	}
	else {
		[_methodsSource setEndpoint:null];
		[_methodsTable reloadData];
	}
}

- (IBAction) developerButtonClicked:(id)sender {
	ref<EPEndpoint> enp = [_endpointsSource endpointWithIndex:[_endpointsTable selectedRow]];
	if(enp) {
		TJTrayRemoteAppDelegate* app = (TJTrayRemoteAppDelegate*)[[NSApplication sharedApplication] delegate];
		ref<Service> svc = [app discovery]->GetServiceForEndpoint(enp);
		std::wstring defPath;
	
		if(svc && svc->GetAttribute(L"EPDefinitionPath", defPath)) {
			NSString* url = [NSString stringWithFormat:@"http://%s:%d%s",Mbs(svc->GetHostName()).c_str(), svc->GetPort(), Mbs(defPath).c_str(), nil];
			[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
		}
	}
}

- (IBAction) methodSelected: (id)sender {
	ref<EPMethod> method = [_methodsSource methodWithIndex:[_methodsTable selectedRow]];
	ref<EPEndpoint> enp = [_endpointsSource endpointWithIndex:[_endpointsTable selectedRow]];
	if(method && enp) {
		std::vector< ref<EPParameter> > params;
		method->GetParameters(params);
		
		TJTrayRemoteAppDelegate* app = (TJTrayRemoteAppDelegate*)[[NSApplication sharedApplication] delegate];
		
		if(params.size()>0) {
			[_methodsView setMethod:method endpoint:enp connection:[app discovery]->GetConnectionForEndpoint(enp) state:[app discovery]->GetStateForEndpoint(enp)];
			[_flipper flip:nil];
		}
		else {
			[app executeMethod:method onEndpoint:enp];
		}
	}
}

- (void)dealloc {
	[_methodsTable release];
	[_endpointsTable release];
	[_parametersView release];
	[_flipper release];
	[_methodsView release];
	[_developerButton release];
	[_parametersWindow release];
	[super dealloc];
}

@end

@implementation TTMethodsDataSource

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row {
	return NO;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	if(_endpoint) {
		std::vector< ref<EPMethod> > methods;
		_endpoint->GetMethods(methods);
		return methods.size();
	}
	return 0;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
	return 18.0f;
}

- (void) setEndpoint:(ref<EPEndpoint>)ep {
	_endpoint = ep;
}

- (ref<EPMethod>) methodWithIndex:(int)idx {
	if(_endpoint) {
		std::vector< ref<EPMethod> > methods;
		_endpoint->GetMethods(methods);
		if(idx<methods.size()) {
			return methods.at(idx);
		}
		return nil;
	}
	return nil;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	ref<EPMethod> enp = [self methodWithIndex:row];
	if(enp) {
		return [NSString stringWithUTF8String:Mbs(enp->GetFriendlyName()).c_str()];
	}
	return nil;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	ref<EPMethod> ep = [self methodWithIndex:row];
	if(ep) {
		std::vector< ref<EPParameter> > params;
		ep->GetParameters(params);
		if(params.size()==0) {
			NSActionCell* nsa = [[NSActionCell alloc] initTextCell:[NSString stringWithUTF8String:Mbs(ep->GetFriendlyName()).c_str()]];
			[nsa setFont:[NSFont boldSystemFontOfSize:12.0f]];
			return nsa;
		}
		else {
			NSCell* cell = [[NSCell alloc] initTextCell:[NSString stringWithUTF8String:Mbs(ep->GetFriendlyName()).c_str()]];
			[cell setFont:[NSFont systemFontOfSize:12.0f]];
			return cell;
		}
		
	}
	return nil;
}

- (void)dealloc {
	[super dealloc];
}

@end


@implementation TTEndpointsDataSource
@synthesize app = _app;

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row {
	return NO;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	int r = [_app discovery]->_shownEndpoints.size();
	return r;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
	return 18.0f;
}

- (ref<EPEndpoint>) endpointWithIndex:(int)idx {
	ref<TTDiscovery> disco = [_app discovery];
	ThreadLock lock(&(disco->_lock));
	
	std::set< ref<EPEndpoint> >::iterator it = disco->_shownEndpoints.begin();
	for(int a=0;a<idx;a++) {
		if(it==disco->_shownEndpoints.end()) {
			return ref<Endpoint>(0);
		}
		++it;
	}
	
	if(it!=disco->_shownEndpoints.end()) {
		return *it;
	}
	return ref<Endpoint>(0);
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	ref<EPEndpoint> enp = [self endpointWithIndex:row];
	if(enp) {
		return [NSString stringWithUTF8String:Mbs(enp->GetFriendlyName()).c_str()];
	}
	return nil;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	ref<EPEndpoint> ep = [self endpointWithIndex:row];
	if(ep) {
		NSCell* cell = [[NSCell alloc] initTextCell:[NSString stringWithUTF8String:Mbs(ep->GetFriendlyName()).c_str()]];
		[cell setFont:[NSFont boldSystemFontOfSize:12.0f]];
		return cell;
	}
	return nil;
}

- (void)dealloc {
	[super dealloc];
}

@end
