#include "../include/tjfabricqueue.h"
#include "../../TJScript/include/tjscript.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricmessage.h"
#include "../include/tjfabricmessagescriptable.h"
#include "../include/tjfabric.h"
#include "../include/tjfabricengine.h"

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;

/** QueueGlobalScriptable **/
namespace tj {
	namespace fabric {
		class QueueGlobalScriptable: public ScriptObject<QueueGlobalScriptable> {
			public:
				QueueGlobalScriptable(ref<Queue> q);
				virtual ~QueueGlobalScriptable();
				static void Initialize();
				virtual ref<Scriptable> SPrint(ref<ParameterList> p);
				virtual ref<Scriptable> SSend(ref<ParameterList> p);
			
			protected:
				weak<Queue> _queue;
		};
	}
}

/* Queue */
Queue::Queue(ref<FabricEngine> f): _engine(f) {
}


Queue::~Queue() {
}

void Queue::Clear() {
	ThreadLock lock(&_lock);
	_scriptCache.clear();
	_queue.clear();
}

void Queue::Add(strong<Message> m) {
	ThreadLock lock(&_lock);
	_queue.push_back(m);
	_thread->SignalWorkAdded();
}

void Queue::OnCreated() {
	_global = GC::Hold(new ScriptScope());
	_context = GC::Hold(new ScriptContext(GC::Hold(new QueueGlobalScriptable(this))));
	_context->AddType(L"Message", GC::Hold(new MessageScriptType()));
	
	_thread = GC::Hold(new QueueThread(this));
	_thread->Start();
}

void Queue::WaitForCompletion() {
	_thread->WaitForCompletion();
}

void Queue::ExecuteScript(strong<Rule> r, strong<CompiledScript> cs, strong<Message> m) {
	ThreadLock lock(&_lock);
	if(!r->IsEnabled()) Throw(L"Cannot execute the script of a rule that is disabled", ExceptionTypeError);
	try {
		ref<ScriptScope> sc = GC::Hold(new ScriptScope()); // TODO persist with rule?
		sc->Set(L"message", GC::Hold(new MessageScriptable(m)));
		//_context->SetDebug(true);
		_context->Execute(cs, sc);
	}
	catch(const tj::script::ScriptException& se) {
		Log::Write(L"TJFabric/Engine", std::wstring(L"Script exception in ExecuteScript: ")+se.GetMsg());
	}
}

void Queue::ProcessMessage(strong<Message> m) {
	try {
		ThreadLock lock(&_lock);
		std::deque< ref<Rule> > matchingRules;
		
		ref<FabricEngine> fe = _engine;
		if(fe) {
			ref<Fabric> f = fe->GetFabric();
			if(f) {
				f->GetAllMatchingRules(m->GetPath(), matchingRules);
				std::deque< ref<Rule> >::iterator it = matchingRules.begin();
				while(it!=matchingRules.end()) {
					ref<Rule> rule = *it;
					if(rule) {
						ExecuteScript(rule, GetScriptForRule(rule), m);
					}
					++it;
				}
			}
			else {
				Log::Write(L"TJFabric/Queue", L"Dropping message, no fabric");
			}
		}
	}
	catch(const Exception& e) {
		Log::Write(L"TJFabric/Queue", std::wstring(L"Exception occurred while processing message '")+m->GetPath()+L"': " + e.GetMsg());
	}
}

ref<CompiledScript> Queue::GetScriptForRule(strong<Rule> r) {
	ThreadLock lock(&_lock);
	std::map< ref<Rule>,ref<CompiledScript> >::iterator it = _scriptCache.find(ref<Rule>(r));
	if(it==_scriptCache.end()) {
		try {
			ref<CompiledScript> cs = _context->Compile(r->GetScriptSource());
			_scriptCache[ref<Rule>(r)] = cs;
			return cs;
		}
		catch(const ParserException& e) {
			Log::Write(L"TJFabric/Queue", std::wstring(L"Could not compile script, parser exception: ")+e.GetMsg());
		}
		return null;
	}
	return it->second;
}


QueueThread::QueueThread(ref<Queue> q): _queue(q), _running(false) {
}

QueueThread::~QueueThread() {
	Stop();
	WaitForCompletion();
}

void QueueThread::Start() {
	if(!_running) {
		_running = true;
		Thread::Start();
	}
}

void QueueThread::SignalWorkAdded() {
	if(_running) {
		_signal.Signal();
	}
}

void QueueThread::Stop() {
	if(_running) {
		_running = false;
		_signal.Signal();
	}
}

void QueueThread::Run() {
	Log::Write(L"TJFabric/QueueThread", L"Queue processing thread started");
	while(_running) {
		_signal.Wait();
		if(!_running) {
			Log::Write(L"TJFabric/QueueThread", L"Thread was stopped");
			return;
		}
		
		// Process queue
		{
			ref<Queue> q = _queue;
			if(!q) {
				Log::Write(L"TJFabric/QueueThread", L"Queue does not exist anymore, stopping thread");
				return;
			}
			
			ThreadLock lock(&(q->_lock));
			std::deque< ref<Message> >::iterator it = q->_queue.begin();
			while(it!=q->_queue.end()) {
				ref<Message> msg = *it;
				if(msg) {
					Log::Write(L"TJFabric/QueueThread", std::wstring(L"Process message: ")+msg->GetPath());
					q->ProcessMessage(msg);
				}
				++it;
			}
			q->_queue.clear();
		}
	}
}

/** QueueGlobalScriptable **/
QueueGlobalScriptable::QueueGlobalScriptable(ref<Queue> q): _queue(q) {
}

QueueGlobalScriptable::~QueueGlobalScriptable() {
}

void QueueGlobalScriptable::Initialize() {
	Bind(L"print", &QueueGlobalScriptable::SPrint);
	Bind(L"send", &QueueGlobalScriptable::SSend);
}

ref<Scriptable> QueueGlobalScriptable::SSend(ref<ParameterList> p) {
	static Parameter<std::wstring> PToGroupID(L"group", 0);
	std::wstring gid = PToGroupID.Require(p, L"");
	
	ref<Scriptable> ms = p->Get(L"message");
	if(!ms) {
		ms = p->Get(L"1");
	}
	
	if(ms && ms.IsCastableTo<MessageScriptable>()) {
		strong<Message> m = ref<MessageScriptable>(ms)->GetMessage();
		ref<Queue> q = _queue;
		if(q) {
			ref<FabricEngine> fe = q->_engine;
			if(fe) {
				fe->Send(gid, m);
			}
		}
	}
	return ScriptConstants::Null;
}

ref<Scriptable> QueueGlobalScriptable::SPrint(ref<ParameterList> p) {
	static Parameter<std::wstring> PMessage(L"message", 0);
	std::wstring msg = PMessage.Require(p, L"");
	Log::Write(L"TJFabric/Script", msg);
	return ScriptConstants::Null;
}