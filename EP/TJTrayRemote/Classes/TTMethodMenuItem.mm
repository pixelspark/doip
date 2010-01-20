#import "TTMethodMenuItem.h"
#import "TTParameterMenuItem.h"

@implementation TTMethodMenuItem

- (void) dealloc {
	[_parameterItems release];
	[super dealloc];
}

- (void) execute: (id)sender {
	std::set< EPPath > paths;
	_method->GetPaths(paths);
	if(paths.size()>0) {
		EPPath path = *(paths.begin());
		ref<Message> message = GC::Hold(new Message(path));
		std::vector< ref<EPParameter> > parameters;
		_method->GetParameters(parameters);
		unsigned int i = 0;
		std::vector< ref<EPParameter> >::iterator it = parameters.begin();
		while(it!=parameters.end()) {
			ref<EPParameter> epp = *it;
			if(epp) {
				message->SetParameter(i, epp->GetDefaultValue());
			}
			++i;
			++it;
		}
		
		_connection->Send(message);
	}
}

- (void) update:(ref<EPRemoteState>)epr {
	for(TTParameterMenuItem* tpi in _parameterItems) {
		[tpi update:epr onlyState:YES];
	}
}

- (id) initWithMethod:(ref<EPMethod>)m endpoint:(ref<EPEndpoint>)ep connection:(ref<Connection>)c state:(ref<EPRemoteState>)rs {
	_parameterItems = [[NSMutableArray alloc] init];
	std::string methodName = Mbs(m->GetFriendlyName());
	if(self = [super initWithTitle:[NSString stringWithUTF8String:methodName.c_str()] action:nil keyEquivalent:@""]) {
		_method = m;
		_endpoint = ep;
		_connection = c;
		
		std::string methodDescription = Mbs(m->GetDescription());
		[self setToolTip:[NSString stringWithUTF8String:methodDescription.c_str()]];
		
		/* Add all parameters */
		std::vector< ref<EPParameter> > parameters;
		_method->GetParameters(parameters);
		if(parameters.size()>0) {
			NSMenu* parameterMenu = [[NSMenu alloc] initWithTitle:@""];
			std::vector< ref<EPParameter> >::iterator it = parameters.begin();
			while(it!=parameters.end()) {
				ref<EPParameter> epp = *it;
				if(epp) {
					NSMenuItem* parameterItem = [[TTParameterMenuItem alloc] initWithParameter:epp state:rs];
					[parameterMenu addItem:parameterItem];
					[_parameterItems addObject:parameterItem];
					[parameterItem release];
				}
				++it;
			}
			[self setSubmenu:parameterMenu];
			
			// Add run button(s)
			NSMenuItem* buttonItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
			NSView* wrapper = [[NSView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0,0,180,24))];
			NSButton* runButton = [[NSButton alloc] initWithFrame:NSRectFromCGRect(CGRectMake(10,0,160,24))];
			[runButton setTitle:@"Run right now"];
			[runButton setButtonType:NSMomentaryPushButton];
			[runButton setBezelStyle:NSRoundRectBezelStyle];
			[runButton setTarget:self];
			[runButton setAction:@selector(execute:)];
			[wrapper addSubview:runButton];
			[buttonItem setView:wrapper];
			[parameterMenu addItem:buttonItem];
			
			[parameterMenu release];
		}
	}
	
	return self;
}

- (ref<EPEndpoint>) endpoint {
	return _endpoint;
}

- (ref<EPMethod>) method {
	return _method;
}

@end
