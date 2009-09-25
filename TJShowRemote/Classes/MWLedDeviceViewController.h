#import <UIKit/UIKit.h>

@interface MWLedDeviceViewController : UIViewController <UIAccelerometerDelegate> {
	UISlider* redSlider;
	UISlider* greenSlider;
	UISlider* blueSlider;
	UISlider* graySlider;
	UISegmentedControl* modeSwitch;
	UISwitch* useAccelerometer;
	UISegmentedControl* adSwitch;
}

- (IBAction) valueChanged:(id)sender;
- (IBAction) accelerometerSettingChanged:(id)sender;
- (IBAction) reset:(id)sender;
- (void) setDeviceService:(NSNetService*)service;

@property (nonatomic, retain) IBOutlet UISlider* redSlider;
@property (nonatomic, retain) IBOutlet UISlider* greenSlider;
@property (nonatomic, retain) IBOutlet UISlider* blueSlider;
@property (nonatomic, retain) IBOutlet UISlider* graySlider;
@property (nonatomic, retain) IBOutlet UISegmentedControl* modeSwitch;
@property (nonatomic, retain) IBOutlet UISwitch* useAccelerometer;
@property (nonatomic, retain) IBOutlet UISegmentedControl* adSwitch;

@end
