#ifndef _TJ_FABRIC_QUEUE_H
#define _TJ_FABRIC_QUEUE_H

#include <TJShared/include/tjshared.h>
#include <TJScript/include/tjscript.h>
#include <EP/include/epmessage.h>
#include <EP/include/epconnection.h>
#include <EP/include/epdiscovery.h>

namespace tj {
	namespace fabric {
		using namespace tj::shared;
		using namespace tj::script;
		using namespace tj::ep;
		class QueueThread;
		class Rule;
		class FabricEngine;
		class Fabric;
		
		class Timed: public virtual Object {
			public:
				virtual ~Timed();
				virtual void Run() = 0;
				virtual bool IsCancelled() const;
				virtual void Cancel();
			
			protected:
				Timed();
				bool _cancelled;
		};

		class QueuedMessage: public virtual Object {
			friend class Queue;
			friend class QueueThread;

			public:
				QueuedMessage(strong<Message> msg, ref<Connection> source, ref<ConnectionChannel> sourceChannel);
				virtual ~QueuedMessage();

			protected:
				strong<Message> _message;
				ref<Connection> _source;
				ref<ConnectionChannel> _sourceChannel;
		};

		class QueuedReply: public virtual Object {
			friend class Queue;
			friend class QueueThread;

			public:
				QueuedReply(strong<Message> org, strong<Message> reply, ref<ScriptDelegate> dlg, strong<Connection> con, ref<ConnectionChannel> cc);
				virtual ~QueuedReply();

			protected:
				strong<Message> _originalMessage;
				strong<Message> _replyMessage;
				ref<ScriptDelegate> _delegate;
				strong<Connection> _connection;
				ref<ConnectionChannel> _channel;
		};
		
		class Queue: public virtual Object {
			friend class QueueThread;
			friend class QueueGlobalScriptable;
			friend class TimedScriptExecution;
			typedef std::pair< ref<ScriptDelegate>, ref<ScriptScope> > ScriptCall;
			
			public:
				Queue(ref<FabricEngine> f);
				virtual ~Queue();
				virtual void OnCreated();
				virtual ref<CompiledScript> GetScriptForRule(strong<Rule> r);
				virtual void ExecuteScript(strong<Rule> rule, strong<CompiledScript> script, strong<QueuedMessage> m);
				virtual void Clear();
				virtual void Add(strong<Message> m, ref<Connection> source, ref<ConnectionChannel> sourceChannel);
				virtual void AddReply(strong<QueuedReply> m);
				virtual void AddDiscoveryScriptCall(ref<DiscoveryDefinition> def, ref<Connection> connection, const String& source);
				virtual void AddTimed(const Date& at, strong<Timed> t);
				virtual void AddDeferredScriptCall(ref<ScriptDelegate> dlg, ref<ScriptScope> sc);
			
				virtual void WaitForCompletion();
				virtual void Stop();
			
			protected:
				virtual void SignalWorkAdded();
				virtual void ProcessMessage(strong<QueuedMessage> m);
				virtual void ProcessReply(strong<QueuedReply> r);
				virtual void ProcessTimed(ref<Timed> t);
				virtual void ProcessScriptCall(strong<ScriptDelegate> dlg, ref<ScriptScope> scope);
				

				CriticalSection _lock;
				ref<QueueThread> _thread;
				ref<ScriptContext> _context;
				ref<ScriptScope> _global;
				std::map< ref<Rule>, ref<CompiledScript> > _scriptCache;
				weak<FabricEngine> _engine;
			
				std::deque< ref<QueuedMessage> > _queue;
				std::deque< ref<QueuedReply> > _replyQueue;
				std::deque<ScriptCall> _asyncScriptsQueue;
				std::multimap<tj::shared::Date, ref<Timed> > _timerQueue;
			
				unsigned int _messageCount;
				tj::shared::Date _runningSince;
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
		
		class QueueThread: public Thread {
			friend class Queue;
			
			public:
				QueueThread(ref<Queue> q);
				virtual ~QueueThread();
				virtual void Run();
				virtual void Stop();
				virtual void Start();
			
			protected:
				virtual void SignalWorkAdded();
			
				volatile bool _running;
				Event _signal;
				weak<Queue> _queue;
		};
	}
}

#endif