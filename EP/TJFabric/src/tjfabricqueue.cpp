#include "../include/tjfabricqueue.h"
#include <TJScript/include/tjscript.h>
#include "../include/tjfabricrule.h"
#include "../include/tjfabricmessagescriptable.h"
#include "../include/tjfabric.h"
#include "../include/tjfabricengine.h"
#include "../include/tjfabricscriptables.h"

using namespace tj::shared;
using namespace tj::fabric;
using namespace tj::script;
using namespace tj::ep;

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

void Queue::AddTimed(const Date& date, strong<Timed> t) {
	ThreadLock lock(&_lock);
	_timerQueue.insert(std::pair<Date, ref<Timed> >(date,t));
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
	_context->AddType(L"Date", GC::Hold(new DateScriptType()));
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
				f->GetAllMatchingRules(m, matchingRules);
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
			AddDeferredScriptCall(GC::Hold(new ScriptDelegate(cs, _context)), sc);
		}
	}
	catch(const ParserException& e) {
		Log::Write(L"TJFabric/Queue", std::wstring(L"Could not compile discovery script, parser exception: ")+e.GetMsg());
	}
}

void Queue::AddDeferredScriptCall(ref<ScriptDelegate> dlg, ref<ScriptScope> scope) {
	if(dlg) {
		ThreadLock lock(&_lock);
		_asyncScriptsQueue.push_back(ScriptCall(dlg,scope));
		SignalWorkAdded();
	}
}

void Queue::ProcessScriptCall(strong<ScriptDelegate> dlg, ref<ScriptScope> scope) {
	ThreadLock lock(&_lock);
	ref<ScriptContext> ctx = dlg->GetContext();
	if(ctx) {
		ref<ScriptScope> sc = GC::Hold(new ScriptScope());
		sc->Set(L"globals", _global);
		sc->SetPrevious(scope);
		Log::Write(L"TJFabric/Queue", L"Will process script call");
		ctx->Execute(dlg->GetScript(), sc);
	}
	else {
		Throw(L"No context set for asynchronous script delegate; script not executed", ExceptionTypeError);
	}
}

void Queue::ProcessTimed(ref<Timed> call) {
	ThreadLock lock(&_lock);
	if(call) {
		try {
			if(call->IsCancelled()) {
				Throw(L"Timer was cancelled!", ExceptionTypeError);
			}
			call->Run();
		}
		catch(const Exception& e) {
			Log::Write(L"TJFabric/Queue", std::wstring(L"Exception occurred while executing asynchronous script:") + e.GetMsg());
		}
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
	const static double KMaxWaitSeconds = 10;
	const static double KTimerPrecisionSeconds = 0.001;
	
	Log::Write(L"TJFabric/QueueThread", L"Queue processing thread started");
	
	while(_running) {
		_signal.Reset();
		if(!_running) {
			Log::Write(L"TJFabric/QueueThread", L"Thread was stopped");
			return;
		}
		
		AbsoluteDate nextTimer = 0.0;
		bool hasNextTimer = false;
		// Process queue
		{
			ref<Queue> q = _queue;
			if(!q) {
				Log::Write(L"TJFabric/QueueThread", L"Queue does not exist anymore, stopping thread");
				return;
			}
			
			ThreadLock lock(&(q->_lock));
			// Process timed items
			if(q->_timerQueue.size()>0) {
				Date now;
				std::multimap<Date,ref<Timed> >::iterator it = q->_timerQueue.begin();
				while(it!=q->_timerQueue.end()) {
					if((it->first).ToAbsoluteDate() < (now.ToAbsoluteDate()+KTimerPrecisionSeconds)) {
						// Run and delete
						Log::Write(L"TJFabric/Queue", L"Timer for "+Date(it->first.ToAbsoluteDate()).ToFriendlyString()+L" run at "+Date(now.ToAbsoluteDate()).ToFriendlyString());
						
						q->ProcessTimed(it->second);
						q->_timerQueue.erase(it++);
					}
					else {
						AbsoluteDate nd = it->first.ToAbsoluteDate();
						nextTimer = hasNextTimer ? Util::Min(nextTimer, nd) : nd;
						hasNextTimer = true;
						++it;
					}
				}
			}
			
			// Process asynchronous scripts
			if(q->_asyncScriptsQueue.size()>0) {
				std::deque<Queue::ScriptCall>::iterator it = q->_asyncScriptsQueue.begin();
				while(it!=q->_asyncScriptsQueue.end()) {
					Queue::ScriptCall& call = *it;
					if(call.first) {
						q->ProcessScriptCall(call.first, call.second);
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
		
		if(hasNextTimer) {
			Date now;
			AbsoluteDateInterval seconds = nextTimer - now.ToAbsoluteDate();
			int ms = Util::Max(int(KTimerPrecisionSeconds*1000.0), Util::Min(int(KMaxWaitSeconds*1000.0), int(seconds*1000.0)));
			//Log::Write(L"TJFabric/QueueThread", L"Next at "+Stringify(seconds)+L" Will wait "+Stringify(ms)+L"ms");
			_signal.Wait(ms);
		}
		else {
			_signal.Wait();
		}
	}
}

/** Timed **/
Timed::Timed(): _cancelled(false) {
}

Timed::~Timed() {
}

bool Timed::IsCancelled() const {
	return _cancelled;
}

void Timed::Cancel() {
	_cancelled = true;
}