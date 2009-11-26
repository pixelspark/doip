#ifndef _TJ_EP_EPENDPOINT_H
#define _TJ_EP_EPENDPOINT_H

#include "epinternal.h"
#include "epmessage.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		typedef tj::shared::String EPPath;
		typedef tj::shared::String EPTag;
		typedef int EPMediationLevel;
		
		enum EPMediationLevels {
			EPMediationLevelDefault = 0,
			EPMediationLevelIgnore = -1,
		};

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
				virtual tj::shared::Any GetMinimumValue() const = 0;
				virtual tj::shared::Any GetMaximumValue() const = 0;
				virtual tj::shared::Any GetDefaultValue() const = 0;
				virtual void SetDefaultValue(const tj::shared::Any& val) = 0;
				virtual bool IsDiscrete() const = 0;
			
				virtual bool IsValueValid(const tj::shared::Any& val) const;
				virtual void Save(TiXmlElement* me);
				virtual wchar_t GetValueTypeTag() const;
				virtual tj::shared::Any::Type GetValueType() const;
				
				const static wchar_t* KTypeBoolean;
				const static wchar_t* KTypeInt32;
				const static wchar_t* KTypeDouble;
				const static wchar_t* KTypeString;
				const static wchar_t* KTypeNull;
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
				virtual bool Matches(const tj::shared::String& path) const;
				virtual bool Matches(const tj::shared::String& path, const tj::shared::String& ptags) const;
				virtual bool Matches(tj::shared::strong<Message> msg) const;
		};
		
		class EP_EXPORTED EPEndpoint: public virtual tj::shared::Object {
			public:
				virtual ~EPEndpoint();
				virtual tj::shared::String GetID() const = 0;
				virtual tj::shared::String GetNamespace() const = 0;
				virtual tj::shared::String GetFriendlyName() const = 0;
				virtual tj::shared::String GetVersion() const = 0;
				virtual bool IsDynamic() const = 0;
				virtual EPMediationLevel GetMediationLevel() const = 0;
				virtual void GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const = 0;
				virtual void GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const = 0;
				virtual void GetTags(std::set<EPTag>& tagList) const;
			
				virtual void Save(TiXmlElement* me);
				virtual tj::shared::String GetFullIdentifier() const;
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
				virtual EPMediationLevel GetMediationLevel() const;
				virtual void GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const;
				virtual void GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const;
				virtual void GetTags(std::set<EPTag>& tagList) const;
			
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
			
			protected:
				tj::shared::String _id;
				tj::shared::String _namespace;
				tj::shared::String _friendlyName;
				tj::shared::String _version;
				EPMediationLevel _level;
				bool _dynamic;
				std::vector< tj::shared::ref<EPMethod> > _methods;
				std::vector< tj::shared::ref<EPTransport> > _transports;
				std::set<EPTag> _tags;
		};
		
		class EP_EXPORTED EPMethodDefinition: public EPMethod, public tj::shared::Serializable {
			public:
				EPMethodDefinition();
				EPMethodDefinition(const tj::shared::String& ids, const tj::shared::String& firstPath, const tj::shared::String& friendlyName);
				virtual ~EPMethodDefinition();
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual void  GetPaths(std::set<EPPath>& pathList) const;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void Clone();
			
				virtual void SetID(const tj::shared::String& i);
				virtual void SetFriendlyName(const tj::shared::String& fn);
				virtual void AddPath(const EPPath& pt);
				virtual void AddParameter(tj::shared::ref<EPParameter> p);
			
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
				EPParameterDefinition(const tj::shared::String& friendlyName, const tj::shared::String& type, const tj::shared::String& minValue, const tj::shared::String& maxValue, const tj::shared::String& defaultValue, bool discrete = false);
				virtual ~EPParameterDefinition();
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetType() const;
				virtual tj::shared::Any::Type GetValueType() const;
				virtual tj::shared::Any GetMinimumValue() const;
				virtual tj::shared::Any GetMaximumValue() const;
				virtual tj::shared::Any GetDefaultValue() const;
				virtual void SetDefaultValue(const tj::shared::Any& val);
				virtual bool IsDiscrete() const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
			
			protected:
				tj::shared::String _friendlyName;
				tj::shared::String _type;
				tj::shared::String _minimumValue;
				tj::shared::String _maximumValue;
				tj::shared::String _defaultValue;
				tj::shared::Any _runtimeDefaultValue;
				bool _discrete;
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