
/*

// USEFUL FOR TRACING MEMORY LEAKS BY OBJECT ALLOCATION NUMBER

#ifdef WIN32
#include "win_mem_debug.h"
#endif

#include <crtdbg.h>

#ifdef _DEBUG

#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on

struct CrtBreakAllocSetter {
CrtBreakAllocSetter() {
_crtBreakAlloc = 169;
}
};

CrtBreakAllocSetter g_crtBreakAllocSetter;

#endif//_DEBUG

struct CrtBreakAllocSetter {
CrtBreakAllocSetter() {
CRT_SET
//_crtBreakAlloc = 169;
}
};

CrtBreakAllocSetter g_crtBreakAllocSetter;
*/


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