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