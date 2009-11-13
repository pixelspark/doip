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
		
		class EP_EXPORTED EPSupportsCondition: public EPCondition {
			public:
				EPSupportsCondition();
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