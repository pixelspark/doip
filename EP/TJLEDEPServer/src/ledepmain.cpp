#include <TJShared/include/tjshared.h>
#include <EP/include/eppublication.h>
#include <EP/include/eposcipconnection.h>
#include <TJNP/include/tjwebserver.h>

#include "../include/ledendpoint.h"
#include "../include/tubleds.h"
#include "../include/usp3leds.h"
#include "../include/ambxleds.h"
#include <iomanip>
#include <sstream>

using namespace tj::shared;
using namespace tj::ep;
using namespace tj::ep::leds;

int main (int argc, char * const argv[]) {
	if(argc<2) {
		Log::Write(L"TJLEDEPServer/Main", L"Usage: tjledepd [--daemon] [/dev/ttyUSB0] [settings.xml]");
		return 1;
	}
	
	strong<Daemon> daemon = Daemon::Instance();
	
	Log::Write(L"TJLEDEPServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::wstring settingsPath;
	std::string firstParameter = argv[1];
	std::wstring serialDevicePath;
	if(firstParameter=="--daemon") {
		if(!daemon->Fork(L"tjledepd",true)) {
			return 0;
		}
		
		if(argc>2) {
			serialDevicePath = Wcs(argv[2]);
			if(argc>3) {
				settingsPath = Wcs(argv[3]);
			}
		}
	}
	else {
		serialDevicePath = Wcs(argv[1]);
		if(argc>2) {
			settingsPath = Wcs(argv[2]);
		}
	}
	
	if(settingsPath.length()<1) {
		settingsPath = SettingsStorage::GetSystemSettingsPath(L"TJ", L"LEDEPD", L"global");
	}
	
	Log::Write(L"TJLEDEPServer/Main", L"Loading server settings from "+settingsPath);
	
	ref<SettingsStorage> st = GC::Hold(new SettingsStorage());
	st->SetValue(L"ep.friendly-name", L"LEDs");
	st->SetValue(L"led.device-path", L"/dev/ttyUSB0");
	st->SetValue(L"led.protocol", L"tubled");
	st->SetValue(L"ep.tags", L"Lighting");
	
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
		if(serialDevicePath.length()==0) {
			serialDevicePath = st->GetValue(L"led.device-path");
		}
		String serialProtocol = st->GetValue(L"led.protocol");
		String idh = StringifyHex(hash.Calculate(serialDevicePath));
		Log::Write(L"TJLEDEPServer/Main", L"Using serial device path "+serialDevicePath+L", protocol="+serialProtocol+L" ID="+idh);
		
		ref<LED> leds;
		if(serialProtocol==L"tubled") {
			leds = GC::Hold(new TubLED(Mbs(serialDevicePath).c_str()));
		}
		else if(serialProtocol==L"usp3") {
			leds = GC::Hold(new USP3LED(Mbs(serialDevicePath).c_str()));
		}
		else if(serialProtocol==L"ambx") {
			leds = GC::Hold(new AmbxLED());
		}
		
		if(leds) {
			ref<LEDEndpoint> uspe = GC::Hold(new LEDEndpoint(idh, L"com.tjshow.leds", st->GetValue(L"ep.friendly-name"), leds));
			uspe->SetTags(st->GetValue(L"ep.tags"));
			ref<EPPublication> pub = GC::Hold(new EPPublication(ref<EPEndpoint>(uspe)));
			uspe->Publish(pub);
			Log::Write(L"TJLEDEPServer/Main", L"Running");
			daemon->Run();
		}
		else {
			Log::Write(L"TJLEDEPServer/Main", L"LED protocol not supported");
		}
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
