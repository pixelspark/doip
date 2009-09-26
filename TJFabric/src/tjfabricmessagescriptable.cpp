#include "../include/tjfabricmessagescriptable.h"
using namespace tj::shared;
using namespace tj::script;
using namespace tj::fabric;

MessageScriptable::MessageScriptable(tj::shared::strong<Message> m): _message(m) {
}

MessageScriptable::~MessageScriptable() {
}

void MessageScriptable::Initialize() {
	Bind(L"setPath", &MessageScriptable::SSetPath);
	Bind(L"setParameter", &MessageScriptable::SSetParameter);
	Bind(L"path", &MessageScriptable::SPath);
	Bind(L"get", &MessageScriptable::SGet);
	Bind(L"toString", &MessageScriptable::SToString);
}

ref<Scriptable> MessageScriptable::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(_message->ToString()));
}

ref<Scriptable> MessageScriptable::SSetPath(ref<ParameterList> p) {
	static Parameter<std::wstring> PPath(L"path", 0);
	std::wstring path = PPath.Require(p, L"");
	_message->SetPath(path);
	return ScriptConstants::Null;
}

ref<Scriptable> MessageScriptable::SPath(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(_message->GetPath()));
}

ref<Scriptable> MessageScriptable::SGet(ref<ParameterList> p) {
	static Parameter<int> PKey(L"key", 0);
	int idx = PKey.Require(p,-1);
	if(idx>=0) {
		_message->GetParameter(idx);
	}
	return ScriptConstants::Null;
}

ref<Scriptable> MessageScriptable::SSetParameter(ref<ParameterList> p) {
	static Parameter<int> PIndex(L"index", 0);
	int idx = PIndex.Require(p, -1);
	if(idx>=0) {
		ref<Scriptable> sp = p->Get(L"1");
		if(!sp) {
			sp = p->Get(L"value");
		}
		
		if(sp && sp.IsCastableTo<ScriptAny>()) {
			_message->SetParameter(idx, ref<ScriptAny>(sp)->Unbox());
		}
		else {
			Log::Write(L"TJFabric/MessageScriptable", L"Invalid value for setParameter");
		}
	}
	return ScriptConstants::Null;
}

strong<Message> MessageScriptable::GetMessage() {
	return _message;
}

/** MessageScriptType **/
MessageScriptType::MessageScriptType() {
}

MessageScriptType::~MessageScriptType() {
}

ref<Scriptable> MessageScriptType::Construct(ref<ParameterList> p) {
	static Parameter<std::wstring> PPath(L"path", 0);
	std::wstring path = PPath.Get(p, L"");
	ref<Message> m = GC::Hold(new Message(path));
	
	/* Any extra parameter will be set as parameter in the message
	i.e. you can use new Message("test",1,2,3); */
	unsigned int a = 1;
	while(true) {
		std::wstring argName = Stringify(a);
		ref<Scriptable> arg = p->Get(argName);
		
		
		if(arg && arg.IsCastableTo<ScriptAny>()) {
			Any value = ref<ScriptAny>(arg)->Unbox();
			m->SetParameter(a-1, value);
			++a;
		}
		else {
			break;
		}
	}
	
	return GC::Hold(new MessageScriptable(m));
}