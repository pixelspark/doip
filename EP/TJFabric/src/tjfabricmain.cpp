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
#include <signal.h>

#ifdef TJ_OS_MAC
	#include <sys/stat.h>
#endif

#ifdef TJ_OS_POSIX
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <fcntl.h>
#endif

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;
using namespace tj::np;
using namespace tj::ep;

Event _signalOccurred;
int _lastSignal = 0;

void sigIntHandler(int s) {
	_lastSignal = s;
	_signalOccurred.Signal();
}

int main(int argc, char** argv) {
	signal(SIGINT, sigIntHandler);
	
	#ifdef TJ_OS_POSIX
		signal(SIGHUP, sigIntHandler);
		signal(SIGCHLD, sigIntHandler);
	#endif
	
	#ifdef TJ_OS_MAC
		signal(SIGINFO, sigIntHandler);
	#endif
	
	try {
		Log::SetLogToConsole(true);
		Log::Write(L"TJFabric/Main", std::wstring(L"Starting at ")+Date().ToFriendlyString());
		
		bool runningAsDaemon = false;
		if(argc>1) {
			std::string parameter = argv[1];
			if(parameter=="--daemon") {
				runningAsDaemon = true;
				#ifdef TJ_OS_POSIX
					Log::Write(L"TJFabric/Main", L"Will run as daemon");
					// Fork off a child process that continues to run, while the parent exits
					int i = fork();
					if(i<0) {
						exit(1); /* fork error */
					}
					else if(i>0) {
						exit(0); /* parent exits */
					}
					
					// Make process independent
					setsid();
					chdir("/tmp");
					
					// Restrict file creation (mode will be 750)
					umask(027);
					
					// Let's see if we're the only ones running
					int lfp = open("/var/run/tjfabricd.pid", O_RDWR|O_CREAT, 0640);
					if (lfp<0) {
						Log::Write(L"TJFabric/Main", L"Cannot open lock file");
						exit(1); /* can not open */
					}
					
					if(lockf(lfp,F_TLOCK,0)<0) {
						Log::Write(L"TJFabric/Main", L"Cannot lock file; is another instance already running?");
						exit(0); /* can not lock */
					}
					
					/* save PID to lock file */
					std::ostringstream wos;
					wos << getpid() << '\n';
					std::string pidString = wos.str();
					write(lfp, pidString.c_str(),pidString.length());
				#endif
				
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
		
		// Wait for stop (interruption with SIGINT will signal _globalStop)
		while(true) {
			_signalOccurred.Wait();
			if(_lastSignal==SIGINT) {
				break;
			}
			#ifdef TJ_OS_MAC
				else if(_lastSignal==SIGINFO) {
					std::wostringstream info;
					info << L"GC: " << tj::shared::intern::Resource::GetResourceCount() << L"Threads: " << Thread::GetThreadCount();
					Log::Write(L"TJFabric/Main", info.str());
				}
			#endif

			#ifdef TJ_OS_POSIX
				else if(_lastSignal==SIGHUP) {
					// Reload config?
					Log::Write(L"TJFabric/Main", L"Received hang-up signal; will reload configuration");
				}
				else if(_lastSignal==SIGCHLD) {
					Log::Write(L"TJFabric/Main", L"A child process terminated");
				}
			#endif

			else {
				break;
			}
		}
		
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