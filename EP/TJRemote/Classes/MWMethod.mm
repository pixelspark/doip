#import "MWMethod.h"
#import "MWEndpoint.h"
#import "MWSliderView.h"
#import "MWFavorite.h"
#import "../../../Libraries/TinyXML/tinyxml.h"

@implementation MWParameter
@synthesize friendlyName = _friendly;
@synthesize minimumValue = _min;
@synthesize defaultValue = _default;
@synthesize maximumValue = _max;
@synthesize type = _type;
@synthesize parent = _parent;
@synthesize value = _value;
@synthesize identifier = _id;
@synthesize nature = _nature;
@dynamic discrete;

- (id) initWithCoder:(NSCoder *)aDecoder {
	if(self = [super init]) {
		self.friendlyName = [aDecoder decodeObjectForKey:@"FriendlyName"];
		self.minimumValue = [aDecoder decodeObjectForKey:@"MinValue"];
		self.maximumValue = [aDecoder decodeObjectForKey:@"MaxValue"];
		self.type = [aDecoder decodeObjectForKey:@"Type"];
		self.value = [aDecoder decodeObjectForKey:@"Value"];
		self.identifier = [aDecoder decodeObjectForKey:@"ID"];
		self.nature = [aDecoder decodeObjectForKey:@"Nature"];
	}
	return self;
}

- (void) encodeWithCoder:(NSCoder *)aCoder {
	[aCoder encodeObject:self.friendlyName forKey:@"FriendlyName"];
	[aCoder encodeObject:self.minimumValue forKey:@"MinValue"];
	[aCoder encodeObject:self.maximumValue forKey:@"MaxValue"];
	[aCoder encodeObject:self.type forKey:@"Type"];
	[aCoder encodeObject:self.value forKey:@"Value"];
	[aCoder encodeObject:self.identifier forKey:@"ID"];
	[aCoder encodeObject:self.nature forKey:@"Nature"];
}

- (MWParameter*) clone {
	MWParameter* np = [[MWParameter alloc] init];
	np.friendlyName = self.friendlyName;
	np.minimumValue = self.minimumValue;
	np.maximumValue = self.maximumValue;
	np.type = self.type;
	np.value = self.value;
	np.identifier = self.identifier;
	np.nature = self.nature;
	return [np autorelease];
}

- (bool) discrete {
	return [_nature isEqualToString:@"discrete"];
}

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
		UISwitch* sw = [[UISwitch alloc] initWithFrame:CGRectMake(rect.size.width+rect.origin.x-96, rect.origin.y, 96, rect.size.height)];
		[sw setOn:[_default isEqualToString:@"yes"] || [_default isEqualToString:@"1"]];
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
			self.nature = [self attribute:"nature" fromElement:def defaultsTo:@""];
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
	[_nature release];
	[_value release];
	[super dealloc];
}

@end

@implementation MWMethod

static NSMutableDictionary* _icons;

@synthesize pattern = _pattern;
@synthesize parameters = _parameters;
@synthesize friendlyName = _friendly;
@synthesize parent = _parent;
@synthesize friendlyDescription = _description;

+ (void) initialize {
	NSDictionary* icons = [NSDictionary dictionaryWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ep-icons" ofType:@"plist"]];
	_icons = [[NSMutableDictionary alloc] initWithCapacity:[icons count]];
	for(NSString* key in [icons keyEnumerator]) {
		NSString* path = [icons objectForKey:key];
		NSString* pngPath = [[NSBundle mainBundle] pathForResource:path ofType:@"png"];
		UIImage* icon = [UIImage imageWithContentsOfFile:pngPath];
		if(icon!=nil) {
			[_icons setObject:icon forKey:key];
		}
		else {
			NSLog(@"Could not load icon with key %@ path: %@ realpath: %@", key, path, pngPath);
		}
	}
}

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

- (MWFavorite*) createFavorite {
	NSMutableArray* args = [[NSMutableArray alloc] initWithCapacity:[_parameters count]];
	for (MWParameter* p in _parameters) {
		[args addObject:[p clone]];
	}
	
	MWFavorite* fav = [[MWFavorite alloc] initWithPath:_pattern andArguments:args];
	fav.friendlyName = self.friendlyName;
	[args release];
	return [fav autorelease];
}

- (void) setupCell:(UITableViewCell *)cell {
	cell.detailTextLabel.text = [self friendlyDescription];
	cell.textLabel.text = [self friendlyName];
	cell.textLabel.textColor = [UIColor whiteColor];
	[[cell.contentView viewWithTag:1337] removeFromSuperview];
	
	if([_icons objectForKey:self.pattern]!=nil) {
		cell.imageView.image = [_icons objectForKey:self.pattern];
	}
	else {
		cell.imageView.image = nil;
	}
	
	// Add buttons, sliders, etc to the view
	if([_parameters count]==0) {
		// Do nothing
		cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
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
		cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
	}
	else {
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
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
