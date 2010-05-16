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
 
 #ifndef _TJ_FABRIC_GROUP_H
#define _TJ_FABRIC_GROUP_H

#include <TJShared/include/tjshared.h>
#include <EP/include/ependpoint.h>
#include <EP/include/epconnection.h>
#include <EP/include/epdiscovery.h>

#include <map>

namespace tj {
	namespace fabric {
		class ConnectedGroup;
		
		class Group: public virtual tj::shared::Object, public tj::shared::Serializable {
			friend class ConnectedGroup;
			
			public:
				Group();
				virtual ~Group();
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* me);
				virtual void Clone();
				virtual void Clear();
				virtual tj::shared::String GetID() const;
				virtual tj::ep::Direction GetDirection() const;
				virtual tj::shared::String GetPrefix() const;
				virtual bool IsLazy() const;
				virtual bool PassesFilter(const tj::shared::String& path) const;
				virtual void GetTransports(std::vector< tj::shared::ref<tj::ep::EPTransport> >& transportsList) const;
				virtual bool GetDiscoveryScript(tj::shared::ref<tj::ep::DiscoveryDefinition> disco, tj::shared::String& scriptSource, bool appear) const;
			
			protected:
				tj::shared::String _id;
				tj::ep::Direction _direction;
				bool _lazy;
				std::deque< tj::shared::ref<tj::ep::ConnectionDefinition> > _connections;
				std::deque< tj::shared::ref<tj::ep::DiscoveryDefinition> > _discoveries;
				std::map< tj::shared::ref<tj::ep::DiscoveryDefinition>, tj::shared::String > _appearDiscoveryScripts;
				std::map< tj::shared::ref<tj::ep::DiscoveryDefinition>, tj::shared::String > _disappearDiscoveryScripts;
				std::deque< tj::shared::String > _filter;
				tj::shared::String _prefix;
		};
	}
}

#endif