#import "MWLedDeviceViewController.h"
#include "../../Libraries/OSCPack/osc/OscOutboundPacketStream.h"
#include "../../Libraries/OSCPack/ip/IpEndpointName.h"
#include "../../Libraries/OSCPack/ip/UdpSocket.h"
#include <arpa/inet.h>
using namespace osc;

@interface MWLedDeviceViewController ()
	UdpSocket* sock;
@end

@implementation MWLedDeviceViewController
@synthesize redSlider, greenSlider, blueSlider, graySlider, modeSwitch, useAccelerometer, adSwitch;

- (void) setDeviceService:(NSNetService*)service {
	if(sock!=0) {
		delete sock;
	}
	
	// Lookup the address
	if([[service addresses] count]<1) {
		return;
	}
	
	NSData* data = [[service addresses] objectAtIndex:0];
	if(data==nil) {
		return;
	}
	
	struct sockaddr_in* address = (struct sockaddr_in*)[data bytes];
	sock = new UdpSocket();
	sock->Connect(IpEndpointName(ntohl(address->sin_addr.s_addr), [service port]));
	
	[self.navigationItem setTitle:[service name]];
}

- (IBAction) reset: (id)sender {
	if(sock!=NULL) {
		char buffer[1024];
		OutboundPacketStream ops(buffer,1023);
		ops << osc::BeginMessage("reset") << osc::EndMessage;
		sock->Send(ops.Data(), ops.Size());
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
	if(sock!=NULL) {
		float d = [graySlider value];
		int r = (int)([redSlider value]*255.0f*d);
		int g = (int)([greenSlider value]*255.0f*d);
		int b = (int)([blueSlider value]*255.0f*d);
		
		char buffer[1024];
		OutboundPacketStream ops(buffer,1023);
		
		
		switch([modeSwitch selectedSegmentIndex]) {
			case 1:
				ops << osc::BeginMessage("/tj/ep/color/set") << (int32)r << (int32)g << (int32)b << osc::EndMessage;
				sock->Send(ops.Data(), ops.Size());
				break;
				
			case 2:
				ops << osc::BeginMessage("/tj/ep/color/fade") << (int32)r << (int32)g << (int32)b << osc::EndMessage;
				sock->Send(ops.Data(), ops.Size());
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
	delete sock;
	[redSlider release];
	[greenSlider release];
	[blueSlider release];
	[graySlider release];
	[modeSwitch release];
	[useAccelerometer release];
    [super dealloc];
}


@end
