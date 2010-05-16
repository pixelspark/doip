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
 
 #ifndef _BTEP_ENDPOINT_H
#define _BTEP_ENDPOINT_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpointserver.h>
#include <EP/include/epmessage.h>
#include <EP/include/eppublication.h>

namespace tj {
	namespace ep {
		namespace bt {
			using namespace tj::shared;
			using namespace tj::ep;
			
			class BTEndpoint: public EPEndpointServer<BTEndpoint> {
				public:
					BTEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName);
					virtual ~BTEndpoint();
					virtual EPMediationLevel GetMediationLevel() const;
					virtual void OnCreated();
					
				protected:
					ref<EPPublication> _publication;
			};
			
		}
	}
}

#endif