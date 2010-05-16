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
 
 #ifndef _TJ_FABRIC_H
#define _TJ_FABRIC_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpoint.h>

namespace tj {
	namespace fabric {
		class Rule;
		class Variable;
		class Group;
		class FabricEngine;
		
		class Fabric: public virtual tj::shared::Object, public tj::ep::EPEndpoint, public tj::shared::Serializable {
			friend class FabricEngine;
			
			public:
				Fabric();
				virtual ~Fabric();
				virtual void Load(TiXmlElement* me);
				virtual void SaveFabric(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void Clone();
				virtual void Clear();
				virtual tj::shared::String GetTitle() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetNamespace() const;
				virtual bool IsDynamic() const;
				virtual tj::shared::String GetAuthor() const;
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetPackage() const;
				virtual tj::shared::String GetVersion() const;
				virtual tj::ep::EPMediationLevel GetMediationLevel() const;
				virtual tj::shared::ref<tj::ep::EPStateDefinition> CreateDefaultState() const;
				virtual tj::shared::ref<Rule> GetFirstMatchingRule(const tj::shared::String& msg);
				virtual void GetAllMatchingRules(tj::shared::strong<tj::ep::Message> message, std::deque< tj::shared::ref<Rule> >& results);
				virtual void GetMethods(std::vector< tj::shared::ref<tj::ep::EPMethod> >& methodList) const;
				virtual void GetTransports(std::vector< tj::shared::ref<tj::ep::EPTransport> >& transportsList) const;
				virtual void GetTags(std::set<tj::ep::EPTag>& tagList) const;

				static void LoadRecursive(const std::string& path, tj::shared::strong<Fabric> f);
			
			protected:
				tj::shared::CriticalSection _lock;
				tj::shared::String _id;
				tj::shared::String _author;
				tj::shared::String _title;
				tj::shared::String _package;
				unsigned int _version;
				tj::ep::EPMediationLevel _mediationLevel;
				std::deque< tj::shared::ref<Rule> > _rules;
				std::deque< tj::shared::ref<Variable> > _variables;
				std::deque< tj::shared::ref<Group> > _groups;
				std::set<tj::ep::EPTag> _tags;
		};
	}
}

#endif