cmake_minimum_required(VERSION 3.8)

project(ear-production-suite VERSION 0.6.0
                             DESCRIPTION "AudioPlugins for next generation audio productions"
                             LANGUAGES CXX)

include("cmake/get_git_version.cmake")
update_version_from_git(CMAKE_PROJECT_VERSION)

message(STATUS "EAR Production Suite Version: " ${CMAKE_PROJECT_VERSION})
###########################################################
# set global paths
############################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

############################################################
# user config options
############################################################
include(FeatureSummary)
option(EAR_PLUGINS_UNIT_TESTS "Build units tests" ON)
option(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS "Build all monitoring plugins" ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND (NOT INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN))
    if(APPLE)
        set(INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN ON CACHE INTERNAL "Has default install prefix been overridden once?" FORCE)
        set(CMAKE_INSTALL_PREFIX "~/Library/Audio/Plug-Ins/VST3" CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
    endif()
    if(WIN32)
        set(INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN ON CACHE INTERNAL "Has default install prefix been overridden once?" FORCE)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/Common Files/VST3" CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
    endif()
endif()

option(JUCE_DISABLE_ASSERTIONS "Disable JUCE assertions (avoids discrete channels assert, but also others!!!)" ON)
if(JUCE_DISABLE_ASSERTIONS)
	add_compile_definitions(JUCE_DISABLE_ASSERTIONS)
endif()

find_package(JUCE REQUIRED)
find_package(VST3 REQUIRED)
find_package(Boost 1.66 REQUIRED)
find_package(nng REQUIRED)
include(cmake/check_nng.cmake)
check_nng_pipe_callback_signature()
set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

############################################################
# add targets
############################################################

configure_file(${PROJECT_SOURCE_DIR}/resources/config.h.in ${PROJECT_BINARY_DIR}/ear-plugin-base/config.h)

add_subdirectory(lib)
add_subdirectory(plugins)
add_subdirectory(tools)
add_subdirectory(packaging)
if(EAR_PLUGINS_UNIT_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

############################################################
# FeatureSummary
############################################################
add_feature_info(EAR_PLUGINS_UNIT_TESTS ${EAR_PLUGINS_UNIT_TESTS} "Build and run unit tests")
add_feature_info(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS ${EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS} "Build monitoring plugin for each speaker setup")
feature_summary(WHAT ALL)
