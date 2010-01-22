#import "MWParameter.h"
#import "MWParameterView.h"
#import "MWMethod.h"
#import "MWEndpoint.h"
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
@synthesize bindValue = _bindValue;
@dynamic discrete;

- (NSString*) attribute:(const char*)name fromElement:(TiXmlElement*)elm defaultsTo:(NSString*)def {
	const char* value = elm->Attribute(name);
	if(value==0) {
		return def;
	}
	return [NSString stringWithUTF8String:value];
}

- (void) boundStateChanged:(NSNotification*)ns {
	MWStateChange* change = [ns object];
	if(change!=nil) {
		if([_bindValue isEqualToString:[change key]]) {
			self.value = [change value];
			NSNotification* nt = [NSNotification notificationWithName:@"MWParameterValueChange" object:self];
			[[NSNotificationCenter defaultCenter] postNotification:nt];
		}
	}
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
			self.bindValue = [self attribute:"bind-value" fromElement:def defaultsTo:@""];
			
			TiXmlElement* option = def->FirstChildElement("option");
			while(option!=0) {
				NSString* name = [self attribute:"name" fromElement:option defaultsTo:@""];
				NSString* value = [self attribute:"value" fromElement:option defaultsTo:@""];
				[_options setObject:value forKey:name];
				option = option->NextSiblingElement("option");
			}
			
			if([self.bindValue length]>0) {
				[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(boundStateChanged:) name:@"MWStateChange" object:nil];
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
	@try {
		return [[MWParameterView alloc] initWithParameter:self inRect:rect immediate:imm inController:cs];
	}
	@catch (NSException * e) {
	}
	@finally {
	}
	return nil;
}

- (void) dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[_friendly release];
	[_min release];
	[_max release];
	[_id release];
	[_type release];
	[_nature release];
	[_value release];
	[_options release];
	[_bindValue release];
	[super dealloc];
}

@end