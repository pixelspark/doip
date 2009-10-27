#ifndef _TJ_FABRIC_MESSAGE_SCRIPTABLE_H
#define _TJ_FABRIC_MESSAGE_SCRIPTABLE_H

#include "tjfabricmessage.h"
#include "../../../TJScript/include/tjscript.h"

namespace tj {
	namespace fabric {
		class MessageScriptable: public tj::script::ScriptObject<MessageScriptable> {
			public:
				MessageScriptable(tj::shared::strong<Message> m);
				virtual ~MessageScriptable();
				static void Initialize();
				virtual tj::shared::ref<tj::script::Scriptable> SSetPath(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SPath(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SGet(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SSetParameter(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SToString(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SParameterTypes(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SParameterCount(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::strong<Message> GetMessage();
			
			protected:
				tj::shared::strong<Message> _message;
		};
		
		class MessageScriptType: public tj::script::ScriptType {
			public:
				MessageScriptType();
				virtual ~MessageScriptType();
				tj::shared::ref<tj::script::Scriptable> Construct(tj::shared::ref<tj::script::ParameterList> p);
		};
	}
}

#endif