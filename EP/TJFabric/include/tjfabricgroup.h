#ifndef _TJ_FABRIC_GROUP_H
#define _TJ_FABRIC_GROUP_H

#include "../../../TJShared/include/tjshared.h"
#include "../../EPFramework/include/ependpoint.h"
#include "../../EPFramework/include/epconnection.h"
#include "../../EPFramework/include/epdiscovery.h"

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
				virtual bool GetDiscoveryScript(tj::shared::ref<tj::ep::DiscoveryDefinition> disco, tj::shared::String& scriptSource) const;
			
			protected:
				tj::shared::String _id;
				tj::ep::Direction _direction;
				bool _lazy;
				std::deque< tj::shared::ref<tj::ep::ConnectionDefinition> > _connections;
				std::deque< tj::shared::ref<tj::ep::DiscoveryDefinition> > _discoveries;
				std::map< tj::shared::ref<tj::ep::DiscoveryDefinition>, tj::shared::String > _discoveryScripts;
				std::deque< tj::shared::String > _filter;
				tj::shared::String _prefix;
		};
	}
}

#endif