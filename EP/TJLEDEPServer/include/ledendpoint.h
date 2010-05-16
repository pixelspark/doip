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
					
				protected:
					virtual void SetColorNormalized(double r, double g, double b);
					strong<LED> _device;
					Any _dim, _r, _g, _b;
			};
			
		}
	}
}

#endif