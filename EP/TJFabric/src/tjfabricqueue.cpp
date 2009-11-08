#include "../include/tjfabricqueue.h"
#include "../../../TJScript/include/tjscript.h"
#include "../include/tjfabricrule.h"
#include "../include/tjfabricmessagescriptable.h"
#include "../include/tjfabric.h"
#include "../include/tjfabricengine.h"

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;
using namespace tj::ep;

/** QueueGlobalScriptable **/
namespace tj {
	namespace fabric {
		class ConnectionScriptable: public ScriptObject<ConnectionScriptable> {
			public:
				ConnectionScriptable(ref<Connection> c, ref<ConnectionChannel> chan);
				virtual ~ConnectionScriptable();
				static void Initialize();
				virtual ref<Scriptable> SToString(ref<ParameterList> p);
				virtual ref<Connection> GetConnection();
				virtual ref<ConnectionChannel> GetConnectionChannel();

			protected:
				ref<Connection> _connection;
				ref<ConnectionChannel> _channel;
		};

		class QueueGlobalScriptable: public ScriptObject<QueueGlobalScriptable> {
			public:
				QueueGlobalScriptable(ref<Queue> q);
				virtual ~QueueGlobalScriptable();
				static void Initialize();
				virtual ref<Scriptable> SPrint(ref<ParameterList> p);
				virtual ref<Scriptable> SSend(ref<ParameterList> p);
				virtual ref<Scriptable> SDefer(ref<ParameterList> p);
				virtual ref<Scriptable> STry(ref<ParameterList> p);
			
			protected:
				weak<Queue> _queue;
		};

		class QueueReplyHandler: public ReplyHandler {
			public:
				QueueReplyHandler(ref<Queue> q, ref<ScriptDelegate> dlg);
				virtual ~QueueReplyHandler();
				virtual void OnReceiveReply(strong<Message> originalMessage, strong<Message> replyMessage, strong<Connection> con, ref<ConnectionChannel> cc);
				virtual void OnEndReply(strong<Message> originalMessage);

			protected:
				weak<Queue> _queue;
				ref<ScriptDelegate> _dlg;
		};
	}
}

/** QueueReplyHandler **/
QueueReplyHandler::QueueReplyHandler(ref<Queue> q, ref<ScriptDelegate> dlg): _queue(q), _dlg(dlg) {
}

QueueReplyHandler::~QueueReplyHandler() {
}

void QueueReplyHandler::OnReceiveReply(strong<Message> originalMessage, strong<Message> replyMessage, strong<Connection> connection, ref<ConnectionChannel> channel) {
	ref<Queue> q = _queue;
	if(q) {
		q->AddReply(GC::Hold(new QueuedReply(originalMessage, replyMessage, _dlg, connection, channel)));
	}
}

void QueueReplyHandler::OnEndReply(strong<Message> originalMessage) {
}

/* QueuedReply */
QueuedReply::QueuedReply(strong<Message> o, strong<Message> r, ref<ScriptDelegate> d, strong<Connection> con, ref<ConnectionChannel> cc): _originalMessage(o), _replyMessage(r), _delegate(d), _connection(con), _channel(cc) {
}

QueuedReply::~QueuedReply() {
}

/** QueuedMessage **/
QueuedMessage::QueuedMessage(strong<Message> msg, ref<Connection> src, ref<ConnectionChannel> cc): _message(msg), _source(src), _sourceChannel(cc) {
}

QueuedMessage::~QueuedMessage() {
}

/* Queue */
Queue::Queue(ref<FabricEngine> f): _engine(f) {
}


Queue::~Queue() {
}

void Queue::Stop() {
	if(_thread) {
		_thread->Stop();
	}
}

void Queue::Clear() {
	ThreadLock lock(&_lock);
	_scriptCache.clear();
	_queue.clear();
	_global = GC::Hold(new ScriptScope());
}

void Queue::AddReply(strong<QueuedReply> m) {
	ThreadLock lock(&_lock);
	_replyQueue.push_back(m);
	SignalWorkAdded();
}

void Queue::Add(strong<Message> m, ref<Connection> c, ref<ConnectionChannel> cc) {
	ThreadLock lock(&_lock);
	_queue.push_back(GC::Hold(new QueuedMessage(m, c, cc)));
	SignalWorkAdded();
}

void Queue::SignalWorkAdded() {
	if(!_thread) {
		_thread = GC::Hold(new QueueThread(this));
		_thread->Start();
	}
	_thread->SignalWorkAdded();
}

void Queue::OnCreated() {
	_global = GC::Hold(new ScriptScope());
	_context = GC::Hold(new ScriptContext(GC::Hold(new QueueGlobalScriptable(this))));
	_context->AddType(L"Message", GC::Hold(new MessageScriptType()));
}

void Queue::WaitForCompletion() {
	if(_thread) {
		_thread->WaitForCompletion();
	}
}

