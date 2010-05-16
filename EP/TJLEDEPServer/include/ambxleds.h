/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_LEDEP_AMBXLEDS_H
#define _TJ_LEDEP_AMBXLEDS_H

#include <string>
#include <stdio.h>
#include "leds.h"

namespace tj {
	namespace ep {
		namespace leds {
			class AmbxLED: public LED {
				public:
					enum Light {
						NoLight = 0x00,
						LeftSpeakerLight = 0x0B,
						RightSpeakerLight = 0x1B,
						LeftWallLight = 0x2B,
						CenterWallLight = 0x3B,
						RightWallLight = 0x4B,
					};
				
					AmbxLED();
					virtual ~AmbxLED();
					virtual void SetColorFading(unsigned char r, unsigned char g, unsigned char b);
					virtual void SetColorDirectly(unsigned char r, unsigned char g, unsigned char b);
					
				protected:
					virtual void SetColorFading(Light l, unsigned char r, unsigned char g, unsigned char b);
					virtual void SetColor(Light l, unsigned char r, unsigned char g, unsigned char b);
					static void Initialize();
					static bool _usbInitialized;
					void* _device;
					unsigned char _r, _g, _b;
				
					enum Commands {
						CommandSetLightColor = 0x03,
						CommandListWithDelay = 0x72,
						CommandSetFanSpeed = 0x01,
					};
			};
		}
	} 
}

#endif