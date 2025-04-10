cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EPS_VERSION_BASE 1.2.0) # This is used as fallback if getting from git fails. Must be only numeric.
# NOTE: also update hard-coded version number in
# - vcpkg.json

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
include(get_git_version)
set(GIT_VERSION_BASE ${EPS_VERSION_BASE})
set(EPS_VERSION_CURRENT ${EPS_VERSION_BASE})
update_version_from_git(GIT_VERSION_BASE EPS_VERSION_CURRENT)
if(${GIT_VERSION_BASE} VERSION_GREATER ${EPS_VERSION_BASE} AND (NOT(EPS_PACKAGING)))
    message(WARNING "EPS_VERSION_BASE is lower than most recent git tag. Do you need to update it?")
endif()
set(EPS_VERSION_BASE ${GIT_VERSION_BASE})

project(ear-production-suite VERSION ${EPS_VERSION_BASE} LANGUAGES CXX)

message(STATUS "\n-----------------------------------------------------------------------------------")
message(STATUS "   EAR Production Suite Base Version:      " ${EPS_VERSION_BASE})
message(STATUS "   EAR Production Suite Current Version:   " ${EPS_VERSION_CURRENT})
message(STATUS "-----------------------------------------------------------------------------------\n")

include(FeatureSummary)

option(EPS_USE_BAREBONES_PROFILE "Use the bare-bones version of the EBU Production Profile" OFF)
if(EPS_USE_BAREBONES_PROFILE)
	add_compile_definitions(BAREBONESPROFILE)
endif()

include(eps_set_install_paths)
eps_set_install_paths()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)
option(BUILD_TESTING "Build tests" ON)
set(Boost_USE_STATIC_LIBS ON)
include(enable_msvc_static_runtime)
if(UNIX)
    find_package(Threads REQUIRED QUIET)
endif()
include(check_nng)
find_package(nng REQUIRED QUIET)
add_subdirectory(submodules)

set(EPS_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared)
set(JUCE_SUPPORT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shared/resources)
add_subdirectory(${EPS_SHARED_DIR}/version)

set(EPS_PLUGIN_TARGETS "" CACHE INTERNAL "")
add_subdirectory(ear-production-suite-plugins)
add_subdirectory(reaper-adm-extension)
add_subdirectory(reaper-adm-export-source-plugin)
option(EPS_BUILD_TOOLS "Build EPS project update tool" ON)
if(EPS_BUILD_TOOLS)
add_subdirectory(tools)
endif()

option(EPS_BUILD_PACKAGE "INSTALL supplementary files to form package" OFF)
if(EPS_BUILD_PACKAGE)
    add_subdirectory(packaging)
endif()

option(EPS_QUIET "Disable output of configuration summary" OFF)
if(NOT EPS_QUIET)
    message(STATUS "\n")
    feature_summary(WHAT ALL)
endif()
