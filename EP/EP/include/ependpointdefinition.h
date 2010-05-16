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
 
 /* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_EP_EPENDPOINT_DEFINITION_H
#define _TJ_EP_EPENDPOINT_DEFINITION_H

#include "epinternal.h"
#include "epmessage.h"
#include "ependpoint.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
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
				virtual tj::shared::String GetDescription() const;
				virtual void  GetPaths(std::set<EPPath>& pathList) const;
				virtual void GetParameters(std::vector< tj::shared::ref<EPParameter> >& parameterList) const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual void Clone();
				virtual tj::shared::String GetEnabledBinding() const;
				
				virtual void SetID(const tj::shared::String& i);
				virtual void SetFriendlyName(const tj::shared::String& fn);
				virtual void SetDescription(const tj::shared::String& ds);
				virtual void AddPath(const EPPath& pt);
				virtual void AddParameter(tj::shared::ref<EPParameter> p);
				
			protected:
				tj::shared::String _id;
				tj::shared::String _friendlyName;
				std::set<EPPath> _paths;
				tj::shared::String _description;
				tj::shared::String _enabledBinding;
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
				EPParameterDefinition(const tj::shared::String& friendlyName, const tj::shared::String& type, const tj::shared::String& minValue, const tj::shared::String& maxValue, const tj::shared::String& defaultValue, EPParameter::Nature = EPParameter::NatureUnknown, const tj::shared::String& valueBinding=L"");
				virtual ~EPParameterDefinition();
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetType() const;
				virtual tj::shared::Any::Type GetValueType() const;
				virtual tj::shared::Any GetMinimumValue() const;
				virtual tj::shared::Any GetMaximumValue() const;
				virtual tj::shared::Any GetDefaultValue() const;;
				virtual void SetDefaultValue(const tj::shared::Any& val);
				virtual tj::shared::String GetValueBinding() const;
				virtual void SetValueBinding(const tj::shared::String& i);
				virtual EPParameter::Nature GetNature() const;
				virtual void Load(TiXmlElement* me);
				virtual void Save(TiXmlElement* me);
				virtual bool HasOptions() const;
				virtual void GetOptions(std::set< EPOption >& optionList) const;
				virtual void AddOption(const EPOption& epo);
				
			protected:
				tj::shared::String _friendlyName;
				tj::shared::String _type;
				tj::shared::String _minimumValue;
				tj::shared::String _maximumValue;
				tj::shared::String _defaultValue;
				tj::shared::String _valueBinding;
				tj::shared::Any _runtimeDefaultValue;
				std::set< EPOption > _options;
				EPParameter::Nature _nature;
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

#endif