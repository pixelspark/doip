#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricmessage.h"
#include "../../TJScript/include/tjscript.h"

#include <iomanip>
#include <sstream>
using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;

int main(int argc, char** argv) {
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
		
		ref<FabricEngine> fe = GC::Hold(new FabricEngine());
		strong<Fabric> fabric = fe->GetFabric();

		/** Load the fabric configuration file **/
		Fabric::LoadRecursive(fabricFile, fabric);
		fe->Connect(true);
		
		// For testing
		ref<Queue> q = fe->GetQueue();
		ref<Message> msg = GC::Hold(new Message(L"init"));
		msg->SetParameter(0,Any(12));

		#ifdef TJ_OS_POSIX
			sleep(1);
		#endif

		#ifdef TJ_OS_WIN
			Sleep(10000);
		#endif
		q->Add(msg);
		
		q->WaitForCompletion();
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