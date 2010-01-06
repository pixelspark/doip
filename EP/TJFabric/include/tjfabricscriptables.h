#ifndef _TJ_FABRIC_SCRIPTABLES_H
#define _TJ_FABRIC_SCRIPTABLES_H

#include <TJShared/include/tjshared.h>
#include <TJScript/include/tjscript.h>
#include <EP/include/epconnection.h>

namespace tj {
	namespace fabric {
		using namespace tj::shared;
		using namespace tj::script;
		using namespace tj::ep;
		class Queue;
		class Timed;
		
		class DateScriptable: public ScriptObject<DateScriptable> {
			public:
				DateScriptable();
				DateScriptable(const Date& d);
				virtual ~DateScriptable();
				virtual const Date& GetDate() const;
				static void Initialize();
			
				virtual ref<Scriptable> SMonth(ref<ParameterList> p);
				virtual ref<Scriptable> SDayOfMonth(ref<ParameterList> p);
				virtual ref<Scriptable> SYear(ref<ParameterList> p);
				virtual ref<Scriptable> SDayOfWeek(ref<ParameterList> p);
				virtual ref<Scriptable> SHour(ref<ParameterList> p);
				virtual ref<Scriptable> SMinute(ref<ParameterList> p);
				virtual ref<Scriptable> SSecond(ref<ParameterList> p);
				virtual ref<Scriptable> SAbsolute(ref<ParameterList> p);
				virtual ref<Scriptable> SToString(ref<ParameterList> p);
			
			protected:
				Date _date;
		};
		
		class DateScriptType: public ScriptType {
			public:
				DateScriptType();
				virtual ~DateScriptType();
				ref<Scriptable> Construct(ref<ParameterList> p);
		};
		
		
		class ConnectionScriptable: public ScriptObject<ConnectionScriptable> {
			public:
				ConnectionScriptable(tj::shared::ref<Connection> c, ref<ConnectionChannel> chan);
				virtual ~ConnectionScriptable();
				static void Initialize();
				virtual ref<Scriptable> SToString(ref<ParameterList> p);
				virtual ref<Connection> GetConnection();
				virtual ref<ConnectionChannel> GetConnectionChannel();
				
			protected:
				ref<tj::ep::Connection> _connection;
				ref<tj::ep::ConnectionChannel> _channel;
		};
		
		class TimedScriptable: public ScriptObject<TimedScriptable> {
			public:
				TimedScriptable(strong<Timed> t);
				virtual ~TimedScriptable();
				virtual ref<Scriptable> SCancel(ref<ParameterList> p);
				virtual ref<Scriptable> SToString(ref<ParameterList> p);
				virtual ref<Scriptable> SIsCancelled(ref<ParameterList> p);
				static void Initialize();
			
			protected:
				strong<Timed> _timed;
		};
		
		class QueueGlobalScriptable: public ScriptObject<QueueGlobalScriptable> {
			public:
				QueueGlobalScriptable(ref<Queue> q);
				virtual ~QueueGlobalScriptable();
				static void Initialize();
				virtual ref<Scriptable> SPrint(ref<ParameterList> p);
				virtual ref<Scriptable> SSend(ref<ParameterList> p);
				virtual ref<Scriptable> SSchedule(ref<ParameterList> p);
				
			protected:
				weak<Queue> _queue;
		};
		
	}
}


#endif