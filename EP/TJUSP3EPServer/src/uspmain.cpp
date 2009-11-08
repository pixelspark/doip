#include "../../../TJShared/include/tjshared.h"
#include "../../EPFramework/include/ependpointserver.h"
#include "../../EPFramework/include/eppublication.h"
#include "../../EPFramework/include/eposcipconnection.h"

using namespace tj::shared;
using namespace tj::ep;

#include <iomanip>
#include <sstream>

#include "usp3.h"

class USPEndpoint: public EPEndpointServer<USPEndpoint> {
	public:
		USPEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& device);
		virtual ~USPEndpoint();
		virtual void OnCreated();
		virtual void MReset(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void MDim(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void MSetColor(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void MFadeColor(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void MPowerOff(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void MPowerSleep(strong<Message> m, ref<Connection> src, ref<ConnectionChannel> cc);
		virtual void UpdateColor(bool fade);
		virtual void PowerUp();
	
	protected:
		usp3::ChromoflexStripeDevice _device;
		float _dim, _r, _g, _b;
};

USPEndpoint::USPEndpoint(const std::wstring& id, const std::wstring& nsp, const std::wstring& friendlyName, const std::string& devicePath): EPEndpointServer<USPEndpoint>(id,nsp,friendlyName), _device(devicePath.c_str()), _dim(1.0f), _r(0.0f), _g(0.0f), _b(0.0f) {
}

void USPEndpoint::OnCreated() {
	EPEndpointServer<USPEndpoint>::OnCreated();
	
	// Create an inbound UDP connection with a random port number
	ref<OSCOverUDPConnectionDefinition> udpd = GC::Hold(new OSCOverUDPConnectionDefinition());
	udpd->SetAddress(L"");
	udpd->SetPort(0);
	udpd->SetFormat(L"osc");
	udpd->SetFraming(L"");
	ref<OSCOverUDPConnection> inConnection = ConnectionFactory::Instance()->CreateFromDefinition(ref<ConnectionDefinition>(udpd), DirectionInbound, this);
	
	if(inConnection) {
		inConnection->EventMessageReceived.AddListener(this);
		udpd->SetPort(inConnection->GetInboundPort());
		AddTransport(ref<EPTransport>(udpd), inConnection);
	}
	
	// Create the methods
	ref<EPMethodDefinition> setColor = GC::Hold(new EPMethodDefinition(L"setColor", L"/ep/basic/color/set", L"Set color"));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	setColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	AddMethod(ref<EPMethod>(setColor), &USPEndpoint::MSetColor);
	
	ref<EPMethodDefinition> fadeColor = GC::Hold(new EPMethodDefinition(L"fadeColor", L"/ep/basic/color/fade", L"Fade to color"));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Red", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Green", L"int32", L"0", L"255", L"0")));
	fadeColor->AddParameter(GC::Hold(new EPParameterDefinition(L"Blue", L"int32", L"0", L"255", L"0")));
	AddMethod(ref<EPMethod>(fadeColor), &USPEndpoint::MFadeColor);
	
	ref<EPMethodDefinition> dim = GC::Hold(new EPMethodDefinition(L"dim", L"/ep/basic/dim", L"Dim light"));
	dim->AddParameter(GC::Hold(new EPParameterDefinition(L"Value", L"double", L"0", L"1", L"1")));
	AddMethod(ref<EPMethod>(dim), &USPEndpoint::MDim);
	
	ref<EPMethodDefinition> reset = GC::Hold(new EPMethodDefinition(L"reset", L"/ep/basic/reset", L"Reset device"));
	AddMethod(ref<EPMethod>(reset), &USPEndpoint::MReset);
	
	ref<EPMethodDefinition> powerSleep = GC::Hold(new EPMethodDefinition(L"sleep", L"/ep/basic/power/sleep", L"Sleep device"));
	AddMethod(ref<EPMethod>(powerSleep), &USPEndpoint::MPowerSleep);
	
	ref<EPMethodDefinition> powerOff = GC::Hold(new EPMethodDefinition(L"off", L"/ep/basic/power/off", L"Turn off device"));
	AddMethod(ref<EPMethod>(powerOff), &USPEndpoint::MPowerOff);
}

void USPEndpoint::PowerUp() {
	if(_dim<0.0) {
		_dim = -_dim;
	}
}

void USPEndpoint::UpdateColor(bool fading) {
	float dim = _dim;
	if(dim<0.0f) {
		dim = 0.0f;
	}
	unsigned char rc = int(_r * dim) & 0xFF;
	unsigned char gc = int(_g * dim) & 0xFF;
	unsigned char bc = int(_b * dim) & 0xFF;
	
	_device.WriteRegister(usp3::ChromoflexStripeDevice::KRegisterStatus, 0x01);
	_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterSetR, rc, gc, bc, 0x00);
	if(!fading) {
		_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterLevelR, rc, gc, bc, 0x00);
	}
}

void USPEndpoint::MReset(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_device.WriteReset();
	_dim = 1.0f;
	_r = 1.0f;
	_g = 1.0f;
	_b = 1.0f;
}

void USPEndpoint::MDim(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_dim = m->GetParameter(0);
	UpdateColor(true);
}


void USPEndpoint::MPowerSleep(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_dim = -_dim;
	UpdateColor(true);
}

void USPEndpoint::MPowerOff(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	_r = 1.0;
	_g = 1.0;
	_b = 1.0;
	_dim = -1.0f;
	UpdateColor(false);
}

void USPEndpoint::MSetColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_r = msg->GetParameter(0);
	_g = msg->GetParameter(1);
	_b = msg->GetParameter(2);
	UpdateColor(false);
}

void USPEndpoint::MFadeColor(strong<Message> msg, ref<Connection> c, ref<ConnectionChannel> cc) {
	PowerUp();
	_r = msg->GetParameter(0);
	_g = msg->GetParameter(1);
	_b = msg->GetParameter(2);
	UpdateColor(true);
	
}

USPEndpoint::~USPEndpoint() {
}

int main (int argc, char * const argv[]) {
	if(argc<2) {
		Log::Write(L"TJUSP3EPServer/Main", L"Usage: tjusp3epserverd /dev/serialdevice");
		return 1;
	}
	
	strong<Daemon> daemon = Daemon::Instance();
	
	Log::Write(L"TJUSP3EPServer/Main", L"Starting at "+Date().ToFriendlyString());
	std::string serialDevicePath;
	std::string firstParameter = argv[1];
	if(firstParameter=="--daemon") {
		if(!daemon->Fork(L"tjusp3epserverd",true)) {
			return 0;
		}
		
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
		String hs = Wcs(serialDevicePath);
		String idh = StringifyHex(hash.Calculate(hs));
		Log::Write(L"TJUSP3EPServer/Main", L"hash for path "+Wcs(serialDevicePath)+L" is "+idh);
		ref<USPEndpoint> uspe = GC::Hold(new USPEndpoint(idh, L"com.tjshow.usp3epd", L"USP3 lights", serialDevicePath));
		ref<EPPublication> pub = GC::Hold(new EPPublication(ref<EPEndpoint>(uspe)));
		Log::Write(L"TJUSP3EPServer/Main", L"Running");
		daemon->Run();
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
