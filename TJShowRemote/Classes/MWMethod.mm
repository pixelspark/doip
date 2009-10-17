#import "MWMethod.h"
#import "MWEndpoint.h"
#import "../../Libraries/TinyXML/tinyxml.h"

@implementation MWParameter
@synthesize friendlyName = _friendly;
@synthesize minimumValue = _min;
@synthesize defaultValue = _default;
@synthesize maximumValue = _max;
@synthesize type = _type;
@synthesize parent = _parent;
@synthesize value = _value;
@synthesize identifier = _id;

- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt {
	self.value = [NSNumber numberWithFloat:((UISlider*)slider).value];
	[self.parent execute];
};

- (void) switchValueChanged: (UISwitch*)sw event:(UIEvent*)evt {
	self.value = [NSNumber numberWithBool:((UISwitch*)sw).on];
	[self.parent execute];
}

- (UIView*) createView: (CGRect)rect {
	NSLog(@"createView type=%@", _type);
	if([_type isEqualToString:@"int32"]) {
		UISlider* slider = [[UISlider alloc] initWithFrame:rect];
		[slider setMinimumValue:[_min floatValue]];
		[slider setMaximumValue:[_max floatValue]];
		[slider setValue:[_default floatValue]];
		[slider addTarget:self action:@selector(sliderValueChanged:event:) forControlEvents:UIControlEventValueChanged];
		[slider autorelease];
		return slider;
	}
	else if([_type isEqualToString:@"bool"]) {
		UISwitch* sw = [[UISwitch alloc] initWithFrame:rect];
		[sw setOn:[_default boolValue]];
		[sw autorelease];
		[sw addTarget:self action:@selector(switchValueChanged:event:) forControlEvents:UIControlEventValueChanged];
		return sw;
	}
	return nil;
}

- (id) initFromDefinition:(TiXmlElement *)def inMethod:(MWMethod*)method {
	if(self = [super init]) {
		@try {
			self.parent = method;
			self.friendlyName = [NSString stringWithUTF8String:def->Attribute("friendly-name")];
			self.type = [NSString stringWithUTF8String:def->Attribute("type")];
			self.defaultValue = [NSString stringWithUTF8String:def->Attribute("default")];
			self.value = self.defaultValue;
			self.minimumValue = [NSString stringWithUTF8String:def->Attribute("min")];
			self.maximumValue = [NSString stringWithUTF8String:def->Attribute("max")];
			self.identifier = [NSString stringWithUTF8String:def->Attribute("id")];
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

- (void) setupCell:(UITableViewCell *)cell {
	cell.textLabel.text = [self friendlyName];
	cell.textLabel.textColor = [UIColor whiteColor];
	[[cell.contentView viewWithTag:1337] removeFromSuperview];
	
	// Add buttons, sliders, etc to the view
	if([_parameters count]==1) {
		MWParameter* firstParameter = [_parameters objectAtIndex:0];
		if(firstParameter!=nil) {
			CGRect rect = CGRectMake(180, 8, 130, 28);
			UIView* pv = [firstParameter createView:rect];
			if(pv!=nil) {
				pv.tag = 1337;
				[cell.contentView addSubview:pv];
			}
		}
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
