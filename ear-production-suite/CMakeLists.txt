cmake_minimum_required(VERSION 3.8)

project(ear-production-suite VERSION ${EPS_VERSION_BASE}
                             DESCRIPTION "AudioPlugins for next generation audio productions"
                             LANGUAGES CXX)
							 
set(CMAKE_PROJECT_VERSION ${EPS_VERSION_CURRENT})
message(STATUS "Project ear-production-suite version: " ${CMAKE_PROJECT_VERSION})

############################################################
# user config options
############################################################
include(FeatureSummary)
option(EAR_PLUGINS_UNIT_TESTS "Build units tests" ON)
option(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS "Build all monitoring plugins" ON)

option(JUCE_DISABLE_ASSERTIONS "Disable JUCE assertions (avoids discrete channels assert, but also others!!!)" ON)
if(JUCE_DISABLE_ASSERTIONS)
	add_compile_definitions(JUCE_DISABLE_ASSERTIONS)
endif()

find_package(JUCE REQUIRED)
find_package(VST3 REQUIRED)
find_package(Boost 1.66 REQUIRED)
find_package(nng REQUIRED)
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
