#include <TJShared/include/tjshared.h>
#include <EP/include/eppublication.h>
#include <EP/include/eposcipconnection.h>
#include <TJNP/include/tjwebserver.h>
#include "libudev.h"

#include <iomanip>
#include <sstream>

using namespace tj::shared;
using namespace tj::ep;

static void log_fn(struct udev *udev, int priority,
				   const char *file, int line, const char *fn,
				   const char *format, va_list args)
{
	
	fprintf(stderr, "%s: ", fn);
	vfprintf(stderr, format, args);
}


static void print_device(struct udev_device *device) {
	try {
		const char *str;
		dev_t devnum;
		int count;
		struct udev_list_entry *list_entry;
		
		str = udev_device_get_syspath(device);
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; system path: "+Wcs(std::string(str)));
		
		str = udev_device_get_sysname(device);
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; system name: "+Wcs(std::string(str)));
		
		str = udev_device_get_sysnum(device);
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; system num: "+Wcs(std::string(str)));
		
		str = udev_device_get_devpath(device);
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; dev path: "+Wcs(std::string(str)));
		
		str = udev_device_get_subsystem(device);
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; subsys: "+Wcs(std::string(str)));
		
		str = udev_device_get_devtype(device);
		if(str!=0) {
			Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; dev type: "+Wcs(std::string(str)));
		}
		
		str = udev_device_get_driver(device);
		if(str!=0) {
			Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; driver: "+Wcs(std::string(str)));
		}
		
		str = udev_device_get_devnode(device);
		if(str!=0) {
			Log::Write(L"TJPlugServer/DeviceDiscovery", L"Found device; name: "+Wcs(std::string(str)));
		}
	}
	catch(...) {
		Log::Write(L"TJPlugServer/DeviceDiscovery", L"Exception in print_device");
	}
	/*
	count = 0;
	udev_list_entry_foreach(list_entry, udev_device_get_devlinks_list_entry(device)) {
		printf("link:      '%s'\n", udev_list_entry_get_name(list_entry));
		count++;
	}
	if (count > 0)
		printf("found %i links\n", count);
	
	count = 0;
	udev_list_entry_foreach(list_entry, udev_device_get_properties_list_entry(device)) {
		printf("property:  '%s=%s'\n",
		       udev_list_entry_get_name(list_entry),
		       udev_list_entry_get_value(list_entry));
		count++;
	}
	if (count > 0)
		printf("found %i properties\n", count);
	
	str = udev_device_get_property_value(device, "MAJOR");
	if (str != NULL)
		printf("MAJOR: '%s'\n", str);
	
	str = udev_device_get_sysattr_value(device, "dev");
	if (str != NULL)
		printf("attr{dev}: '%s'\n", str);
	
	printf("\n");*/
}


static int test_monitor(struct udev *udev) {
	struct udev_monitor *udev_monitor;
	fd_set readfds;
	int fd;
	
	udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
	if (udev_monitor == NULL) {
		Log::Write(L"TJPlugServer/Monitor",L"no socket");
		return -1;
	}
	/*if (udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "block", NULL) < 0 ||
	    udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "tty", NULL) < 0 ||
		udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "bluetooth", NULL) < 0 ||
	    udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "usb", "usb_device") < 0) {
		Log::Write(L"TJPlugServer/Monitor",L"filter failed");
		return -1;
	}*/
	if (udev_monitor_enable_receiving(udev_monitor) < 0) {
		Log::Write(L"TJPlugServer/Monitor",L"bind failed");
		return -1;
	}
	
	fd = udev_monitor_get_fd(udev_monitor);
	FD_ZERO(&readfds);
	
	while (1) {
		struct udev_device *device;
		int fdcount;
		
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(fd, &readfds);
		
		Log::Write(L"TJPlugServer/Monitor",L"waiting for events\n");
		fdcount = select(fd+1, &readfds, NULL, NULL, NULL);
		Log::Write(L"TJPlugServer/Monitor",L"got stuff\n");
		
		if (FD_ISSET(fd, &readfds)) {
			device = udev_monitor_receive_device(udev_monitor);
			if (device == NULL) {
				Log::Write(L"TJPlugServer/Monitor",L"no device\n");
				continue;
			}
			print_device(device);
			udev_device_unref(device);
		}
		
		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			Log::Write(L"TJPlugServer/Monitor",L"exit\n");
			break;
		}
	}
	
	udev_monitor_unref(udev_monitor);
	return 0;
}


int main (int argc, char * const argv[]) {
	if(argc<2) {
		Log::Write(L"TJPlugServer/Main", L"Usage: tjplugd [--daemon] [settings.xml]");
		return 1;
	}
	
	strong<Daemon> daemon = Daemon::Instance();
	SharedDispatcher sd;
	
	Log::Write(L"TJPlugServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::string firstParameter = argv[1];
	if(firstParameter=="--daemon") {
		if(!daemon->Fork(L"tjplugd",true)) {
			return 0;
		}
	}
	
	std::wstring settingsPath = SettingsStorage::GetSystemSettingsPath(L"TJ", L"PlugD", L"global");
	Log::Write(L"TJPlugServer/Main", L"Loading server settings from "+settingsPath);
	
	ref<SettingsStorage> st = GC::Hold(new SettingsStorage());
	// Set default settings values here (st->SetValue)
	
	try {
		st->LoadFile(settingsPath);
	}
	catch(...) {
		Log::Write(L"TJPlugServer/Main", L"Could not load settings file at path "+settingsPath+L"; using defaults");
	}
	st->SaveFile(settingsPath);
	
	try {
		struct udev* udev = udev_new();
		printf("context: %p\n", udev);
		if (udev == NULL) {
			printf("no context\n");
			return 1;
		}
		
		///udev_log_init("tjplugd");
		///udev_set_log_fn(udev, log_fn);
		test_monitor(udev);
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
	
	Log::Write(L"TJPlugServer/Main", L"Graceful shutdown");
	return 0;
}
