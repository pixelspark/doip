#ifndef _TJ_EP_MESSAGE_H
#define _TJ_EP_MESSAGE_H

#include "internal/ep.h"

namespace tj {
	namespace ep {
		class EP_EXPORTED Message: public virtual tj::shared::Object {
			public:
				Message(const tj::shared::String& path);
				virtual ~Message();
				virtual const tj::shared::String& GetPath() const;
				virtual void SetPath(const tj::shared::String& m);
				virtual void SetParameter(unsigned int i, const tj::shared::Any& d);
				virtual tj::shared::Any GetParameter(unsigned int i) const;
				virtual unsigned int GetParameterCount() const;
				virtual tj::shared::String ToString() const;
				virtual tj::shared::String GetParameterTags() const;
				
			protected:
				tj::shared::String _path;
				std::map<unsigned int, tj::shared::Any > _parameters;
		};
	}
}

#endif