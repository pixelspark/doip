#ifndef _TJFABRIC_RULE_H
#define _TJFABRIC_RULE_H

#include "../../TJShared/include/tjshared.h"

namespace tj {
	namespace fabric {
		class Rule: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				Rule();
				virtual ~Rule();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void SaveEndpointMethodDefinition(TiXmlElement* me);
				virtual void Clone();
				virtual bool Matches(const std::wstring& msg) const;
				virtual bool Matches(const std::wstring& msg, const std::wstring& tags) const;
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetScriptSource() const;
				virtual bool IsEnabled() const;
				virtual bool IsPublic() const;
				virtual tj::shared::String ToString() const;
			
			protected:
				tj::shared::String _id;
				std::set<tj::shared::String> _patterns;
				std::set<tj::shared::String> _parameterTags;
				std::wstring _script;
				bool _isEnabled;
				bool _isPublic;
				std::wstring _name;
		};
	}
}

#endif