void Queue::ExecuteScript(strong<Rule> r, strong<CompiledScript> cs, strong<QueuedMessage> m) {
	ThreadLock lock(&_lock);
	if(!r->IsEnabled()) Throw(L"Cannot execute the script of a rule that is disabled", ExceptionTypeError);
	try {
		ref<ScriptScope> sc = GC::Hold(new ScriptScope()); // TODO persist with rule?
		sc->Set(L"globals", _global);
		sc->Set(L"message", GC::Hold(new MessageScriptable(m->_message)));
		sc->Set(L"source", GC::Hold(new ConnectionScriptable(m->_source, m->_sourceChannel)));
		//_context->SetDebug(true);
		_context->Execute(cs, sc);
	}
	catch(const tj::script::ScriptException& se) {
		Log::Write(L"TJFabric/Engine", std::wstring(L"Script exception in ExecuteScript: ")+se.GetMsg());
	}
}

void Queue::ProcessReply(strong<QueuedReply> r) {
	try {
		ThreadLock lock(&_lock);
		ref<ScriptScope> sc = GC::Hold(new ScriptScope());
		sc->Set(L"globals", _global);
		sc->Set(L"reply", GC::Hold(new MessageScriptable(r->_replyMessage)));
		sc->Set(L"message", GC::Hold(new MessageScriptable(r->_originalMessage)));
		sc->Set(L"source", GC::Hold(new ConnectionScriptable(r->_connection, r->_channel)));
		_context->Execute(r->_delegate->GetScript(), sc);
	}
	catch(const Exception& e) {
		Log::Write(L"TJFabric/Queue", std::wstring(L"Exception occurred while processing reply '")+r->_replyMessage->GetPath()+L" to "+r->_originalMessage->GetPath()+L"': " + e.GetMsg());
	}
}

