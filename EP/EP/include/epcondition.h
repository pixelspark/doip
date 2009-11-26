#ifndef _EP_CONDITION_H
#define _EP_CONDITION_H

#include "epinternal.h"
#include "ependpoint.h"

namespace tj {
	namespace ep {
		class EP_EXPORTED EPCondition: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				virtual ~EPCondition();
				virtual bool Matches(tj::shared::strong<EPEndpoint> ep) = 0;
		};
		
		class EP_EXPORTED EPLogicCondition: public EPCondition {
			enum EPLogicOp {
				EPLogicAnd = 1,
				EPLogicOr,
			};
			
			public:
				EPLogicCondition();
				virtual ~EPLogicCondition();
				virtual bool Matches(tj::shared::strong<EPEndpoint> ep);
				virtual void Save(TiXmlElement* parent);
				virtual void Load(TiXmlElement* you);
				virtual void AddCondition(tj::shared::strong<EPCondition> ep);
			
			protected:
				std::deque< tj::shared::ref<EPCondition> > _conditions;
				EPLogicOp _op;
		};
		
		class EP_EXPORTED EPHasTagCondition: public EPCondition {
			public:
				EPHasTagCondition(const EPTag& tag = L"");
				virtual ~EPHasTagCondition();
				virtual void Load(TiXmlElement* you);
				virtual void Save(TiXmlElement* you);
				virtual bool Matches(tj::shared::strong<EPEndpoint> ep);
			
			protected:
				EPTag _tag;
		};
		
		class EP_EXPORTED EPSpecificCondition: public EPCondition {
			public:
				EPSpecificCondition(const tj::shared::String& epid = L"", const tj::shared::String& nsp = L"");
				virtual ~EPSpecificCondition();
				virtual void Load(TiXmlElement* you);
				virtual void Save(TiXmlElement* me);
				virtual bool Matches(tj::shared::strong<EPEndpoint> ep);
			
			protected:
				tj::shared::String _epid;
				tj::shared::String _namespace;
		};
		
		class EP_EXPORTED EPSupportsCondition: public EPCondition {
			public:
				EPSupportsCondition();
				virtual ~EPSupportsCondition();
				virtual void Load(TiXmlElement* you);
				virtual void Save(TiXmlElement* parent);
				virtual bool Matches(tj::shared::strong<EPEndpoint> ep);
			
			protected:
				EPPath _pathPattern;
				EPPath _replyPattern;
		};
		
		class EP_EXPORTED EPConditionFactory {
			public:
				static tj::shared::ref<EPCondition> Load(TiXmlElement* root);
				static void Save(tj::shared::strong<EPCondition> ep, TiXmlElement* root);
		};
	}
}

#endif