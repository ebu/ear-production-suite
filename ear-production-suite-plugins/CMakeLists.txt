cmake_minimum_required(VERSION 3.8)

project(ear-production-suite-plugins VERSION ${EPS_VERSION_BASE}
                                     DESCRIPTION "AudioPlugins for next generation audio productions"
                                     LANGUAGES CXX)
							 
message(STATUS "Project ear-production-suite-plugins version: " ${CMAKE_PROJECT_VERSION})
set(IDE_FOLDER_PLUGINS "EPS-Plugins")
set(IDE_FOLDER_TESTS "EPS-Plugins/tests")

############################################################
# user config options
############################################################
option(EAR_PLUGINS_UNIT_TESTS "Build units tests" ON)
option(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS "Build all monitoring plugins" ON)

option(JUCE_DISABLE_ASSERTIONS "Disable JUCE assertions (avoids discrete channels assert, but also others!!!)" ON)
if(JUCE_DISABLE_ASSERTIONS)
	add_compile_definitions(JUCE_DISABLE_ASSERTIONS)
endif()

find_package(JUCE REQUIRED QUIET)
find_package(Boost 1.66 REQUIRED QUIET)
find_package(nng REQUIRED QUIET)
check_nng_pipe_callback_signature()
set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED QUIET)
find_package(spdlog REQUIRED QUIET)


############################################################
# add targets
############################################################

configure_file(${JUCE_SUPPORT_RESOURCES}/config.h.in ${PROJECT_BINARY_DIR}/ear-plugin-base/config.h)
set(EPS_PLUGIN_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(MSVC)
  add_compile_options(/bigobj)
endif()

if(DEFINED EPS_ENABLE_LOGGING)
    if(EPS_ENABLE_LOGGING)
        add_compile_definitions(EPS_ENABLE_LOGGING)
    endif()
else()
    add_compile_definitions($<$<CONFIG:Debug,RelWithDebInfo>:EPS_ENABLE_LOGGING>)
endif()

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
add_feature_info(EPS_USE_BAREBONES_PROFILE ${EPS_USE_BAREBONES_PROFILE} "Use only bare-bones profile")
add_feature_info(EAR_PLUGINS_UNIT_TESTS ${EAR_PLUGINS_UNIT_TESTS} "Build and run unit tests")
add_feature_info(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS ${EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS} "Build monitoring plugin for each speaker setup")
