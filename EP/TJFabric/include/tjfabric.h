#ifndef _TJ_FABRIC_H
#define _TJ_FABRIC_H

#include "../../../TJShared/include/tjshared.h"

namespace tj {
	namespace fabric {
		class Rule;
		class Group;
		class FabricEngine;
		
		class Fabric: public virtual tj::shared::Object, public tj::shared::Serializable {
			friend class FabricEngine;
			
			public:
				Fabric();
				virtual ~Fabric();
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void SaveEndpointDefinition(TiXmlElement* me);
				virtual void Clone();
				virtual void Clear();
				virtual tj::shared::String GetTitle() const;
				virtual tj::shared::String GetAuthor() const;
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetPackage() const;
				virtual tj::shared::String GetFullIdentifier() const;
				virtual unsigned int GetVersion() const;
				virtual tj::shared::ref<Rule> GetFirstMatchingRule(const tj::shared::String& msg);
				virtual void GetAllMatchingRules(const tj::shared::String& path, const tj::shared::String& tags, std::deque< tj::shared::ref<Rule> >& results);
			
				static void LoadRecursive(const std::string& path, tj::shared::strong<Fabric> f);
			
			protected:
				tj::shared::CriticalSection _lock;
				tj::shared::String _id;
				tj::shared::String _author;
				tj::shared::String _title;
				tj::shared::String _package;
				unsigned int _version;
				std::deque< tj::shared::ref<Rule> > _rules;
				std::deque< tj::shared::ref<Group> > _groups;
		};
	}
}

#endif