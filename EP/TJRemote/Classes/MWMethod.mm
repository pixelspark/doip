#import "MWMethod.h"
#import "MWEndpoint.h"
#import "MWFavorite.h"
#import "../../../Libraries/TinyXML/tinyxml.h"

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

- (void) setupCell:(UITableViewCell *)cell inController:(UIViewController*)controller {
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
			CGRect rect = CGRectMake(160, 8, 120, 28);
			UIView* pv = [firstParameter createView:rect immediate:YES inController:controller];
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
