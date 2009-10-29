#ifndef _TJ_BETWEEN_FAB_CONNECTION_H
#define _TJ_BETWEEN_FAB_CONNECTION_H

#include "../../../../TJShared/include/tjshared.h"
#include "../../../EPFramework/include/ependpoint.h"
#include "../../include/tjfabricconnection.h"
#include "../../include/tjfabricgroup.h"
#include "../../include/tjfabricengine.h"

namespace tj {
	namespace fabric {
		namespace connections {
			class BetweenConnectionDefinition: public tj::fabric::ConnectionDefinition {
				friend class BetweenConnection;
				
				public:
					BetweenConnectionDefinition();
					virtual ~BetweenConnectionDefinition();
					virtual void Save(TiXmlElement* me);
					virtual void Load(TiXmlElement* me);
					
				protected:
					std::wstring _pattern;
			};
			
			class BetweenConnection: public Connection {
				public:
					BetweenConnection();
					virtual ~BetweenConnection();
					virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<FabricEngine> fe);
					virtual void CreateForTransport(tj::shared::strong<tj::ep::EPTransport> ept, const tj::np::NetworkAddress& address);
				
					virtual void Send(tj::shared::strong<Message> msg);
				
				protected:
					std::wstring _pattern;
			};
		}
	}
}

#endif