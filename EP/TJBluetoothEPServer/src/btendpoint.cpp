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