void Queue::ProcessMessage(strong<QueuedMessage> qm) {
	strong<Message> m = qm->_message;

	try {
		ThreadLock lock(&_lock);
		std::deque< ref<Rule> > matchingRules;
		
		ref<FabricEngine> fe = _engine;
		if(fe) {
			ref<Fabric> f = fe->GetFabric();
			if(f) {
				f->GetAllMatchingRules(m->GetPath(), m->GetParameterTags(), matchingRules);
				std::deque< ref<Rule> >::iterator it = matchingRules.begin();
				while(it!=matchingRules.end()) {
					ref<Rule> rule = *it;
					if(rule) {
						ExecuteScript(rule, GetScriptForRule(rule), qm);
					}
					++it;
				}
			}
			else {
				Log::Write(L"TJFabric/Queue", L"Dropping message, no fabric");
			}
		}
		else {
			Log::Write(L"TJFabric/Queue", L"Dropping message, no fabric engine");
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

void Queue::AddDiscoveryScriptCall(ref<DiscoveryDefinition> def, ref<Connection> connection, const String& source) {
	try {
		ref<CompiledScript> cs = _context->Compile(source);
		if(cs) {
			ref<ScriptScope> sc = GC::Hold(new ScriptScope());
			sc->Set(L"globals", _global);
			sc->Set(L"source", GC::Hold(new ConnectionScriptable(connection, null)));
			RunAsynchronously(GC::Hold(new ScriptDelegate(cs, _context)), sc);
		}
	}
	catch(const ParserException& e) {
		Log::Write(L"TJFabric/Queue", std::wstring(L"Could not compile discovery script, parser exception: ")+e.GetMsg());
	}
}


void Queue::RunAsynchronously(ref<ScriptDelegate> dlg, ref<ScriptScope> scope) {
	if(dlg) {
		ThreadLock lock(&_lock);
		_asyncScriptsQueue.push_back(ScriptCall(dlg,scope));
		SignalWorkAdded();
	}
}

void Queue::ProcessScriptCall(Queue::ScriptCall& call) {
	ThreadLock lock(&_lock);
	
	try {
		ref<ScriptContext> ctx = call.first->GetContext();
		if(ctx) {
			ctx->Execute(call.first->GetScript(), call.second);
		}
		else {
			Throw(L"No context set for asynchronous script delegate; script not executed", ExceptionTypeError);
		}
	}
	catch(const Exception& e) {
		Log::Write(L"TJFabric/Queue", std::wstring(L"Exception occurred while executing asynchronous script:") + e.GetMsg());
	}	
}

/** QueueThread **/
QueueThread::QueueThread(ref<Queue> q): _queue(q), _running(false) {
}

QueueThread::~QueueThread() {
	Stop();
	WaitForCompletion();
}

void QueueThread::Start() {
	if(!_running) {
		_running = true;
		_signal.Reset();
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
		_signal.Reset();
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
			
			// Process asynchronous scripts
			if(q->_asyncScriptsQueue.size()>0) {
				std::deque<Queue::ScriptCall>::iterator it = q->_asyncScriptsQueue.begin();
				while(it!=q->_asyncScriptsQueue.end()) {
					Queue::ScriptCall& call = *it;
					if(call.first) {
						q->ProcessScriptCall(call);
					}
					++it;
				}
				q->_asyncScriptsQueue.clear();
			}

			// Process queued replies
			if(q->_replyQueue.size()>0) {
				std::deque< ref<QueuedReply> >::iterator it = q->_replyQueue.begin();
				while(it!=q->_replyQueue.end()) {
					ref<QueuedReply> msg = *it;
					if(msg) {
						Log::Write(L"TJFabric/QueueThread", std::wstring(L"Process reply: ")+msg->_replyMessage->GetPath()+L" to: "+msg->_originalMessage->GetPath());
						q->ProcessReply(msg);
					}
					++it;
				}
				q->_replyQueue.clear();
			}

			// Process queued messages
			if(q->_queue.size()>0) {
				std::deque< ref<QueuedMessage> >::iterator it = q->_queue.begin();
				while(it!=q->_queue.end()) {
					ref<QueuedMessage> msg = *it;
					if(msg) {
						Log::Write(L"TJFabric/QueueThread", std::wstring(L"Process message: ")+msg->_message->GetPath());
						q->ProcessMessage(msg);
					}
					++it;
				}
				q->_queue.clear();
			}
		}
		
		_signal.Wait();
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
	Bind(L"defer", &QueueGlobalScriptable::SDefer);
	Bind(L"try", &QueueGlobalScriptable::STry);
}

ref<Scriptable> QueueGlobalScriptable::STry(ref<ParameterList> p) {
	ref<Scriptable> ms = p->Get(L"0");
	
	if(ms && ms.IsCastableTo<ScriptDelegate>()) {
		ref<Queue> q = _queue;
		if(q) {
			ref<ScriptDelegate> dgate = ms;
			try {
				p->Set(L"globals", q->_global);
				q->_context->Execute(dgate->GetScript(), p);
			}
			catch(const Exception& e) {
				Log::Write(L"TJFabric/Queue", L"Try-block exception: "+e.GetMsg());
			}
			catch(const std::exception& e) {
				Log::Write(L"TJFabric/Queue", L"Try-block standard exception: "+Wcs(e.what()));
			}
			catch(...) {
				Log::Write(L"TJFabric/Queue", L"Unknown exception in try-block");
			}
			return ScriptConstants::Null;
		}
	}
	Throw(L"Invalid argument to defer; should be a delegate!", ExceptionTypeError);
	
}

ref<Scriptable> QueueGlobalScriptable::SDefer(ref<ParameterList> p) {
	return null; // Not implemented yet
}

ref<Scriptable> QueueGlobalScriptable::SSend(ref<ParameterList> p) {	
	Log::Write(L"TJFabric/QueueGlobalScriptable", L"SSend");

	ref<Scriptable> ms = p->Get(L"message");
	if(!ms) {
		ms = p->Get(L"1");
	}

	ref<Scriptable> rh = p->Get(L"reply");
	if(!rh) {
		rh = p->Get(L"2");
	}

	ref<Scriptable> to = p->Get(L"to");
	if(!to) {
		to = p->Get(L"0");
		if(!to) {
			throw ScriptException(L"Missing parameter 'to' in send method");
		}
	}
	
	if(ms && ms.IsCastableTo<MessageScriptable>()) {
		strong<Message> m = ref<MessageScriptable>(ms)->GetMessage();
		ref<Queue> q = _queue;
		if(q) {
			ref<FabricEngine> fe = q->_engine;
			if(fe) {
				// If we have a handler delegate, hand it to the engine
				ref<ScriptDelegate> handler;
				ref<ReplyHandler> replyHandler;
				if(rh.IsCastableTo<ScriptDelegate>()) {
					handler = rh;
					Log::Write(L"TJFabric/QueueGlobalScriptable", L"Has handler in SSend, creating delegate-based reply handler");
					replyHandler = GC::Hold(new QueueReplyHandler(q, handler));
				}

				if(to.IsCastableTo<ConnectionScriptable>()) {
					ref<ConnectionScriptable> conScriptable = to;
					if(conScriptable) {
						Log::Write(L"TJFabric/QueueGlobalScriptable", L"Has connection in SSend");
						ref<Connection> connection = conScriptable->GetConnection();
						if(connection) {
							connection->Send(m, replyHandler, conScriptable->GetConnectionChannel());
						}
						else {
							Throw(L"Cannot send() to null connection", ExceptionTypeError);
						}
					}
				}
				else {
					fe->Send(ScriptContext::GetValue<std::wstring>(to, L""), m, replyHandler);
				}
			}
		}
	}
	return ScriptConstants::Null;
}

ref<Scriptable> QueueGlobalScriptable::SPrint(ref<ParameterList> p) {
	static script::Parameter<std::wstring> PMessage(L"message", 0);
	std::wstring msg = PMessage.Require(p, L"");
	Log::Write(L"TJFabric/Script", msg);
	return ScriptConstants::Null;
}

ConnectionScriptable::ConnectionScriptable(ref<Connection> c, ref<ConnectionChannel> cc): _connection(c), _channel(cc) {
}

ConnectionScriptable::~ConnectionScriptable() {
}

void ConnectionScriptable::Initialize() {
	Bind(L"toString", &ConnectionScriptable::SToString);
}

ref<Scriptable> ConnectionScriptable::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[Connection]"));
}

ref<Connection> ConnectionScriptable::GetConnection() {
	return _connection;
}

ref<ConnectionChannel> ConnectionScriptable::GetConnectionChannel() {
	return _channel;
}