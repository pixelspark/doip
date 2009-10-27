#ifndef _TJ_OSC_UTIL_H
#define _TJ_OSC_UTIL_H

#include "../../../../TJShared/include/tjshared.h"
#include "../../../../Libraries/OSCPack/osc/OscReceivedElements.h"

namespace tj {
	namespace fabric {
		namespace connections {
			class OSCUtil {
				public:
					static bool ArgumentToAny(const osc::ReceivedMessageArgument& arg, tj::shared::Any& any);
			};
		}
	}
}

#endif