#ifndef _TJ_LEDEP_LEDS_H
#define _TJ_LEDEP_LEDS_H

#include <string>
#include <stdio.h>

namespace tj {
	namespace ep {
		namespace leds {
			class Leds {
				public:
					Leds(const char* device);
					virtual ~Leds();
					void Fade(unsigned char r, unsigned char g, unsigned char b);
					void Direct(unsigned char r, unsigned char g, unsigned char b);
					
				protected:
					void SendCommand(const std::string& cmd);
				FILE* _f;
			};
		}
	} 
}

#endif