#import "TTMethodViewController.h"
#import "TJTrayRemoteAppDelegate.h"

@implementation TTMethodViewController
@synthesize methodNameLabel = _methodNameLabel;
@synthesize parametersSource = _parametersSource;
@synthesize parametersTable = _parametersTable;

- (void) setMethod:(ref<EPMethod>)method endpoint:(ref<EPEndpoint>)ep connection:(ref<Connection>)c state:(ref<EPRemoteState>)rs {
	[_methodNameLabel setTitleWithMnemonic:[NSString stringWithUTF8String:Mbs(method->GetFriendlyName()).c_str()]];
	[_parametersSource setMethod:method withRemoteState:rs];
	_endpoint = ep;
	_method = method;
}

- (IBAction) runMethod:(id)sender {
	TJTrayRemoteAppDelegate* app = (TJTrayRemoteAppDelegate*)[[NSApplication sharedApplication] delegate];
	[app executeMethod:_method onEndpoint:_endpoint];
}

- (void) dealloc {
	[_parametersTable release];
	[_parametersSource release];
	[_methodNameLabel release];
	[super dealloc];
}

@end

@implementation TTParametersDataSource

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row {
	return NO;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	if(_method) {
		std::vector< ref<EPParameter> > params;
		_method->GetParameters(params);
		return params.size();
	}
	return 0;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
	return 18.0f;
}

- (void) setMethod:(ref<EPMethod>)ep withRemoteState:(ref<EPRemoteState>)state {
	_method = ep;
	_state = state;
}

- (ref<EPParameter>) parameterWithIndex:(int)idx {
	if(_method) {
		std::vector< ref<EPParameter> > params;
		_method->GetParameters(params);
		return params.at(idx);
	}
	return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)column row:(NSInteger)idx {
	ref<EPParameter> enp = [self parameterWithIndex:idx];
	if(enp) {
		if([[column identifier] isEqualToString:@"control"]) {
			std::wstring type = enp->GetType();
			if(type==EPParameter::KTypeDouble || type==EPParameter::KTypeInt32) {
				NSNumber* num = anObject;
				enp->SetDefaultValue(Any([num doubleValue]));
			}
			else if(type==EPParameter::KTypeBoolean) {
				NSNumber* num = anObject;
				enp->SetDefaultValue(Any([num boolValue]));
			}
		}
	}
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	ref<EPParameter> enp = [self parameterWithIndex:row];
	if(enp) {
		if([[tableColumn identifier] isEqualToString:@"name"]) {
			return [NSString stringWithUTF8String:Mbs(enp->GetFriendlyName()).c_str()];
		}
		else if([[tableColumn identifier] isEqualToString:@"control"]) {
			std::wstring type = enp->GetType();
			Any value = enp->GetDefaultValue();
			
			if(type==EPParameter::KTypeDouble || type==EPParameter::KTypeInt32) {
				return [NSNumber numberWithDouble:double(value)];
			}
			else if(type==EPParameter::KTypeBoolean) {
				return [NSNumber numberWithBool:bool(value)];
			}
		}
		else {
			if(enp->GetNature()==EPParameter::NatureDiscrete) {
				Any value = enp->GetDefaultValue();
				return [NSString stringWithUTF8String:Mbs(value.ToString()).c_str()];
			}
		}
	}
	return nil;
}

- (NSCell*) createCellForParameter: (ref<EPParameter>)ep {
	std::wstring type = ep->GetType();
	
	if(type==EPParameter::KTypeDouble || type==EPParameter::KTypeInt32) {
		NSSliderCell* slider = [[NSSliderCell alloc] initTextCell:@""];
		[slider setMinValue:(double)ep->GetMinimumValue()];
		[slider setMaxValue:(double)ep->GetMaximumValue()];
		return slider;
	}
	else if(type==EPParameter::KTypeBoolean) {
		NSButtonCell* button = [[NSButtonCell alloc] initTextCell:@""];
		[button setButtonType:NSSwitchButton];
		return button;
	}
	
	return nil;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	if(tableColumn==nil) {
		return nil; // If we return something when called with a nil column, the cell spans all columns
	}
	
	ref<EPParameter> ep = [self parameterWithIndex:row];
	if(ep) {	
		if([[tableColumn identifier] isEqualToString:@"name"]) {
			NSCell* cell = [[NSCell alloc] initTextCell:[NSString stringWithUTF8String:Mbs(ep->GetFriendlyName()).c_str()]];
			[cell setFont:[NSFont boldSystemFontOfSize:12.0f]];
			return cell;
		}
		else if([[tableColumn identifier] isEqualToString:@"control"]) {
			return [self createCellForParameter:ep];
		}
		else {
			if(ep->GetNature()==EPParameter::NatureDiscrete) {
				NSCell* cell = [[NSCell alloc] initTextCell:@""];
				[cell setFont:[NSFont systemFontOfSize:12.0f]];
				return cell;
			}
		}
	}
	return nil;
}

- (void)dealloc {
	[super dealloc];
}

@end
