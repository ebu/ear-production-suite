#include <WDL/swell/swell-types.h>

// there are three ways to get at the SWELL symbols:
//
// - 1, SWELL_PROVIDED_BY_APP, SWELL_LOAD_SWELL_DYLIB, defined in swell-modstub-generic
//
//   swell symbols are looked up by dlopening libswell.so, assuming that the
//   parent application has loaded such a library itself
//
// - 2, SWELL_PROVIDED_BY_APP, no SWELL_LOAD_SWELL_DYLIB, defined in swell-modstub-generic
//
//   the application will call SWELL_dllMain with a function that can be used
//   to look up the symbols
//
// - 3, no SWELL_PROVIDED_BY_APP, defined in swell-appstub-generic
//
//   we compile the SWELL functions and provide them ourselves
//
// normally on linux we use method 1 (2 doesn't seem to work), but when testing
// libswell doesn't exist, although the rest of the code is linked against it,
// so the symbols will still exist. we could compile it ourselves, but we don't
// actually need to use any of the functions for testing the non-gui parts, so
// it's ok to enable method 2 -- swell symbols will still exist as function
// pointers, but will be left as NULL at run-time
#ifdef SWELL_TESTING
#undef SWELL_LOAD_SWELL_DYLIB
#endif

#include <WDL/swell/swell-modstub-generic.cpp>
