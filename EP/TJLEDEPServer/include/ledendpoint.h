#ifndef _LEDEP_ENDPOINT_H
#define _LEDEP_ENDPOINT_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpointserver.h>
#include <EP/include/epmessage.h>

#include "leds.h"

namespace tj {
	namespace ep {
		namespace leds {
			using namespace tj::shared;
			using namespace tj::ep;
			
			class LEDEndpoint: public EPEndpointServer<LEDEndpoint> {
				public:
					LEDEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, strong<LED> device);
					virtual ~LEDEndpoint();
					virtual EPMediationLevel GetMediationLevel() const;
					virtual void OnCreated();
					virtual void MReset(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void MDim(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void MSetColor(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void MFadeColor(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void MPowerOff(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void MPowerSleep(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
					virtual void UpdateColor(bool fade);
					virtual void PowerUp();
					
				protected:
					strong<LED> _device;
					float _dim, _r, _g, _b;
			};
			
		}
	}
}

#endif