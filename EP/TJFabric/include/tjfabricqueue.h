#ifndef _TJ_FABRIC_QUEUE_H
#define _TJ_FABRIC_QUEUE_H

#include "../../../TJShared/include/tjshared.h"
#include "../../../TJScript/include/tjscript.h"
#include "../../EPFramework/include/epmessage.h"
#include "../../EPFramework/include/epconnection.h"

namespace tj {
	namespace script {
		class ScriptContext;
		class ScriptScope;
		class CompiledScript;
	}
}

namespace tj {
	namespace fabric {
		class QueueThread;
		class Rule;
		class FabricEngine;
		class Fabric;

		class QueuedMessage: public virtual tj::shared::Object {
			friend class Queue;
			friend class QueueThread;

			public:
				QueuedMessage(tj::shared::strong<tj::ep::Message> msg, tj::shared::ref<tj::ep::Connection> source, tj::shared::ref<tj::ep::ConnectionChannel> sourceChannel);
				virtual ~QueuedMessage();

			protected:
				tj::shared::strong<tj::ep::Message> _message;
				tj::shared::ref<tj::ep::Connection> _source;
				tj::shared::ref<tj::ep::ConnectionChannel> _sourceChannel;
		};

		class QueuedReply: public virtual tj::shared::Object {
			friend class Queue;
			friend class QueueThread;

			public:
				QueuedReply(tj::shared::strong<tj::ep::Message> org, tj::shared::strong<tj::ep::Message> reply, tj::shared::ref<tj::script::ScriptDelegate> dlg, tj::shared::strong<tj::ep::Connection> con, tj::shared::ref<tj::ep::ConnectionChannel> cc);
				virtual ~QueuedReply();

			protected:
				tj::shared::strong<tj::ep::Message> _originalMessage;
				tj::shared::strong<tj::ep::Message> _replyMessage;
				tj::shared::ref<tj::script::ScriptDelegate> _delegate;
				tj::shared::strong<tj::ep::Connection> _connection;
				tj::shared::ref<tj::ep::ConnectionChannel> _channel;
		};
		
		class Queue: public virtual tj::shared::Object {
			friend class QueueThread;
			friend class QueueGlobalScriptable;
			
			public:
				Queue(tj::shared::ref<FabricEngine> f);
				virtual ~Queue();
				virtual void OnCreated();
				virtual tj::shared::ref<tj::script::CompiledScript> GetScriptForRule(tj::shared::strong<Rule> r);
				virtual void ExecuteScript(tj::shared::strong<Rule> rule, tj::shared::strong<tj::script::CompiledScript> script, tj::shared::strong<QueuedMessage> m);
				virtual void Clear();
				virtual void Add(tj::shared::strong<tj::ep::Message> m, tj::shared::ref<tj::ep::Connection> source, tj::shared::ref<tj::ep::ConnectionChannel> sourceChannel);
				virtual void AddReply(tj::shared::strong<QueuedReply> m);
				virtual void WaitForCompletion();
				virtual void Stop();
			
			protected:
				virtual void SignalWorkAdded();
				virtual void ProcessMessage(tj::shared::strong<QueuedMessage> m);
				virtual void ProcessReply(tj::shared::strong<QueuedReply> r);

				tj::shared::CriticalSection _lock;
				tj::shared::ref<QueueThread> _thread;
				tj::shared::ref<tj::script::ScriptContext> _context;
				tj::shared::ref<tj::script::ScriptScope> _global;
				std::map< tj::shared::ref<Rule>, tj::shared::ref<tj::script::CompiledScript> > _scriptCache;
				std::deque< tj::shared::ref<QueuedMessage> > _queue;
				std::deque< tj::shared::ref<QueuedReply> > _replyQueue;
				tj::shared::weak<FabricEngine> _engine;
		};
		
		class QueueThread: public tj::shared::Thread {
			friend class Queue;
			
			public:
				QueueThread(tj::shared::ref<Queue> q);
				virtual ~QueueThread();
				virtual void Run();
				virtual void Stop();
				virtual void Start();
			
			protected:
				virtual void SignalWorkAdded();
			
				volatile bool _running;
				tj::shared::Event _signal;
				tj::shared::weak<Queue> _queue;
		};
	}
}

#endif