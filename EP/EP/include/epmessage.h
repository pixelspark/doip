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
 
 #ifndef _TJ_EP_MESSAGE_H
#define _TJ_EP_MESSAGE_H

#include "epinternal.h"

namespace tj {
	namespace ep {
		class EP_EXPORTED Message: public virtual tj::shared::Object, public tj::shared::Recycleable {
			public:
				Message(const tj::shared::String& path = L"");
				virtual ~Message();
				virtual void OnRecycle();
				virtual void OnReuse();
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