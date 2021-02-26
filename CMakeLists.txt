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

<<<<<<< HEAD
set(EPS_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared)
set(JUCE_SUPPORT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shared/resources)
=======
add_subdirectory(ear-production-suite)
set_target_properties(
	ear_logging_server
	ear-plugin-base
	direct_speakers_VST3 
	ear_binaural_monitoring_VST3
	ear_monitoring_0+2+0_VST3
	ear_monitoring_0+5+0_VST3
	ear_monitoring_0+7+0_VST3
	ear_monitoring_2+5+0_VST3
	ear_monitoring_3+7+0_VST3
	ear_monitoring_4+5+0_VST3
	ear_monitoring_4+5+1_VST3
	ear_monitoring_4+7+0_VST3
	ear_monitoring_4+9+0_VST3
	ear_monitoring_9+10+3_VST3
	object_VST3
	scene_VST3
PROPERTIES FOLDER EPS-ear_production_suite_vsts)
set_target_properties(
	connection_id_tests 
	connection_manager_tests 
	message_coding_tests 
	monitoring_audio_processor_tests 
	nng_tests 
	plugin_test_tools 
	programme_store_adm_populator_tests
	programme_store_adm_serializer_tests
	protobuf_tests
	scene_gains_calculator_tests
	scene_tests
	test_host_runner
	test_sender
	variable_block_adapter_tests
PROPERTIES FOLDER EPS-ear_production_suite_vsts-tests)
set_target_properties(
	input_plugin_mockup 
	monitoring_mockup 
	scene_mockup
PROPERTIES FOLDER EPS-ear_production_suite_vsts-mockups)
>>>>>>> d5c1010... Template for binaural plugin built from existing monitoring

add_subdirectory(ear-production-suite-plugins)
add_subdirectory(reaper-adm-extension)
add_subdirectory(reaper-adm-export-source-plugin)
add_subdirectory(tools)

if(EPS_BUILD_PACKAGE)
    add_subdirectory(packaging)
endif()

message(STATUS "\n")
feature_summary(WHAT ALL)