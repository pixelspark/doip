#include "../include/tubleds.h"

#include <TJShared/include/tjshared.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace tj::shared;
using namespace tj::ep::leds;

TubLED::TubLED(const char* dev) {
	_f = fopen(dev, "w");
	if(_f==0) {
		Throw(L"Could not open led device", ExceptionTypeError);
	}
}

TubLED::~TubLED() {
	fclose(_f);
}

void TubLED::SetColorFading(unsigned char r, unsigned char g, unsigned char b) {	
	std::ostringstream tll;
	tll << "0:c" 
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(r)
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(g) 
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(b) 
	<< ";";
	SendCommand(tll.str());
}

void TubLED::SetColorDirectly(unsigned char r, unsigned char g, unsigned char b) {	
	std::ostringstream tll;
	tll << "0:n" 
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(r)
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(g) 
	<< std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(b) 
	<< ";";
	SendCommand(tll.str());
}

void TubLED::SendCommand(const std::string& cmd) {
	Log::Write(L"TJLEDEPServer/TubLED", L"Command: "+Wcs(cmd));
	fwrite(cmd.c_str(), cmd.length(), 1, _f);
	fflush(_f);
}