#include "../include/ledendpoint.h"
#include <EP/include/eposcipconnection.h>

using namespace tj::ep::leds;
using namespace tj::ep;
using namespace tj::shared;

LEDEndpoint::~LEDEndpoint() {
}

LEDEndpoint::LEDEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, strong<LED> device): EPEndpointServer<LEDEndpoint>(id,nsp,friendlyName), _device(device), _dim(1.0f), _r(0.0f), _g(0.0f), _b(0.0f) {
}

EPMediationLevel LEDEndpoint::GetMediationLevel() const {
	return EPMediationLevelDefault;
}

void LEDEndpoint::OnCreated() {
	EPEndpointServer<LEDEndpoint>::OnCreated();
	
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
	
	// Create the methods
	ref<EPMethodDefinition> setColor = GC::Hold(new EPMethodDefinition(L"setColor", L"/ep/basic/color/set", L"Set color"));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	AddMethod(ref<EPMethod>(setColor), &LEDEndpoint::MSetColor);
	
	ref<EPMethodDefinition> fadeColor = GC::Hold(new EPMethodDefinition(L"fadeColor", L"/ep/basic/color/fade", L"Fade to color"));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	AddMethod(ref<EPMethod>(fadeColor), &LEDEndpoint::MFadeColor);
	
	ref<EPMethodDefinition> dim = GC::Hold(new EPMethodDefinition(L"dim", L"/ep/basic/dim", L"Dim light"));
	dim->AddParameter(GC::Hold(new EPParameterDefinition(L"Value", L"double", L"0", L"1", L"1")));
	AddMethod(ref<EPMethod>(dim), &LEDEndpoint::MDim);
	
	ref<EPMethodDefinition> reset = GC::Hold(new EPMethodDefinition(L"reset", L"/ep/basic/reset", L"Reset device"));
	AddMethod(ref<EPMethod>(reset), &LEDEndpoint::MReset);
	
	ref<EPMethodDefinition> powerSleep = GC::Hold(new EPMethodDefinition(L"sleep", L"/ep/basic/power/sleep", L"Sleep device"));
	AddMethod(ref<EPMethod>(powerSleep), &LEDEndpoint::MPowerSleep);
	
	ref<EPMethodDefinition> powerOff = GC::Hold(new EPMethodDefinition(L"off", L"/ep/basic/power/off", L"Turn off device"));
	AddMethod(ref<EPMethod>(powerOff), &LEDEndpoint::MPowerOff);
}

void LEDEndpoint::PowerUp() {
	if(_dim<0.0) {
		_dim = -_dim;
	}
}

void LEDEndpoint::UpdateColor(bool fading) {
	float dim = _dim;
	if(dim<0.0f) {
		dim = 0.0f;
	}
	unsigned char rc = int(_r * dim) & 0xFF;
	unsigned char gc = int(_g * dim) & 0xFF;
	unsigned char bc = int(_b * dim) & 0xFF;
	
	if(fading) {
		_device->SetColorFading(rc,gc,bc);
	}
	else {
		_device->SetColorDirectly(rc,gc,bc);
	}
}

void LEDEndpoint::MReset(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_device->SetColorDirectly(0,0,0);
	_dim = 1.0f;
	_r = 1.0f;
	_g = 1.0f;
	_b = 1.0f;
}

void LEDEndpoint::MDim(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_dim = m->GetParameter(0);
	UpdateColor(true);
}


void LEDEndpoint::MPowerSleep(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_dim = -_dim;
	UpdateColor(true);
}

void LEDEndpoint::MPowerOff(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_r = 1.0;
	_g = 1.0;
	_b = 1.0;
	_dim = -1.0f;
	UpdateColor(false);
}

void LEDEndpoint::MSetColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_r = msg->GetParameter(0);
	_g = msg->GetParameter(1);
	_b = msg->GetParameter(2);
	UpdateColor(false);
}

void LEDEndpoint::MFadeColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_r = msg->GetParameter(0);
	_g = msg->GetParameter(1);
	_b = msg->GetParameter(2);
	UpdateColor(true);
	
}
