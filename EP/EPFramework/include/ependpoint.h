#ifndef _TJ_EP_EPENDPOINT_H
#define _TJ_EP_EPENDPOINT_H

#include "internal/ep.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		typedef tj::shared::String EPPath;
		
		class EP_EXPORTED EPTransport: public virtual tj::shared::Object {
			public:
				virtual ~EPTransport();
				virtual tj::shared::String GetType() const = 0;
				virtual tj::shared::String GetAddress() const = 0;
				virtual tj::shared::String GetFormat() const = 0;
				virtual tj::shared::String GetFraming() const = 0;
				virtual unsigned short GetPort() const = 0;
				virtual void Save(TiXmlElement* me);
		};
		
		class EP_EXPORTED EPParameter: public virtual tj::shared::Object {
			public:
				virtual ~EPParameter();
				virtual tj::shared::String GetFriendlyName() const = 0;
				virtual tj::shared::String GetType() const = 0;
				virtual tj::shared::Any::Type GetValueType() const = 0;
				virtual tj::shared::Any GetMinimumValue() const = 0;
				virtual tj::shared::Any GetMaximumValue() const = 0;
				virtual tj::shared::Any GetDefaultValue() const = 0;
				virtual void Save(TiXmlElement* me);
		};
		
		class EXPORTED EPReply: public virtual tj::shared::Object {
			public:
				virtual ~EPReply();
				virtual tj::shared::String GetPath() const = 0;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const = 0;
				virtual void Save(TiXmlElement* me);
		};

		class EP_EXPORTED EPMethod: public virtual tj::shared::Object {
			public:
				virtual ~EPMethod();
				virtual tj::shared::String GetID() const = 0;
				virtual tj::shared::String GetFriendlyName() const = 0;
				virtual void GetPaths(std::set<EPPath>& pathList) const = 0;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const = 0;
				virtual void Save(TiXmlElement* me);
				virtual void GetReplies(std::vector< tj::shared::ref<EPReply> >& replyList) const;
		};
		
		class EP_EXPORTED EPEndpoint: public virtual tj::shared::Object {
			public:
				virtual ~EPEndpoint();
				virtual tj::shared::String GetID() const = 0;
				virtual tj::shared::String GetNamespace() const = 0;
				virtual tj::shared::String GetFullIdentifier() const = 0;
				virtual tj::shared::String GetFriendlyName() const = 0;
				virtual tj::shared::String GetVersion() const = 0;
				virtual bool IsDynamic() const = 0;
				virtual void GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const = 0;
				virtual void GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const = 0;
				virtual void Save(TiXmlElement* me);
		};		
		
		class EP_EXPORTED EPEndpointDefinition: public EPEndpoint, public tj::shared::Serializable {
			public:
				EPEndpointDefinition();
				virtual ~EPEndpointDefinition();
				virtual void Clone();
			
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetNamespace() const;
				virtual tj::shared::String GetFullIdentifier() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetVersion() const;
				virtual bool IsDynamic() const;
				virtual void GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const;
				virtual void GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
			
			protected:
				tj::shared::String _id;
				tj::shared::String _namespace;
				tj::shared::String _friendlyName;
				tj::shared::String _version;
				bool _dynamic;
				std::vector< tj::shared::ref<EPMethod> > _methods;
				std::vector< tj::shared::ref<EPTransport> > _transports;
		};
		
		class EP_EXPORTED EPMethodDefinition: public EPMethod, public tj::shared::Serializable {
			public:
				EPMethodDefinition();
				virtual ~EPMethodDefinition();
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual void  GetPaths(std::set<EPPath>& pathList) const;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void Clone();
			
			protected:
				tj::shared::String _id;
				tj::shared::String _friendlyName;
				std::set<EPPath> _paths;
				std::vector< tj::shared::ref<EPParameter> > _parameters;
		};

		class EP_EXPORTED EPReplyDefinition: public EPReply, public tj::shared::Serializable {
			public:
				EPReplyDefinition();
				virtual ~EPReplyDefinition();
				virtual tj::shared::String GetPath() const;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
			
			protected:
				tj::shared::String _path;
				std::vector< tj::shared::ref<EPParameter> > _parameters;
		};
		
		class EP_EXPORTED EPParameterDefinition: public EPParameter, public tj::shared::Serializable {
			public:
				EPParameterDefinition();
				virtual ~EPParameterDefinition();
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetType() const;
				virtual tj::shared::Any::Type GetValueType() const;
				virtual tj::shared::Any GetMinimumValue() const;
				virtual tj::shared::Any GetMaximumValue() const;
				virtual tj::shared::Any GetDefaultValue() const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
			
			protected:
				tj::shared::String _friendlyName;
				tj::shared::String _type;
				tj::shared::String _minimumValue;
				tj::shared::String _maximumValue;
				tj::shared::String _defaultValue;
		};
		
		class EP_EXPORTED EPTransportDefinition: public EPTransport, public tj::shared::Serializable {
			public:
				EPTransportDefinition();
				~EPTransportDefinition();
				virtual tj::shared::String GetType() const;
				virtual tj::shared::String GetAddress() const;
				virtual tj::shared::String GetFormat() const;
				virtual tj::shared::String GetFraming() const;
				virtual unsigned short GetPort() const;
				virtual void Save(TiXmlElement* me);
				virtual void Load(TiXmlElement* me);
			
			protected:
				tj::shared::String _type;
				tj::shared::String _address;
				tj::shared::String _format;
				tj::shared::String _framing;
				unsigned short _port;
		};
	}
}

#pragma warning(pop)
#endif