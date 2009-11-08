#include "../include/tjfabricengine.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricqueue.h"
#include "../../../TJScript/include/tjscript.h"
#include "../../../TJNP/include/tjnetworkaddress.h"
#include "../../../TJNP/include/tjsocket.h"
#include "../../../TJNP/include/tjhttp.h"
#include "../../EPFramework/include/epmessage.h"

#include <iomanip>
#include <sstream>

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;
using namespace tj::np;
using namespace tj::ep;

int main(int argc, char** argv) {
	try {
		Log::SetLogToConsole(true);
		Log::Write(L"TJFabric/Main", std::wstring(L"Starting at ")+Date().ToFriendlyString());
		strong<Daemon> daemon = Daemon::Instance();
		
		bool runningAsDaemon = false;
		if(argc>1) {
			std::string parameter = argv[1];
			if(parameter=="--daemon") {
				if(!daemon->Fork(L"tjfabricd", true)) {
					return 0;
				}
				runningAsDaemon = true;
			}
		}
		
		// Load fabric configuration files
		std::map<std::string, ref<FabricEngine> > fabrics;
		for(int a=(runningAsDaemon ? 2 : 1); a < argc; a++) {
			ref<FabricEngine> engine = GC::Hold(new FabricEngine());
			strong<Fabric> fabric = engine->GetFabric();
			Fabric::LoadRecursive(argv[a], fabric);
			engine->Connect(true);
			fabrics[argv[a]] = engine;

			if(a==(argc-1)) {
				// Last fabric, send init message
				engine->GetQueue()->Add(GC::Hold(new Message(L"init")), null, null);
			}
		}
		
		daemon->Run();
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