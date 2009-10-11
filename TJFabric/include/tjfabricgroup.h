#ifndef _TJ_FABRIC_GROUP_H
#define _TJ_FABRIC_GROUP_H

#include "../../TJShared/include/tjshared.h"

namespace tj {
	namespace fabric {
		class ConnectedGroup;
		
		enum Direction {
			DirectionNone = 0,
			DirectionInbound = 1,
			DirectionOutbound = 2,
			DirectionBoth = DirectionInbound | DirectionOutbound,
		};
		
		class ConnectionDefinition: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				virtual ~ConnectionDefinition();
				virtual std::wstring GetType() const;
				
			protected:
				ConnectionDefinition(const std::wstring& type);
				std::wstring _type;
		};
		
		class ConnectionDefinitionFactory: public tj::shared::PrototypeBasedFactory<ConnectionDefinition> {
			public:
				virtual ~ConnectionDefinitionFactory();
				virtual tj::shared::ref<ConnectionDefinition> Load(TiXmlElement* me);
				virtual void Save(tj::shared::strong<ConnectionDefinition> c, TiXmlElement* me);
				
				static tj::shared::strong<ConnectionDefinitionFactory> Instance();
				
			protected:
				ConnectionDefinitionFactory();
				static tj::shared::ref<ConnectionDefinitionFactory> _instance;
		};
		
		class DiscoveryDefinition: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				virtual ~DiscoveryDefinition();
				virtual std::wstring GetType() const;
				
			protected:
				DiscoveryDefinition(const std::wstring& type);
				std::wstring _type;
		};
		
		class DiscoveryDefinitionFactory: public tj::shared::PrototypeBasedFactory<DiscoveryDefinition> {
			public:
				virtual ~DiscoveryDefinitionFactory();
				virtual tj::shared::ref<DiscoveryDefinition> Load(TiXmlElement* me);
				virtual void Save(tj::shared::strong<DiscoveryDefinition> c, TiXmlElement* me);
				
				static tj::shared::strong<DiscoveryDefinitionFactory> Instance();
				
			protected:
				DiscoveryDefinitionFactory();
				static tj::shared::ref<DiscoveryDefinitionFactory> _instance;
		};
		
		class Group: public virtual tj::shared::Object, public tj::shared::Serializable {
			friend class ConnectedGroup;
			
			public:
				Group();
				virtual ~Group();
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* me);
				virtual void SaveEndpointDefinition(TiXmlElement* transports);
				virtual void Clone();
				virtual void Clear();
				virtual std::wstring GetID() const;
				virtual Direction GetDirection() const;
				virtual bool PassesFilter(const tj::shared::String& path) const;
			
			protected:
				std::wstring _id;
				Direction _direction;
				std::deque< tj::shared::ref<ConnectionDefinition> > _connections;
				std::deque< tj::shared::ref<DiscoveryDefinition> > _discoveries;
				std::deque< tj::shared::String > _filter;
		};
	}
}

#endif