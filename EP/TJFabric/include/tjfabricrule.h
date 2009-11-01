#ifndef _TJFABRIC_RULE_H
#define _TJFABRIC_RULE_H

#include "../../../TJShared/include/tjshared.h"
#include "../../EPFramework/include/ependpoint.h"

namespace tj {
	namespace fabric {
		class Parameter: public virtual tj::shared::Object, public tj::ep::EPParameter, public tj::shared::Serializable {
			public:
				Parameter();
				virtual ~Parameter();
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* me);
				virtual std::wstring GetFriendlyName() const;
				virtual std::wstring GetType() const;
				virtual tj::shared::Any GetMinimumValue() const;
				virtual tj::shared::Any GetMaximumValue() const;
				virtual tj::shared::Any GetDefaultValue() const;
				virtual tj::shared::Any::Type GetValueType() const;
				virtual wchar_t GetValueTypeTag() const;
			
				const static wchar_t* KTypeBoolean;
				const static wchar_t* KTypeInt32;
				const static wchar_t* KTypeDouble;
				const static wchar_t* KTypeString;
				const static wchar_t* KTypeNull;
			
			protected:
				tj::shared::String _friendly;
				tj::shared::String _type;
				tj::shared::String _min;
				tj::shared::String _max;
				tj::shared::String _default;
		};
		
		class Rule: public virtual tj::shared::Object, public tj::ep::EPMethod, public tj::shared::Serializable {
			public:
				Rule();
				virtual ~Rule();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void SaveRule(TiXmlElement* me);
				virtual void Clone();
				virtual bool Matches(const std::wstring& msg) const;
				virtual bool Matches(const std::wstring& msg, const std::wstring& tags) const;
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetScriptSource() const;
				virtual bool IsEnabled() const;
				virtual bool IsPublic() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String ToString() const;
				virtual void GetPaths(std::set<tj::ep::EPPath>& pathList) const;
				virtual void GetParameters(std::vector< tj::shared::ref<tj::ep::EPParameter> >& parameterList) const;
				virtual void GetReplies(std::vector< tj::shared::ref<tj::ep::EPReply> >& replyList) const;
			
			protected:
				tj::shared::String _id;
				std::set<tj::shared::String> _patterns;
				std::deque< tj::shared::ref<Parameter> > _parameters;
				std::vector< tj::shared::ref<tj::ep::EPReply> > _replies;
				std::wstring _script;
				bool _isEnabled;
				bool _isPublic;
				std::wstring _name;
		};
	}
}

#endif