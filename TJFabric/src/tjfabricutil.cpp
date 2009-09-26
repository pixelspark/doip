#include "../include/tjfabricutil.h"
#include "../../TJNP/include/tjpattern.h"
using namespace tj::shared;
using namespace tj::fabric;

bool Pattern::Matches(const String& p, const String& t) {
	return tj::np::pattern::Pattern::Match(p.c_str(), t.c_str());
}