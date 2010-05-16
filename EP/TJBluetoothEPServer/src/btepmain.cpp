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
 
 #include <TJShared/include/tjshared.h>
#include <EP/include/eppublication.h>
#include <EP/include/eposcipconnection.h>
#include <TJNP/include/tjwebserver.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "../include/btendpoint.h"

#include <iomanip>
#include <sstream>

using namespace tj::shared;
using namespace tj::ep;
using namespace tj::ep::bt;

void UpdateBluetoothEndpoints(std::map<String, ref<EPEndpoint> >& endpoints, int btDeviceID, int btSocket) {
	const static int KMaxInquiries = 255;
	inquiry_info* inquiries = new inquiry_info[KMaxInquiries];
    int len  = 8;
    int flags = IREQ_CACHE_FLUSH;
    
    // Start inquiry
    int numInquiries = hci_inquiry(btDeviceID, len, KMaxInquiries, NULL, &inquiries, flags);
    if(numInquiries<0) {
    	Throw(L"Bluetooth enquiry failed", ExceptionTypeError);
    }

	// Print a list of devices
	char addr[19] = { 0 };
    char name[248] = { 0 };
    
    std::map< String, ref<EPEndpoint> > newEndpoints;
    
    for (int i = 0; i < numInquiries; i++) {
        ba2str(&(inquiries[i].bdaddr), addr);
        String address = Wcs(std::string(addr));
        std::map< String, ref<EPEndpoint> >::iterator it = endpoints.find(address);
        if(it!=endpoints.end()) {
        	newEndpoints[address] = it->second;	
        }
        else {
        	// Create new endpoint
	        memset(name, 0, sizeof(name));
	        if(hci_read_remote_name(btSocket, &(inquiries[i].bdaddr), sizeof(name), name, 0) < 0) {
	        	Log::Write(L"TJBluetoothEPServer/Main", L"Could not get bluetooth device name for device "+Wcs(std::string(addr)));
	        }
	        //Log::Write(L"TJBluetoothEPServer/Main", L"Found "+Wcs(std::string(addr))+L": "+Wcs(std::string(name)));
	        newEndpoints[address] = GC::Hold(new BTEndpoint(address, L"org.bluetooth.device", Wcs(std::string(name))));
	    }
    }
   	delete[] inquiries;		
   	endpoints = newEndpoints;
}

int main (int argc, char * const argv[]) {
	if(argc<2) {
		Log::Write(L"TJBluetoothEPServer/Main", L"Usage: tjtoothepd [--daemon] [settings.xml]");
		return 1;
	}
	
	strong<Daemon> daemon = Daemon::Instance();
	SharedDispatcher sd;
	
	Log::Write(L"TJBluetoothEPServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::wstring settingsPath;
	std::string firstParameter = argv[1];
	std::wstring serialDevicePath;
	if(firstParameter=="--daemon") {
		if(!daemon->Fork(L"tjtoothepd",true)) {
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
		settingsPath = SettingsStorage::GetSystemSettingsPath(L"TJ", L"ToothEPD", L"global");
	}
	
	Log::Write(L"TJBluetoothEPServer/Main", L"Loading server settings from "+settingsPath);
	
	ref<SettingsStorage> st = GC::Hold(new SettingsStorage());
	// Set default settings values here (st->SetValue)
	
	try {
		st->LoadFile(settingsPath);
	}
	catch(...) {
		Log::Write(L"TJBluetoothEPServer/Main", L"Could not load settings file at path "+settingsPath+L"; using defaults");
	}
	st->SaveFile(settingsPath);
	
	try {
		// Connect to the Bluetooth dongle
		int btDeviceID = hci_get_route(NULL);
		int btSocket = hci_open_dev(btDeviceID);
		if(btDeviceID < 0 || btSocket < 0) {
			Throw(L"Could not open socket to bluetooth device; do you have a bluetooth dongle connected?", ExceptionTypeError);		
		}
		Log::Write(L"TJBluetoothEPServer/Main", L"Bluetooth device="+Stringify(btDeviceID)+L" socket="+Stringify(btSocket));
		
		// Continuously scan for devices and add them to our list
		std::map< String, ref<EPEndpoint> > endpoints;
		while(true) {
			UpdateBluetoothEndpoints(endpoints, btDeviceID, btSocket);	
			sleep(40);
		}
		close(btSocket);
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
	
	Log::Write(L"TJBluetoothEPServer/Main", L"Graceful shutdown");
	return 0;
}
