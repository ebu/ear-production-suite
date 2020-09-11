#ifdef WIN32
#if defined(DEBUG) || defined(_DEBUG)
#define DO_ALLOC_DEBUG
#endif
#endif

#ifdef DO_ALLOC_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define CRT_SET _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
                _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG); \
                _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG); \
                _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#else
#define CRT_SET
#endif