#pragma once

#ifdef WIN32
#if defined(DEBUG) || defined(_DEBUG)
#define DO_ALLOC_DEBUG
#endif
#endif

#ifdef DO_ALLOC_DEBUG

// _CRTDBG_MAP_ALLOC works by overriding calls to raw memory functions with macros that track the callers.
// However, JUCE has methods with these names internally.
// We need to make sure we don't override them with crtdbg.h macros as they have a different interface and therefore don't compile when overridden with the macros.
// JUCE also includes crtdbg.h itself, so we manually include crtdbg.h ourselves first with _CRTDBG_MAP_ALLOC pre-defined.

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#pragma push_macro("calloc")
#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#pragma push_macro("_recalloc")
#pragma push_macro("_aligned_free")
#pragma push_macro("_aligned_malloc")
#pragma push_macro("_aligned_offset_malloc")
#pragma push_macro("_aligned_realloc")
#pragma push_macro("_aligned_recalloc")
#pragma push_macro("_aligned_offset_realloc")
#pragma push_macro("_aligned_offset_recalloc")
#pragma push_macro("_aligned_msize")

#undef calloc
#undef free
#undef malloc
#undef realloc
#undef _recalloc
#undef _aligned_free
#undef _aligned_malloc
#undef _aligned_offset_malloc
#undef _aligned_realloc
#undef _aligned_recalloc
#undef _aligned_offset_realloc
#undef _aligned_offset_recalloc
#undef _aligned_msize

#include "JuceHeader.h"

#pragma pop_macro("_aligned_msize")
#pragma pop_macro("_aligned_offset_recalloc")
#pragma pop_macro("_aligned_offset_realloc")
#pragma pop_macro("_aligned_recalloc")
#pragma pop_macro("_aligned_realloc")
#pragma pop_macro("_aligned_offset_malloc")
#pragma pop_macro("_aligned_malloc")
#pragma pop_macro("_aligned_free")
#pragma pop_macro("_recalloc")
#pragma pop_macro("realloc")
#pragma pop_macro("malloc")
#pragma pop_macro("free")
#pragma pop_macro("calloc")

#define CRT_SET _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
                _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG); \
                _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG); \
                _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

#else
#include "JuceHeader.h"
#define CRT_SET
#endif
