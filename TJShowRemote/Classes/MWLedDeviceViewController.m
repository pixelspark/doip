#import "MWLedDeviceViewController.h"

@implementation MWLedDeviceViewController
@synthesize redSlider, greenSlider, blueSlider, graySlider, modeSwitch, useAccelerometer, adSwitch;

- (void) setDeviceService:(NSNetService*)service {
	if(port!=nil) {
		[port release];
	}
	
	// Lookup the address
	if([[service addresses] count]<1) {
		return;
	}
	
	NSData* data = [[service addresses] objectAtIndex:0];
	if(data==nil) {
		return;
	}
	
	struct sockaddr* address = (struct sockaddr*)[data bytes];
	port = [[OSCPort oscPortWithAddress:address] retain];
	
	[self.navigationItem setTitle:[service name]];
}

- (IBAction) reset: (id)sender {
	if(port!=nil) {
		[port sendTo:"reset" types:""];
	}
}

float clamp(float v, float lower, float upper) {
	if(v>upper) {
		return upper;
	}
	else if(v<lower) {
		return lower;
	}
	return v;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration {
	float d = ([acceleration x] + 1.0f) / 2.0f;
	float g = (([acceleration y] + 1.0f) / 2.0f) * 4.0f;
	bool discrete = [adSwitch selectedSegmentIndex]==1;
	if(discrete) {
		g = floor(g*2.0f) / 2.0f;
	}
	
	
	//float b = ([acceleration z] + 1.0f) / 2.0f;
	
	graySlider.value = d;
	
	redSlider.value = 1.0f - clamp(fabs(1.0f - g), 0.0f, 1.0f);
	greenSlider.value = 1.0f - clamp(fabs(2.0f - g), 0.0f, 1.0f);
	blueSlider.value = 1.0f - clamp(fabs(3.0f - g), 0.0f, 1.0f);
	[self valueChanged:nil];
}

- (IBAction) accelerometerSettingChanged: (id)sender {
	bool enabled = [useAccelerometer isOn];
	if(enabled) {
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval:0.1];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
	}
	else {
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval:0.0];
		[[UIAccelerometer sharedAccelerometer] setDelegate:nil];
	}
}

- (IBAction) valueChanged:(id)sender {
	if(port!=nil) {
		float d = [graySlider value];
		int r = (int)([redSlider value]*255.0f*d);
		int g = (int)([greenSlider value]*255.0f*d);
		int b = (int)([blueSlider value]*255.0f*d);
		
		switch([modeSwitch selectedSegmentIndex]) {
			case 1:
				[port sendTo:"set" types:"iii", r, g, b];
				break;
				
			case 2:
				[port sendTo:"fade" types:"iii", r, g, b];
				break;
				
			case 0:
			default:
				// Do nothing
				break;
		};
	}
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	[[UIAccelerometer sharedAccelerometer] setDelegate:nil];
}

- (void)dealloc {
	[port release];
	[redSlider release];
	[greenSlider release];
	[blueSlider release];
	[graySlider release];
	[modeSwitch release];
	[useAccelerometer release];
    [super dealloc];
}


@end
