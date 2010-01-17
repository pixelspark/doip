#import "TTParameterMenuItem.h"

@implementation TTParameterMenuItem

- (void) update {
	std::wstring type = _parameter->GetType();
	if(type==EPParameter::KTypeBoolean) {
		// Boolean; simply set a check mark
		bool defaultValue = (bool)_parameter->GetDefaultValue();
		[(NSButton*)_dataView setState:(defaultValue ? NSOnState: NSOffState)];
	}
	else {
		if(_parameter->HasOptions()) {
			// TODO
		}
		else {
			if(type==EPParameter::KTypeDouble || type==EPParameter::KTypeInt32) {
				double value = _parameter->GetDefaultValue();
				[(NSSlider*)_dataView setDoubleValue:value];
			}
		}
	}
	
	if(_valueLabel!=nil) {
		std::string value = Mbs(_parameter->GetDefaultValue().Force(_parameter->GetValueType()).ToString());
		[_valueLabel setTitleWithMnemonic:[NSString stringWithUTF8String:value.c_str()]];
	}
}

- (void) onCheckboxClick: (id)sender {
	NSButton* checkbox = (NSButton*)sender;
	_parameter->SetDefaultValue(Any(bool([checkbox state]==NSOnState)).Force(Any::TypeBool));
	[self update];
}

- (void) onSliderChange: (id)sender {
	NSSlider* slider = (NSSlider*)sender;
	_parameter->SetDefaultValue(Any([slider doubleValue]));
	[self update];
}

- (id) initWithParameter:(ref<EPParameter>)p {
	std::string parameterName = Mbs(p->GetFriendlyName());
	if(self = [super initWithTitle:[NSString stringWithUTF8String:parameterName.c_str()] action:nil keyEquivalent:@""]) {
		_parameter = p;
		
		int KWidth = 180;
		int KValueWidth = 24;
		int KMargin = 10;
		NSView* wrapper = [[NSView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0,0,KWidth,24))];
		NSRect controlFrame = NSRectFromCGRect(CGRectMake(KMargin,0,KWidth-2*KMargin,24));
		NSRect labelFrame = NSRectFromCGRect(CGRectMake(KMargin, 2, (KWidth-3*KMargin)*(1.0-0.619), 22));
		NSRect rightFrame = NSRectFromCGRect(CGRectMake((KWidth-2*KMargin)*(1.0-0.619)+KMargin, 0, (KWidth-2*KMargin)*(0.619), 24));
		NSRect rightFrameWithValue = rightFrame;
		rightFrameWithValue.size.width -= KValueWidth;
		NSRect	valueFrame = rightFrame;
		valueFrame.origin.x = rightFrameWithValue.size.width+rightFrameWithValue.origin.x;
		valueFrame.size.width = KValueWidth;
		valueFrame.size.height -= 8;
		valueFrame.origin.y += 4;
		
		std::wstring type = _parameter->GetType();
		if(type==EPParameter::KTypeBoolean) {
			NSButton* checkbox = [[NSButton alloc] initWithFrame:controlFrame];
			[checkbox setButtonType:NSSwitchButton];
			[checkbox setTitle:[NSString stringWithUTF8String:parameterName.c_str()]];
			[checkbox setFont:[NSFont systemFontOfSize:15]];
			[checkbox setTarget:self];
			[checkbox setAction:@selector(onCheckboxClick:)];
		
			[wrapper addSubview:checkbox];
			_dataView = checkbox;
		}
		else {
			if(_parameter->HasOptions()) {
				std::set< EPOption > options;
				_parameter->GetOptions(options);
				
				float wrapperHeight = options.size()*24;
				NSRect wrapperFrame = [wrapper frame];
				NSRect newWrapperFrame = NSRectFromCGRect(CGRectMake(wrapperFrame.origin.x, wrapperFrame.origin.y, wrapperFrame.size.width, wrapperHeight));
				[wrapper setFrame:newWrapperFrame];
				NSMatrix* matrix = [[NSMatrix alloc] initWithFrame:newWrapperFrame mode:NSRadioModeMatrix cellClass:[NSButtonCell class] numberOfRows:options.size() numberOfColumns:1];
				[wrapper addSubview:matrix];
				_dataView = matrix;
			}
			else {
				NSTextField* label = [[NSTextField alloc] initWithFrame:labelFrame];
				[label setTitleWithMnemonic:[NSString stringWithUTF8String:parameterName.c_str()]];
				[label setBordered:FALSE];
				[label setBezeled:NO];
				[label setFont:[NSFont systemFontOfSize:14]];
				[label setAlignment:NSRightTextAlignment];
				[wrapper addSubview:label];
				[label release];

				bool hasValueLabel = _parameter->GetNature()==EPParameter::NatureDiscrete;
				if(hasValueLabel) {
					// Create a value label
					_valueLabel = [[NSTextField alloc] initWithFrame:valueFrame];
					[_valueLabel setBordered:FALSE];
					[_valueLabel setBezeled:NO];
					[_valueLabel setFont:[NSFont systemFontOfSize:10]];
					[_valueLabel setAlignment:NSLeftTextAlignment];
					//[_valueLabel setVerticalAlignment:NSTextBlockMiddleAlignment];
					[wrapper addSubview:_valueLabel];
				}

				if(type==EPParameter::KTypeDouble || type==EPParameter::KTypeInt32) {
					NSSlider* slider = [[NSSlider alloc] initWithFrame:(hasValueLabel ? rightFrameWithValue: rightFrame)];
					[slider setMinValue:(double)_parameter->GetMinimumValue()];
					[slider setMaxValue:(double)_parameter->GetMaximumValue()];
					[slider setTitle:[NSString stringWithUTF8String:parameterName.c_str()]];
					[slider setAction:@selector(onSliderChange:)];
					[slider setTarget:self];
					[wrapper addSubview:slider];
					_dataView = slider;
				}
				else if(type==EPParameter::KTypeString) {
					NSTextField* text = [[NSTextField alloc] initWithFrame:rightFrame];
					[text setEditable:YES];
					[text setEnabled:YES];
					[text setBezelStyle:NSTextFieldRoundedBezel];
					[text setTitleWithMnemonic:[NSString stringWithUTF8String:Mbs(p->GetDefaultValue()).c_str()]];
					[wrapper addSubview:text];
					_dataView = text;
				}
			}
			
		}
		[self setView:wrapper];
		[wrapper release];
		[self update];
	}
	
	return self;
}

- (void) dealloc {
	[_dataView release];
	[_valueLabel release];
	[super dealloc];
}

@end
