#include <TJShared/include/tjshared.h>
#include <EP/include/ependpointserver.h>
#include <EP/include/eppublication.h>
#include <EP/include/eposcipconnection.h>
#include <TJNP/include/tjwebserver.h>

#include "leds.h"
#include <iomanip>
#include <sstream>

using namespace tj::shared;
using namespace tj::ep;
using namespace tj::ep::leds;

class LEDEndpoint: public EPEndpointServer<LEDEndpoint> {
	public:
		LEDEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& device);
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
		Leds _device;
		float _dim, _r, _g, _b;
};

LEDEndpoint::LEDEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& devicePath): EPEndpointServer<LEDEndpoint>(id,nsp,friendlyName), _device(devicePath.c_str()), _dim(1.0f), _r(0.0f), _g(0.0f), _b(0.0f) {
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
		_device.Fade(rc,gc,bc);
	}
	else {
		_device.Direct(rc,gc,bc);
	}
}

void LEDEndpoint::MReset(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_device.Direct(0,0,0);
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

LEDEndpoint::~LEDEndpoint() {
}

int main (int argc, char * const argv[]) {
	if(argc<2) {
		Log::Write(L"TJLEDEPServer/Main", L"Usage: tjledepd [--daemon] [settings.xml]");
		return 1;
	}
	
	strong<Daemon> daemon = Daemon::Instance();
	
	Log::Write(L"TJLEDEPServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::wstring settingsPath;
	std::string firstParameter = argv[1];
	if(firstParameter=="--daemon") {
		if(!daemon->Fork(L"tjledepd",true)) {
			return 0;
		}
		
		if(argc>2) {
			settingsPath = Wcs(argv[2]);
		}
	}
	else {
		settingsPath = Wcs(argv[1]);
	}
	
	if(settingsPath.length()<1) {
		settingsPath = SettingsStorage::GetSystemSettingsPath(L"TJ", L"LEDEPD", L"global");
	}
	
	Log::Write(L"TJLEDEPServer/Main", L"Loading server settings from "+settingsPath);
	
	ref<SettingsStorage> st = GC::Hold(new SettingsStorage());
	st->SetValue(L"ep.friendly-name", L"LEDs");
	st->SetValue(L"led.device-path", L"/dev/ttyUSB0");
	try {
		st->LoadFile(settingsPath);
	}
	catch(...) {
		Log::Write(L"TJLEDEPServer/Main", L"Could not load settings file at path "+settingsPath+L"; using defaults");
	}
	st->SaveFile(settingsPath);
	
	// Publish endpoint
	try {
		Hash hash;
		String serialDevicePath = st->GetValue(L"led.device-path");
		String idh = StringifyHex(hash.Calculate(serialDevicePath));
		Log::Write(L"TJLEDEPServer/Main", L"Using serial device path "+serialDevicePath+L" ID="+idh);
		ref<LEDEndpoint> uspe = GC::Hold(new LEDEndpoint(idh, L"com.tjshow.leds", st->GetValue(L"ep.friendly-name"), Mbs(serialDevicePath)));
		//ref<tj::np::WebServer> ws = GC::Hold(new tj::np::WebServer(2122));
		ref<EPPublication> pub = GC::Hold(new EPPublication(ref<EPEndpoint>(uspe)));
		Log::Write(L"TJLEDEPServer/Main", L"Running");
		daemon->Run();
	}
	catch(const Exception& e) {
		std::wcerr << L"Exception: " << e.GetMsg() << std::endl;
		return -1;
	}
	catch(std::exception& e) {
		std::cerr << "Standard Exception: " << e.what() << std::endl;
		return -1;
	}
	catch(const char* msg) {
		std::cerr << "Error: " << msg << std::endl;
		return -1;
	}
	catch(...) {
		std::wcerr << L"Unknown exception occurred" << std::endl;
		return -1;
	}
	
	Log::Write(L"TJLEDEPServer/Main", L"Graceful shutdown");
	return 0;
}
