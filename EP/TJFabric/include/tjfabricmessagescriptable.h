/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_FABRIC_MESSAGE_SCRIPTABLE_H
#define _TJ_FABRIC_MESSAGE_SCRIPTABLE_H

#include <EP/include/epmessage.h>
#include <TJScript/include/tjscript.h>

namespace tj {
	namespace fabric {
		class MessageScriptable: public tj::script::ScriptObject<MessageScriptable> {
			public:
				MessageScriptable(tj::shared::strong<tj::ep::Message> m);
				virtual ~MessageScriptable();
				static void Initialize();
				virtual tj::shared::ref<tj::script::Scriptable> SSetPath(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SPath(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SGet(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SSetParameter(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SToString(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SParameterTypes(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::ref<tj::script::Scriptable> SParameterCount(tj::shared::ref<tj::script::ParameterList> p);
				virtual tj::shared::strong<tj::ep::Message> GetMessage();
			
			protected:
				tj::shared::strong<tj::ep::Message> _message;
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