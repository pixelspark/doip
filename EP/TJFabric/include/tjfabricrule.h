#ifndef _TJFABRIC_RULE_H
#define _TJFABRIC_RULE_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpoint.h>

namespace tj {
	namespace fabric {
		class Rule: public virtual tj::shared::Object, public tj::ep::EPMethod, public tj::shared::Serializable {
			public:
				Rule();
				virtual ~Rule();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void SaveRule(TiXmlElement* me);
				virtual void Clone();
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
				std::deque< tj::shared::ref<tj::ep::EPParameterDefinition> > _parameters;
				std::vector< tj::shared::ref<tj::ep::EPReply> > _replies;
				std::wstring _script;
				bool _isEnabled;
				bool _isPublic;
				std::wstring _name;
		};
	}
}

#endif