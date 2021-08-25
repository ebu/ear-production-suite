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
set(EPS_VERSIONING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared/version)

add_custom_target(ear-version-generator ALL
	WORKING_DIRECTORY ${EPS_VERSIONING_DIR}
	COMMAND ${CMAKE_COMMAND} -P ${EPS_VERSIONING_DIR}/gen_version_defines.cmake
	)
	
add_library(ear-version STATIC
	${EPS_VERSIONING_DIR}/eps_version.cpp
	${EPS_VERSIONING_DIR}/eps_version_autogen_defines.h
	${EPS_VERSIONING_DIR}/eps_version.h)
add_dependencies(ear-version ear-version-generator)

set_target_properties(ear-version PROPERTIES FOLDER versioning)
set_target_properties(ear-version-generator PROPERTIES FOLDER versioning)

add_subdirectory(ear-production-suite-plugins)
add_subdirectory(reaper-adm-extension)
add_subdirectory(reaper-adm-export-source-plugin)
add_subdirectory(tools)

if(EPS_BUILD_PACKAGE)
    add_subdirectory(packaging)
endif()

message(STATUS "\n")
feature_summary(WHAT ALL)