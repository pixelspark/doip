#ifndef _TJ_LEDEP_LEDS_H
#define _TJ_LEDEP_LEDS_H

#include <string>
#include <stdio.h>
#include "leds.h"

namespace tj {
	namespace ep {
		namespace leds {
			/** The TubLED protocol is a very simple protocol for controlling LEDs.
			The original implementation used an Arduino controller connected to i2c
			RGB LEDs. Although the protocol allows to select a different color for 
			each LED, this functionality is not (yet) implemented here. Instead, this
			implementation will only be able to change the color of all LEDs at the
			same time. 
			
			The protocol itself is line-based and always starts with a few numbers
			inidicating the devices for which the command is meant (the numbers are 0
			to 9; meaning that at most, there can be 8 different LEDs or groups of LEDs
			connected to the same controller; 0 means 'all LEDs'). This is followed by a ':,
			the command, and finally a ';'. Commands often start with a single letter 
			and then arguments follow, mostly in hexadecimal notation. So, an example
			command is:
			 
			0:cFF0000;
			 
			Which means that all devices connected ('0') will execute the command 'c' 
			(which happens to be 'fade to a particular color'). The color is specified
			as hexadecimal number (also known from the way colors are notated in HTML/CSS)
			and in this example is red.
			**/
			class TubLED: public LED {
				public:
					TubLED(const char* device);
					virtual ~TubLED();
					virtual void SetColorFading(unsigned char r, unsigned char g, unsigned char b);
					virtual void SetColorDirectly(unsigned char r, unsigned char g, unsigned char b);
					
				protected:
					void SendCommand(const std::string& cmd);
					FILE* _f;
			};
		}
	} 
}

#endif