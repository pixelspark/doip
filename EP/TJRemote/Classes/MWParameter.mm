#import "MWParameter.h"
#import "MWSliderView.h"
#import "MWOptionView.h"
#import "MWMethod.h"
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
@synthesize options = _options;
@dynamic discrete;

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
			self.options = [[NSMutableDictionary alloc] init];
			self.parent = method;
			self.friendlyName = [self attribute:"friendly-name" fromElement:def defaultsTo:@""];
			self.type = [self attribute:"type" fromElement:def defaultsTo:@""];
			self.defaultValue = [self attribute:"default" fromElement:def defaultsTo:@""];
			self.value = self.defaultValue;
			self.minimumValue = [self attribute:"min" fromElement:def defaultsTo:@""];
			self.maximumValue = [self attribute:"max" fromElement:def defaultsTo:@""];
			self.identifier = [self attribute:"id" fromElement:def defaultsTo:@""];
			self.nature = [self attribute:"nature" fromElement:def defaultsTo:@""];
			
			TiXmlElement* option = def->FirstChildElement("option");
			while(option!=0) {
				NSString* name = [self attribute:"name" fromElement:option defaultsTo:@""];
				NSString* value = [self attribute:"value" fromElement:option defaultsTo:@""];
				[_options setObject:value forKey:name];
				option = option->NextSiblingElement("option");
			}
		}
		@catch (NSException * e) {
			NSLog(@"Invalid parameter specification, an attribute is probably missing; error was %@", [e reason]);
		}
	}
	return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
	if(self = [super init]) {
		self.options = [[NSMutableDictionary alloc] init];
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

- (UIView*) createView: (CGRect)rect immediate:(BOOL)imm inController:(UIViewController*)cs {
	if([_options count]>0) {
		// The values are restricted; create an MWOptionsView
		MWOptionView* mwo = [[MWOptionView alloc] initWithFrame:rect parameter:self immediate:imm];
		mwo.parentViewController = cs;
		[mwo autorelease];
		return mwo;
	}
	else {
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
	}
	return nil;
}

- (void) dealloc {
	[_friendly release];
	[_min release];
	[_max release];
	[_id release];
	[_type release];
	[_nature release];
	[_value release];
	[_options release];
	[super dealloc];
}

@end