cmake_minimum_required(VERSION 3.10)
set(EPS_VERSION_BASE 0.6.0) # This is used as fallback if getting from git fails. Must be only numeric.

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

include(eps_set_install_paths)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    eps_set_install_paths()
endif()

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

add_subdirectory(ear-production-suite-plugins)
add_subdirectory(reaper-adm-extension)
add_subdirectory(reaper-adm-export-source-plugin)
add_subdirectory(tools)

if(EPS_BUILD_PACKAGE)
    add_subdirectory(packaging)
endif()

message(STATUS "\n")
feature_summary(WHAT ALL)