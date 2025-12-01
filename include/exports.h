#pragma once
#ifdef WIN32
	#ifdef ION_EXPORTS
		#define ION_API __declspec(dllexport)
	#else
		#define ION_API __declspec(dllimport)
#endif
#else
#define ION_API
#endif