#import "MWMethod.h"
#import "MWEndpoint.h"
#import "../../../Libraries/TinyXML/tinyxml.h"

@implementation MWSliderView

- (void) updateLabel {
	if(_parameter.discrete) {
		[_label setText:[NSString stringWithFormat:@"%d",[[_parameter value] intValue]]];
	}
	else {
		[_label setText:[NSString stringWithFormat:@"%0.2f",[[_parameter value] floatValue]]];
	}
};

- (id) initWithFrame:(CGRect)rect parameter:(MWParameter*)parameter immediate:(bool)imm {
	const static int KLabelWidth = 64;
	
	if(self = [super initWithFrame:rect]) {
		_parameter = parameter;
		[_parameter retain];
		_slider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, rect.size.width-KLabelWidth, rect.size.height)];
		[_slider setMinimumValue:[[parameter minimumValue] floatValue]];
		[_slider setMaximumValue:[[parameter maximumValue] floatValue]];
		[_slider setValue:[[parameter value] floatValue]];
		[_slider addTarget:self action:@selector(sliderValueChanged:event:) forControlEvents:UIControlEventValueChanged];
		if(imm) {
			[_slider addTarget:parameter action:@selector(executeHandler:event:) forControlEvents:UIControlEventValueChanged];
		}
		[_slider setAutoresizingMask:UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
		[self addSubview:_slider];
		
		// Add label
		_label = [[UILabel alloc] initWithFrame:CGRectMake(rect.size.width-KLabelWidth, 0, KLabelWidth, rect.size.height)];
		[_label setBackgroundColor:[UIColor clearColor]];
		[_label setOpaque:FALSE];
		[_label setTextColor:[UIColor whiteColor]];
		[_label setTextAlignment:UITextAlignmentRight];
		[_label setAutoresizingMask:UIViewAutoresizingFlexibleLeftMargin];
		[self updateLabel];
		[self addSubview:_label];
	}
	return self;
}
- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt {
	[_parameter sliderValueChanged:slider event:evt];
	[self updateLabel];
};


- (void) dealloc {
	[_parameter release];
	[_slider release];
	[_label release];
	[super dealloc];
}

@end


@implementation MWParameter
@synthesize friendlyName = _friendly;
@synthesize minimumValue = _min;
@synthesize defaultValue = _default;
@synthesize maximumValue = _max;
@synthesize type = _type;
@synthesize parent = _parent;
@synthesize value = _value;
@synthesize identifier = _id;
@synthesize discrete = _discrete;

- (void) textValueChanged: (UIView*)view event:(UIEvent*)evt {
	self.value = [(UITextField*)view text];
}

- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt {
	self.value = [[NSNumber numberWithFloat:((UISlider*)slider).value] stringValue];
};

- (void) executeHandler: (UIView*)vw event:(UIEvent*)evt {
	[self.parent execute];
}

- (void) switchValueChanged: (UISwitch*)sw event:(UIEvent*)evt {
	self.value = [[NSNumber numberWithBool:((UISwitch*)sw).on] stringValue];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder];
	return YES;
}

- (UIView*) createView: (CGRect)rect immediate:(BOOL)imm {
	if([_type isEqualToString:@"int32"] || [_type isEqualToString:@"double"]) {
		MWSliderView* sv = [[MWSliderView alloc] initWithFrame:rect parameter:self immediate:imm];
		[sv autorelease];
		return sv;
	}
	else if([_type isEqualToString:@"bool"]) {
		UISwitch* sw = [[UISwitch alloc] initWithFrame:rect];
		[sw setOn:[_default boolValue]];
		[sw autorelease];
		[sw addTarget:self action:@selector(switchValueChanged:event:) forControlEvents:UIControlEventValueChanged];
		if(imm) {
			[sw addTarget:self action:@selector(executeHandler:event:) forControlEvents:UIControlEventValueChanged];
		}
		return sw;
	}
	else if([_type isEqualToString:@"string"]) {
		UITextField* field = [[UITextField alloc] initWithFrame:rect];
		[field setBorderStyle:UITextBorderStyleRoundedRect];
		[field setDelegate:self];
		if(imm) {
			[field addTarget:self action:@selector(executeHandler:event:) forControlEvents:UIControlEventEditingDidEnd];
		}
		[field autorelease];
		return field;
	}
	return nil;
}

- (NSString*) attribute:(const char*)name fromElement:(TiXmlElement*)elm defaultsTo:(NSString*)def {
	const char* value = elm->Attribute(name);
	if(value==0) {
		return def;
	}
	return [NSString stringWithUTF8String:value];
}

- (id) initFromDefinition:(TiXmlElement *)def inMethod:(MWMethod*)method {
	if(self = [super init]) {
		@try {
			self.parent = method;
			self.friendlyName = [self attribute:"friendly-name" fromElement:def defaultsTo:@""];
			self.type = [self attribute:"type" fromElement:def defaultsTo:@""];
			self.defaultValue = [self attribute:"default" fromElement:def defaultsTo:@""];
			self.value = self.defaultValue;
			self.minimumValue = [self attribute:"min" fromElement:def defaultsTo:@""];
			self.maximumValue = [self attribute:"max" fromElement:def defaultsTo:@""];
			self.identifier = [self attribute:"id" fromElement:def defaultsTo:@""];
			_discrete = false;
			if(def->Attribute("discrete")!=0) {
				_discrete = [[NSString stringWithUTF8String:def->Attribute("discrete")] isEqualToString:@"yes"];
				NSLog(@"Method is discrete? %d", _discrete);
			}
		}
		@catch (NSException * e) {
			NSLog(@"Invalid parameter specification, an attribute is probably missing; error was %@", [e reason]);
		}
	}
	return self;
}

- (void) dealloc {
	[_friendly release];
	[_min release];
	[_max release];
	[_id release];
	[_type release];
	[_value release];
	[super dealloc];
}

@end


@implementation MWMethod

@synthesize pattern = _pattern;
@synthesize parameters = _parameters;
@synthesize friendlyName = _friendly;
@synthesize parent = _parent;

- (id) initWithPattern:(NSString*)pattern friendlyName:(NSString*)fn endpoint:(MWEndpoint*)endpoint {
	if(self = [super init]) {
		self.parent = endpoint;
		self.pattern = pattern;
		self.parameters = [[NSMutableArray alloc] init];
		self.friendlyName = fn;
	}
	return self;
}

- (void) execute {
	[self.parent executeMethod:self];
}

- (bool) parametersFitInCell {
	return [_parameters count]==1;
}

- (void) setupCell:(UITableViewCell *)cell {
	cell.textLabel.text = [self friendlyName];
	cell.textLabel.textColor = [UIColor whiteColor];
	[[cell.contentView viewWithTag:1337] removeFromSuperview];
	
	// Add buttons, sliders, etc to the view
	if([_parameters count]==0) {
		// Do nothing
	}
	else if([self parametersFitInCell]) {
		MWParameter* firstParameter = [_parameters objectAtIndex:0];
		if(firstParameter!=nil) {
			CGRect rect = CGRectMake(160, 8, 150, 28);
			UIView* pv = [firstParameter createView:rect immediate:YES];
			if(pv!=nil) {
				pv.tag = 1337;
				[cell.contentView addSubview:pv];
			}
		}
	}
	else {
		[cell setAccessoryType:UITableViewCellAccessoryDetailDisclosureButton];
	}
}

- (void) dealloc {
	[_pattern release];
	[_parameters release];
	[_friendly release];
	[_parameters release];
	[super dealloc];
}

@end
