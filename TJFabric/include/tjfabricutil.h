#ifndef _TJ_FABRIC_UTIL_H
#define _TJ_FABRIC_UTIL_H

#include "../../TJShared/include/tjshared.h"

namespace tj {
	namespace fabric {
		class Pattern {
			public:
				static bool Matches(const tj::shared::String& pattern, const tj::shared::String& test);
		};
		
	}
}

#endif