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
		
		class EP_EXPORTED EPState {
			public:
				typedef std::map< tj::shared::String, tj::shared::Any > ValueMap;
				virtual ~EPState();
				virtual void SaveState(TiXmlElement* root);
				virtual void GetState(ValueMap& vals) = 0;
				virtual tj::shared::Any GetValue(const tj::shared::String& key) = 0;
		};
		
		class EP_EXPORTED EPStateDefinition {
			public:
				EPStateDefinition();
				virtual ~EPStateDefinition();
				virtual void GetState(EPState::ValueMap& vals);
				virtual tj::shared::Any GetValue(const tj::shared::String& key);
				virtual void SetState(EPState::ValueMap& vals);
				virtual void SetValue(const tj::shared::String& key, const tj::shared::Any& value);
			
			protected:
				EPState::ValueMap _vals;
		};
		
		typedef std::pair< tj::shared::String, tj::shared::Any > EPOption;

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
				enum Nature {
					NatureUnknown = 0,
					NatureDiscrete,
				};
			
				virtual ~EPParameter();
				virtual tj::shared::String GetFriendlyName() const = 0;
				virtual tj::shared::String GetType() const = 0;
				virtual tj::shared::String GetValueBinding() const = 0;
				virtual tj::shared::Any GetMinimumValue() const = 0;
				virtual tj::shared::Any GetMaximumValue() const = 0;
				virtual tj::shared::Any GetDefaultValue() const = 0;
				virtual void SetDefaultValue(const tj::shared::Any& val) = 0;
				virtual Nature GetNature() const = 0;
				virtual bool HasOptions() const = 0;
				virtual void GetOptions(std::set< EPOption >& optionList) const = 0;
			
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
				virtual tj::shared::String GetDescription() const = 0;
				virtual tj::shared::String GetEnabledBinding() const = 0;
			
				virtual void Save(TiXmlElement* me);
				virtual void GetReplies(std::vector< tj::shared::ref<EPReply> >& replyList) const;
				virtual bool Matches(const tj::shared::String& path) const;
				virtual bool Matches(const tj::shared::String& path, const tj::shared::String& ptags) const;
				virtual bool Matches(tj::shared::strong<Message> msg) const;
				virtual bool PersistDefaultValues(tj::shared::strong<Message> msg);
		};
		
		class EP_EXPORTED EPEndpoint: public virtual tj::shared::Object, public tj::shared::Sortable<EPEndpoint> {
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
				virtual bool SortsAfter(const EPEndpoint& o) const;
		};		
	}
}

#pragma warning(pop)
#endif