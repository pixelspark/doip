#include "../../../TJShared/include/tjshared.h"
#include "../../EPFramework/include/ependpoint.h"
#include "../../EPFramework/include/epservermanager.h"
#include "../../EPFramework/include/epconnection.h"
#include "../../EPFramework/include/eposcipconnection.h"
#include "../../../TJScout/include/tjservice.h"
#include "../../../TJNP/include/tjwebserver.h"

using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

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

#include "usp3.h"

class USPEndpoint: public EPEndpointDefinition, public Listener<MessageNotification> {
	public:
		USPEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& device);
		virtual ~USPEndpoint();
		virtual void OnCreated();
		virtual void Notify(ref<Object> src, const MessageNotification& data);
		virtual void UpdateColor(bool fade);
	
	protected:
		ref<OSCOverUDPConnection> _inConnection;
		usp3::ChromoflexStripeDevice _device;
		float _dim, _r, _g, _b;
};

USPEndpoint::USPEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& devicePath): _device(devicePath.c_str()), _dim(1.0f), _r(0.0f), _g(0.0f), _b(0.0f) {
	_id = id;
	_namespace = nsp;
	_friendlyName = friendlyName;
	_dynamic = false;
	_version = L"1.0";
}

void USPEndpoint::OnCreated() {
	EPEndpointDefinition::OnCreated();
	
	// Create an inbound UDP connection with a random port number
	ref<OSCOverUDPConnectionDefinition> udpd = GC::Hold(new OSCOverUDPConnectionDefinition());
	udpd->SetAddress(L"");
	udpd->SetPort(0);
	udpd->SetFormat(L"osc");
	udpd->SetFraming(L"");
	_inConnection = ConnectionFactory::Instance()->CreateFromDefinition(ref<ConnectionDefinition>(udpd), DirectionInbound, this);
	
	if(_inConnection) {
		_inConnection->EventMessageReceived.AddListener(this);
		udpd->SetPort(_inConnection->GetInboundPort());
		_transports.push_back(udpd);
	}
	
	// Create the methods
	ref<EPMethodDefinition> setColor = GC::Hold(new EPMethodDefinition());
	setColor->SetID(L"setColor");
	setColor->AddPath(L"/ep/basic/color/set");
	setColor->SetFriendlyName(L"Set color");
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	_methods.push_back(setColor);
	
	ref<EPMethodDefinition> fadeColor = GC::Hold(new EPMethodDefinition());
	fadeColor->SetID(L"fadeColor");
	fadeColor->AddPath(L"/ep/basic/color/fade");
	fadeColor->SetFriendlyName(L"Fade to color");
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	_methods.push_back(fadeColor);
	
	ref<EPMethodDefinition> dim = GC::Hold(new EPMethodDefinition());
	dim->SetID(L"dim");
	dim->AddPath(L"/ep/basic/dim");
	dim->SetFriendlyName(L"Dim light");
	dim->AddParameter(GC::Hold(new EPParameterDefinition(L"Value", L"double", L"0", L"1", L"1")));
	_methods.push_back(dim);
	
	ref<EPMethodDefinition> reset = GC::Hold(new EPMethodDefinition());
	reset->SetID(L"reset");
	reset->AddPath(L"/ep/basic/reset");
	reset->SetFriendlyName(L"Reset device");
	_methods.push_back(reset);
}

void USPEndpoint::UpdateColor(bool fading) {
	unsigned char rc = int(_r * _dim) & 0xFF;
	unsigned char gc = int(_g * _dim) & 0xFF;
	unsigned char bc = int(_b * _dim) & 0xFF;
	
	_device.WriteRegister(usp3::ChromoflexStripeDevice::KRegisterStatus, 0x01);
	_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterSetR, rc, gc, bc, 0x00);
	if(!fading) {
		_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterLevelR, rc, gc, bc, 0x00);
	}
}

