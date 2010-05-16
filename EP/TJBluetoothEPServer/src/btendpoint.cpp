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
 
 #include "../include/btendpoint.h"

#include <EP/include/eposcipconnection.h>

using namespace tj::ep::bt;
using namespace tj::ep;
using namespace tj::shared;

BTEndpoint::~BTEndpoint() {
}

BTEndpoint::BTEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName): EPEndpointServer<BTEndpoint>(id,nsp,friendlyName) {
}

EPMediationLevel BTEndpoint::GetMediationLevel() const {
	return EPMediationLevelDefault;
}

void BTEndpoint::OnCreated() {
	EPEndpointServer<BTEndpoint>::OnCreated();
	
	// Create an inbound UDP connection with a random port number
	ref<OSCOverUDPConnectionDefinition> udpd = GC::Hold(new OSCOverUDPConnectionDefinition());
	udpd->SetAddress(L"127.0.0.1"); // force IPv4
	udpd->SetPort(0);
	udpd->SetFormat(L"osc");
	udpd->SetFraming(L"");
	ref<OSCOverUDPConnection> inConnection = ConnectionFactory::Instance()->CreateFromDefinition(ref<ConnectionDefinition>(udpd), DirectionInbound, this);
	
	if(inConnection) {
		inConnection->EventMessageReceived.AddListener(this);
		udpd->SetPort(inConnection->GetInboundPort());
		udpd->SetAddress(L"");
		AddTransport(ref<EPTransport>(udpd), inConnection);
	}
	
	// TODO: allow configuration of additional tags (gateway-wide tag, for instance) and maybe specify more information about the services the device supports
	SetTags(L"Mobile Wireless");
	
	_publication = GC::Hold(new EPPublication(ref<EPEndpoint>(this)));
}