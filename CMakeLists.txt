cmake_minimum_required(VERSION 3.10)
project(ear-production-combined VERSION 0.6.0.0 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

include(eps_set_install_paths)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    eps_set_install_paths()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)
option(BUILD_TESTING "Build tests" ON)
set(Boost_USE_STATIC_LIBS ON)
include("enable_msvc_static_runtime")
if(UNIX)
    find_package(Threads REQUIRED)
endif()
find_package(nng REQUIRED)
add_subdirectory(submodules)

add_subdirectory(ear-production-suite)
list(APPEND ear_production_suite_vsts
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
	scene_VST3)
foreach(tgt ${ear_production_suite_vsts})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER EPS-Plugins)
	endif()
endforeach()
list(APPEND ear_production_suite_vsts-tests
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
	variable_block_adapter_tests)
foreach(tgt ${ear_production_suite_vsts-tests})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER EPS-Plugins/tests)
	endif()
endforeach()
list(APPEND ear_production_suite_vsts-mockups
	input_plugin_mockup 
	monitoring_mockup 
	scene_mockup)
foreach(tgt ${ear_production_suite_vsts-mockups})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER EPS-Plugins/mockups)
	endif()
endforeach()

add_subdirectory(reaper-adm-extension)
list(APPEND reaper_extension
	reaper_adm
	reaper_adm_object)
foreach(tgt ${reaper_extension})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER EPS-ReaperExtension)
	endif()
endforeach()
list(APPEND reaper_extension-tests
	testacceptreaper_adm
	testreaper_adm)
foreach(tgt ${reaper_extension-tests})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER EPS-ReaperExtension/tests)
	endif()
endforeach()

add_subdirectory(reaper_adm_stem_plugin)
if(TARGET adm_export_source_VST3)
	set_target_properties(adm_export_source_VST3 PROPERTIES FOLDER EPS-AdmExportSourcePlugin)
endif()

add_subdirectory(tools)
list(APPEND tools
	proj_upgrade
	project_upgrade)
foreach(tgt ${tools})
	if(TARGET ${tgt})
		set_target_properties(${tgt} PROPERTIES FOLDER tools)
	endif()
endforeach()
if(TARGET test_project_upgrade)
	set_target_properties(test_project_upgrade PROPERTIES FOLDER tools/tests)
endif()

if(EPS_BUILD_PACKAGE)
    add_subdirectory(packaging)
endif()
