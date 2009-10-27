#ifndef _TJ_FABRIC_QUEUE_H
#define _TJ_FABRIC_QUEUE_H

#include "../../../TJShared/include/tjshared.h"

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
		class Message;
		class FabricEngine;
		class Fabric;
		
		class Queue: public virtual tj::shared::Object {
			friend class QueueThread;
			friend class QueueGlobalScriptable;
			
			public:
				Queue(tj::shared::ref<FabricEngine> f);
				virtual ~Queue();
				virtual void OnCreated();
				virtual tj::shared::ref<tj::script::CompiledScript> GetScriptForRule(tj::shared::strong<Rule> r);
				virtual void ExecuteScript(tj::shared::strong<Rule> rule, tj::shared::strong<tj::script::CompiledScript> script, tj::shared::strong<Message> m);
				virtual void Clear();
				virtual void Add(tj::shared::strong<Message> m);
				virtual void WaitForCompletion();
				virtual void Stop();
			
			protected:
				virtual void ProcessMessage(tj::shared::strong<Message> m);
				tj::shared::CriticalSection _lock;
				tj::shared::ref<QueueThread> _thread;
				tj::shared::ref<tj::script::ScriptContext> _context;
				tj::shared::ref<tj::script::ScriptScope> _global;
				std::map< tj::shared::ref<Rule>, tj::shared::ref<tj::script::CompiledScript> > _scriptCache;
				std::deque< tj::shared::ref<Message> > _queue;
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