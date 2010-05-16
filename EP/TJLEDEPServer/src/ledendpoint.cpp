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
 
 #include "../include/ledendpoint.h"
#include <EP/include/eposcipconnection.h>
#include <EP/include/ependpointdefinition.h>

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
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"r")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"g")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"b")));
	AddMethod(ref<EPMethod>(setColor), &LEDEndpoint::MSetColor);
	
	ref<EPMethodDefinition> fadeColor = GC::Hold(new EPMethodDefinition(L"fadeColor", L"/ep/basic/color/fade", L"Fade to color"));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"r")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"g")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0", EPParameter::NatureUnknown, L"b")));
	AddMethod(ref<EPMethod>(fadeColor), &LEDEndpoint::MFadeColor);
	
	ref<EPMethodDefinition> dim = GC::Hold(new EPMethodDefinition(L"dim", L"/ep/basic/dim", L"Dim light"));
	dim->AddParameter(GC::Hold(new EPParameterDefinition(L"Value", L"double", L"0", L"1", L"1", EPParameter::NatureUnknown, L"dim")));
	AddMethod(ref<EPMethod>(dim), &LEDEndpoint::MDim);
	
	ref<EPMethodDefinition> reset = GC::Hold(new EPMethodDefinition(L"reset", L"/ep/basic/reset", L"Reset device"));
	AddMethod(ref<EPMethod>(reset), &LEDEndpoint::MReset);
	
	ref<EPMethodDefinition> powerSleep = GC::Hold(new EPMethodDefinition(L"sleep", L"/ep/basic/power/sleep", L"Sleep device"));
	AddMethod(ref<EPMethod>(powerSleep), &LEDEndpoint::MPowerSleep);
	
	ref<EPMethodDefinition> powerOff = GC::Hold(new EPMethodDefinition(L"off", L"/ep/basic/power/off", L"Turn off device"));
	AddMethod(ref<EPMethod>(powerOff), &LEDEndpoint::MPowerOff);
	
	BindVariable(L"dim", &_dim);
	BindVariable(L"r", &_r);
	BindVariable(L"g", &_g);
	BindVariable(L"b", &_b);
}

void LEDEndpoint::UpdateColor(bool fading) {
	float dim = _dim;
	if(dim<0.0f) {
		dim = 0.0f;
	}
	
	unsigned char rc = int(float(_r) * dim) & 0xFF;
	unsigned char gc = int(float(_g) * dim) & 0xFF;
	unsigned char bc = int(float(_b) * dim) & 0xFF;
	
	if(fading) {
		_device->SetColorFading(rc,gc,bc);
	}
	else {
		_device->SetColorDirectly(rc,gc,bc);
	}
}

void LEDEndpoint::MReset(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	_device->SetColorDirectly(0,0,0);
	_dim = 0.0f;
	_r = 255.0f;
	_g = 255.0f;
	_b = 255.0f;
}

void LEDEndpoint::MDim(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	_dim = m->GetParameter(0).Force(Any::TypeDouble);
	UpdateColor(true);
}


void LEDEndpoint::MPowerSleep(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_dim = 0.0f;
	UpdateColor(true);
}

void LEDEndpoint::MPowerOff(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_r = 255.0;
	_g = 255.0;
	_b = 255.0;
	_dim = 0.0f;
	UpdateColor(true);
}

void LEDEndpoint::SetColorNormalized(double r, double g, double b) {
	double maxComponent = Util::Max(r, Util::Max(g,b));
	
	if(maxComponent<=0.0 || r < 0.0 || g < 0.0 || b < 0.0) {
		_r = 255.0;
		_g = 255.0;
		_b = 255.0;
		_dim = 0.0;
	}
	else {
		// Make the largest component 255.0 (so 255*(0.25, 0.5, 0.3) becomes 255*(0.5, 1.0, 0.6)
		// Then calculate the average for both and set the dim level to that ratio
		// A nice side-effect is that this also catches values >> 1.0
		double average = (r + g + b)/3.0;
		r = double(r / maxComponent) * 255.0;
		g = double(g / maxComponent) * 255.0;
		b = double(b / maxComponent) * 255.0;
		double newAverage = (r + g + b)/3;
		_dim = average / newAverage;
		_r = r;
		_g = g;
		_b = b;
	}
}

void LEDEndpoint::MSetColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	SetColorNormalized(double(msg->GetParameter(0)), double(msg->GetParameter(1)), double(msg->GetParameter(2)));
	UpdateColor(false);
}

void LEDEndpoint::MFadeColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	SetColorNormalized(double(msg->GetParameter(0)), double(msg->GetParameter(1)), double(msg->GetParameter(2)));
	UpdateColor(true);
	
}
