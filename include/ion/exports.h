#pragma once
#ifdef WIN32
#ifdef ION_EXPORTS
#define ION_API __declspec(dllexport)
#elif defined(ION_IMPORTS)
#define ION_API __declspec(dllimport)
#else
#define ION_API
#endif
#else
#define ION_API
#endif