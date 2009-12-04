#import "MWOptionView.h"
#import "MWParameter.h"
#import "MWAppDelegate.h"
#import "MWMethod.h"

@implementation MWOptionView
@synthesize parameter = _parameter;
@synthesize parentViewController = _parentViewController;

- (void) didTouchInside:(id)sender {
	// Show a nice chooser view
	MWAppDelegate* appDelegate = (MWAppDelegate*)[[UIApplication sharedApplication] delegate];
	MWChooserController* chooser = appDelegate.chooserController;
	[chooser setParameter:_parameter];
	[chooser setChoiceDelegate:self];
	[chooser showInView:_parentViewController];
}

- (void) chooser:(MWChooserController*)ch hasChangedParameter:(MWParameter*)param {
	[self updateLabel];
	if(_immediate) {
		[param.parent execute];
	}
}

- (id) initWithFrame:(CGRect)frame parameter:(MWParameter*)param immediate:(BOOL)imm {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
		_immediate = imm;
		self.parameter = param;
		self.backgroundColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.3];
		self.titleLabel.font = [UIFont boldSystemFontOfSize:14];
		self.titleLabel.textColor = [UIColor blackColor];
		[self updateLabel];
		[self addTarget:self action:@selector(didTouchInside:) forControlEvents:UIControlEventTouchUpInside];
    }
    return self;
}

- (void) updateLabel {
	NSString* txt = @"??";
	
	for (NSString* name in [_parameter.options keyEnumerator]) {
		NSString* value = [_parameter.options objectForKey:name];
		if([_parameter.value isEqualToString:value]) {
			txt = name;
		}
	}
	
	[self setTitle:txt forState:UIControlStateNormal];
}

- (void)dealloc {
	[_parameter release];
    [super dealloc];
}


@end
