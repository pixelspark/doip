#include "../include/tjfabricscriptables.h"
#include "../include/tjfabricqueue.h"
#include "../include/tjfabricmessagescriptable.h"
#include "../include/tjfabricengine.h"
using namespace tj::shared;
using namespace tj::fabric;

/** DateScriptType **/
DateScriptType::DateScriptType() {
}

DateScriptType::~DateScriptType() {
}

ref<Scriptable> DateScriptType::Construct(ref<ParameterList> p) {
	ref<DateScriptable> ds;
	
	if(p->Exists(L"day") && p->Exists(L"month") && p->Exists(L"year")) {
		if(p->Exists(L"hours") && p->Exists(L"minutes") && p->Exists(L"seconds")) {
			Date date(ScriptContext::GetValue<int>(p->Get(L"year"),0), ScriptContext::GetValue<int>(p->Get(L"month"),0), ScriptContext::GetValue<int>(p->Get(L"day"),0), ScriptContext::GetValue<int>(p->Get(L"hours"),0), ScriptContext::GetValue<int>(p->Get(L"minutes"),0), ScriptContext::GetValue<int>(p->Get(L"seconds"),0));
			ds = GC::Hold(new DateScriptable(date));
		}
		else {
			Date date(ScriptContext::GetValue<int>(p->Get(L"year"),0), ScriptContext::GetValue<int>(p->Get(L"month"),0), ScriptContext::GetValue<int>(p->Get(L"day"),0), 0, 0, 0);
			ds = GC::Hold(new DateScriptable(date));
		}
	}
	else if(p->Exists(L"absolute") || p->Exists(L"0")) {
		ref<Scriptable> absolute = p->Get(L"absolute");
		if(!absolute) {
			absolute = p->Get(L"0");
		}
		
		ds = GC::Hold(new DateScriptable(Date((AbsoluteDate)ScriptContext::GetValue<double>(absolute, 0.0))));
	}
	else {
		ds = GC::Hold(new DateScriptable());
	}
	return ds;
}

/** DateScriptable **/
DateScriptable::DateScriptable() {
}

DateScriptable::DateScriptable(const Date& d): _date(d) {
}

DateScriptable::~DateScriptable() {
}

void DateScriptable::Initialize() {
	Bind(L"month", &DateScriptable::SMonth);
	Bind(L"dayOfMonth", &DateScriptable::SDayOfMonth);
	Bind(L"dayOfWeek", &DateScriptable::SDayOfWeek);
	Bind(L"year", &DateScriptable::SYear);
	Bind(L"second", &DateScriptable::SSecond);
	Bind(L"hour", &DateScriptable::SHour);
	Bind(L"minute", &DateScriptable::SMinute);
	Bind(L"absolute", &DateScriptable::SAbsolute);
}

const Date& DateScriptable::GetDate() const {
	return _date;
}

ref<Scriptable> DateScriptable::SAbsolute(ref<ParameterList> p) {
	return GC::Hold(new ScriptDouble(_date.ToAbsoluteDate()));
}

ref<Scriptable> DateScriptable::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(_date.ToFriendlyString()));
}

ref<Scriptable> DateScriptable::SMonth(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetMonth()));
}
ref<Scriptable> DateScriptable::SDayOfMonth(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetDayOfMonth()));
}
ref<Scriptable> DateScriptable::SYear(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetYear()));
}
ref<Scriptable> DateScriptable::SDayOfWeek(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetDayOfWeek()));
}

ref<Scriptable> DateScriptable::SHour(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetHours()));
}

ref<Scriptable> DateScriptable::SMinute(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetMinutes()));
}

ref<Scriptable> DateScriptable::SSecond(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_date.GetSeconds()));
}

/** TimedScriptExecution **/
namespace tj {
	namespace fabric {
		class TimedScriptExecution: public virtual Object, public virtual Timed {
			public:
				TimedScriptExecution(ref<Queue> queue, strong<ScriptDelegate> dlg, ref<ScriptScope> scope): _delegate(dlg), _scope(scope), _queue(queue) {
				}
			
				virtual ~TimedScriptExecution() {
				}
			
				virtual void Run() {
					ref<Queue> q = _queue;
					if(q) {
						q->ProcessScriptCall(_delegate, _scope);
					}
					else {
						Throw(L"TimedScriptExecution without Queue!", ExceptionTypeError);
					}
				}	
			
			protected:
				weak<Queue> _queue;
				strong<ScriptDelegate> _delegate;
				ref<ScriptScope> _scope;
		};
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
	Bind(L"schedule", &QueueGlobalScriptable::SSchedule);
}

ref<Scriptable> QueueGlobalScriptable::SSchedule(ref<ParameterList> p) {
	ref<Scriptable> date = p->Get(L"0");
	ref<Scriptable> dlg = p->Get(L"1");
	
	if(!date) {
		date = p->Get(L"at");
	}
	
	if(!dlg) {
		dlg = p->Get(L"call");
	}
	
	if(date && dlg) {
		if(!date.IsCastableTo<DateScriptable>()) {
			Throw(L"Date parameter is not a Date object", ExceptionTypeError);
		}
		if(!dlg.IsCastableTo<ScriptDelegate>()) {
			Throw(L"Call parameter is not a ScriptDelegate", ExceptionTypeError);
		}
		
		ref<Queue> q = _queue;
		if(q) {
			ref<TimedScriptExecution> tse = GC::Hold(new TimedScriptExecution(q,ref<ScriptDelegate>(dlg), p));
			q->AddTimed(ref<DateScriptable>(date)->GetDate(), ref<Timed>(tse));
			return GC::Hold(new TimedScriptable(ref<Timed>(tse)));
		}
		return ScriptConstants::Null;
	}
	Throw(L"Invalid arguments to schedule(at,call,..); should be a date and a delegate!", ExceptionTypeError);
}

ref<Scriptable> QueueGlobalScriptable::SDefer(ref<ParameterList> p) {
	ref<Scriptable> dlg = p->Get(L"0");
	
	if(!dlg) {
		dlg = p->Get(L"call");
	}
	
	if(dlg) {
		if(!dlg.IsCastableTo<ScriptDelegate>()) {
			Throw(L"Call parameter is not a ScriptDelegate", ExceptionTypeError);
		}
		
		ref<Queue> q = _queue;
		if(q) {
			q->AddDeferredScriptCall(ref<ScriptDelegate>(dlg), p);
		}
		return ScriptConstants::Null;
	}
	Throw(L"Invalid arguments to defer(call,...); should be a delegate!", ExceptionTypeError);
}

ref<Scriptable> QueueGlobalScriptable::SSend(ref<ParameterList> p) {	
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

/** TimedScriptable **/
TimedScriptable::TimedScriptable(strong<Timed> t): _timed(t) {
}

TimedScriptable::~TimedScriptable() {
}

ref<Scriptable> TimedScriptable::SCancel(ref<ParameterList> p) {
	_timed->Cancel();
	return ScriptConstants::Null;
}	

ref<Scriptable> TimedScriptable::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[Timed item]"));
}

ref<Scriptable> TimedScriptable::SIsCancelled(ref<ParameterList> p) {
	return _timed->IsCancelled() ? ScriptConstants::True : ScriptConstants::False;
}

void TimedScriptable::Initialize() {
	Bind(L"cancel", &TimedScriptable::SCancel);
	Bind(L"isCancelled", &TimedScriptable::SIsCancelled);
	Bind(L"toString", &TimedScriptable::SToString);
}