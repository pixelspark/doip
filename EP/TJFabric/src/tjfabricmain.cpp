#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricmessage.h"
#include "../../../TJScript/include/tjscript.h"

#include <iomanip>
#include <sstream>
#include <signal.h>
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;

Event _globalStop;

void sigIntHandler(int s) {
	if(s==SIGINT) {
		_globalStop.Signal();
	}
}

int main(int argc, char** argv) {
	signal(SIGINT, sigIntHandler);
	
	try {
		Log::SetLogToConsole(true);
		Log::Write(L"TJFabric/Main", std::wstring(L"Starting up at ")+Timestamp(true).ToString());
		
		std::map<std::string, ref<FabricEngine> > fabrics;
		if(argc<2) {
			Log::Write(L"TJFabric/Main", L"No fabric configuration file given, using ./default.fabric");
			std::string fabricFile = "./default.fabric";
		}
		else {
			for(int a=1; a < argc; a++) {
				ref<FabricEngine> engine = GC::Hold(new FabricEngine());
				strong<Fabric> fabric = engine->GetFabric();
				Fabric::LoadRecursive(argv[a], fabric);
				engine->Connect(true);
				fabrics[argv[a]] = engine;
			}
		}
		
		// Wait for stop (interruption with SIGINT will signal _globalStop)
		_globalStop.Wait();
		fabrics.clear();
	}
	catch(const ScriptException& e) {
		std::wcerr << L"Script exception: " << e.GetMsg() << std::endl;
		return -1;
	}
	catch(const Exception& e) {
		std::wcerr << L"Exception: " << e.GetMsg() << std::endl;
		return -1;
	}
	catch(std::exception& e) {
		std::cerr << "Standard Exception: " << e.what() << std::endl;
		return -1;
	}
	catch(...) {
		std::wcerr << L"Unknown exception occurred" << std::endl;
		return -1;
	}
	
	Log::Write(L"TJFabric/Main", L"Graceful shutdown");
	return 0;
}