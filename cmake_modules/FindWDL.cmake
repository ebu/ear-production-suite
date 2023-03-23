if(NOT (TARGET WDL::swell))

find_path(SWELL_INCLUDE_DIR
    WDL/swell/swell.h
    HINTS ${WDL_ROOT}
    ${CMAKE_SOURCE_DIR}/submodules/WDL
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(WDL
    DEFAULT_MSG
    SWELL_INCLUDE_DIR)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
   find_library(COCOA Cocoa)
endif()

set(wdl_sources
    $<$<PLATFORM_ID:Darwin>:${SWELL_INCLUDE_DIR}/WDL/swell/swell-modstub.mm>
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/wdl_linux_stub.cpp>
    )

add_library(WDL::swell INTERFACE IMPORTED)
set_target_properties(WDL::swell PROPERTIES
    INTERFACE_SOURCES "$<BUILD_INTERFACE:${wdl_sources}>"
    INTERFACE_INCLUDE_DIRECTORIES ${SWELL_INCLUDE_DIR}
    INTERFACE_COMPILE_DEFINITIONS "WDL_NO_DEFINE_MINMAX;SWELL_PROVIDED_BY_APP;$<$<PLATFORM_ID:Linux>:SWELL_LOAD_SWELL_DYLIB>"
    INTERFACE_LINK_LIBRARIES $<$<PLATFORM_ID:Darwin>:${COCOA}>)
endif()
