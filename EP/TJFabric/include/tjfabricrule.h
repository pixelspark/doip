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
 
 #ifndef _TJFABRIC_RULE_H
#define _TJFABRIC_RULE_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpoint.h>
#include <EP/include/ependpointdefinition.h>

namespace tj {
	namespace fabric {
		class Variable: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				Variable();
				virtual ~Variable();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void Clone();
				virtual tj::shared::String GetID() const;
				virtual tj::shared::Any GetDefaultValue() const;
			
			protected:
				tj::shared::String _id;
				tj::shared::Any _defaultValue;
		};
		
		class Rule: public virtual tj::shared::Object, public tj::ep::EPMethod, public tj::shared::Serializable {
			public:
				Rule();
				virtual ~Rule();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void SaveRule(TiXmlElement* me);
				virtual void Clone();
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetDescription() const;
				virtual tj::shared::String GetScriptSource() const;
				virtual bool IsEnabled() const;
				virtual bool IsPublic() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String ToString() const;
				virtual void GetPaths(std::set<tj::ep::EPPath>& pathList) const;
				virtual void GetParameters(std::vector< tj::shared::ref<tj::ep::EPParameter> >& parameterList) const;
				virtual void GetReplies(std::vector< tj::shared::ref<tj::ep::EPReply> >& replyList) const;
				virtual tj::shared::String GetEnabledBinding() const;
			
			protected:
				tj::shared::String _id;
				tj::shared::String _enabledBinding;
				std::set<tj::shared::String> _patterns;
				std::deque< tj::shared::ref<tj::ep::EPParameterDefinition> > _parameters;
				std::vector< tj::shared::ref<tj::ep::EPReply> > _replies;
				std::wstring _script;
				std::wstring _description;
				bool _isEnabled;
				bool _isPublic;
				std::wstring _name;
		};
	}
}

#endif