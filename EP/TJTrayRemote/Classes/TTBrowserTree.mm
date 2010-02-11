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
		TJTrayRemoteAppDelegate* app = (TJTrayRemoteAppDelegate*)[[NSApplication sharedApplication] delegate];
		[_methodsView setMethod:method endpoint:enp connection:[app discovery]->GetConnectionForEndpoint(enp) state:[app discovery]->GetStateForEndpoint(enp)];
		[_flipper flip:nil];
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
		NSCell* cell = [[NSCell alloc] initTextCell:[NSString stringWithUTF8String:Mbs(ep->GetFriendlyName()).c_str()]];
		[cell setFont:[NSFont systemFontOfSize:12.0f]];
		return cell;
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
	std::set< ref<EPEndpoint> >::iterator it = [_app discovery]->_endpoints.begin();
	for(int a=0;a<idx;a++) {
		if(it==[_app discovery]->_shownEndpoints.end()) {
			return nil;
		}
		++it;
	}
	
	return *it;
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
