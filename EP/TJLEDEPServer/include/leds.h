#ifndef _LEDEP_LEDS_H
#define _LEDEP_LEDS_H

namespace tj {
	namespace ep {
		namespace leds {
			class LED {
				public:
					virtual ~LED();
					virtual void SetColorFading(unsigned char r, unsigned char g, unsigned char b) = 0;
					virtual void SetColorDirectly(unsigned char r, unsigned char g, unsigned char b) = 0;
			};
		}
	}
}

#endif