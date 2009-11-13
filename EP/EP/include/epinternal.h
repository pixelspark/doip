#ifndef _TJ_EP_INTERNAL_H
#define _TJ_EP_INTERNAL_H
	#include <TJShared/include/tjshared.h>

	#ifdef EPFRAMEWORK_EXPORTS
		#ifdef TJ_OS_WIN
			#define EP_EXPORTED __declspec(dllexport)
		#else
			#define EP_EXPORTED __attribute__ ((visibility("default")))
		#endif
	#else
		#ifdef TJ_OS_WIN
			#define EP_EXPORTED __declspec(dllimport)
		#else
			#define EP_EXPORTED
		#endif
	#endif
#endif