void USPEndpoint::Notify(ref<Object> src, const MessageNotification& data) {
	try {
		Log::Write(L"TJUSP3EPServer/USPEndpoint", L"Receive message");
		ref<Message> msg = data.message;
		if(msg->GetPath()==L"/ep/basic/reset") {
			_device.WriteReset();
		}
		else if(msg->GetPath()==L"/ep/basic/dim") {
			_dim = msg->GetParameter(0);
			UpdateColor(true);
		}
		else if(msg->GetPath()==L"/ep/basic/color/set" || msg->GetPath()==L"/ep/basic/color/fade") {
			_r = msg->GetParameter(0);
			_g = msg->GetParameter(1);
			_b = msg->GetParameter(2);
			UpdateColor(msg->GetPath()==L"/ep/basic/color/fade");
		}
	}
	catch(const Exception& e) {
		Log::Write(L"TJUSP3EPServer/USPEndpoint", L"Exception occurred in message processing: "+e.GetMsg());
	}
}

USPEndpoint::~USPEndpoint() {
}

Event _signalOccurred;
int _lastSignal = 0;

void sigIntHandler(int s) {
	_lastSignal = s;
	_signalOccurred.Signal();
}

int main (int argc, char * const argv[]) {
	// Set up signal handlers
	signal(SIGINT, sigIntHandler);
	
	#ifdef TJ_OS_POSIX
		signal(SIGHUP, sigIntHandler);
		signal(SIGCHLD, sigIntHandler);
	#endif
		
	#ifdef TJ_OS_MAC
		signal(SIGINFO, sigIntHandler);
	#endif
		
	Log::SetLogToConsole(true);
	if(argc<2) {
		Log::Write(L"TJUSP3EPServer/Main", L"Usage: tjusp3epserverd /dev/serialdevice");
		return 1;
	}
	
	Log::Write(L"TJUSP3EPServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::string serialDevicePath;
	std::string firstParameter = argv[1];
	if(firstParameter=="--daemon") {
		// Run as daemon
		#ifdef TJ_OS_POSIX
			Log::Write(L"TJUSP3EPServer/Main", L"Will run as daemon");
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
			int lfp = open("/var/run/tjusp3epserverd.pid", O_RDWR|O_CREAT, 0640);
			if (lfp<0) {
				Log::Write(L"TJUSP3EPServer/Main", L"Cannot open lock file");
				exit(1); /* can not open */
			}
			
			if(lockf(lfp,F_TLOCK,0)<0) {
				Log::Write(L"TJUSP3EPServer/Main", L"Cannot lock file; is another instance already running?");
				exit(0); /* can not lock */
			}
			
			/* save PID to lock file */
			std::ostringstream wos;
			wos << getpid() << '\n';
			std::string pidString = wos.str();
			write(lfp, pidString.c_str(),pidString.length());
		#endif
		
		if(argc>2) {
			serialDevicePath = argv[2];
		}
	}
	else {
		serialDevicePath = argv[1];
	}
	
	// Publish endpoint
	try {
		Hash hash;
		ref<USPEndpoint> uspe = GC::Hold(new USPEndpoint(StringifyHex(hash.Calculate(Wcs(serialDevicePath))), L"com.tjshow.usp3epd", L"USP3 lights", serialDevicePath));
		ref<WebServer> ws = EPServerManager::Instance()->CreateServer(EPServerManager::KPortDontCare);
		ref<EPDefinitionResolver> resolver = GC::Hold(new EPDefinitionResolver(uspe));
		std::wstring definitionPath = L"/ep/" + uspe->GetFullIdentifier() + L"/definition.xml";
		ws->AddResolver(definitionPath,ref<FileRequestResolver>(resolver));
		
		// Advertise the service
		std::map<std::wstring, std::wstring> attributes;
		attributes[L"EPDefinitionPath"] = definitionPath;
		attributes[L"EPMagicNumber"] = EPServerManager::Instance()->GetServerMagic();
		unsigned short actualPort = ws->GetActualPort();
		ref<ServiceRegistration> serviceRegistration = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", uspe->GetFriendlyName(), actualPort, attributes);
		Log::Write(L"TJUSP3EPServer/Main", L"EP service active on port "+Stringify(actualPort)+L" definitionPath="+definitionPath);
		
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
	
	Log::Write(L"TJUSP3EPServer/Main", L"Graceful shutdown");
	return 0;
}
