#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricmessage.h"
#include "../../TJScript/include/tjscript.h"

#include <iomanip>
#include <sstream>
#include <signal.h>
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;

ref<Queue> _globalQueue;
ref<FabricEngine> _globalEngine;

void handler(int s) {
	_globalQueue->Stop();
}

int main(int argc, char** argv) {
	signal(SIGINT, handler);
	
	try {
		Log::SetLogToConsole(true);
		Log::Write(L"TJFabric/Main", std::wstring(L"Starting up at ")+Timestamp(true).ToString());
		
		std::string fabricFile;
		if(argc<2) {
			Log::Write(L"TJFabric/Main", L"No fabric configuration file given, using ./default.fabric");
			fabricFile = "./default.fabric";
		}
		else {
			fabricFile = argv[1];
		}
		
		_globalEngine = GC::Hold(new FabricEngine());
		strong<Fabric> fabric = _globalEngine->GetFabric();

		// Load the fabric configuration file
		Fabric::LoadRecursive(fabricFile, fabric);
		_globalEngine->Connect(true);
		
		// Initialize fabric with init message
		_globalQueue = _globalEngine->GetQueue();
		ref<Message> msg = GC::Hold(new Message(L"init"));
		_globalQueue->Add(msg);
		
		// Wait for completion (interruption with SIGINT will stop the queue thread)
		_globalQueue->WaitForCompletion();
		_globalQueue = null;
		_globalEngine = null;
		Log::Write(L"TJFabric/Main", L"Graceful shutdown");
	}
	catch(const ScriptException& e) {
		std::wcerr << L"Script exception: " << e.GetMsg() << std::endl;
	}
	catch(const Exception& e) {
		std::wcerr << L"Exception: " << e.GetMsg() << std::endl;
	}
	catch(std::exception& e) {
		std::cerr << "Standard Exception: " << e.what() << std::endl;
	}
	catch(...) {
		std::wcerr << L"Unknown exception occurred" << std::endl;
	}
	
	return 0;
}