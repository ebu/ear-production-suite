set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VST_SOURCES
	${EPS_SHARED_DIR}/helper/common_definition_helper.cpp
	
	PluginEditor.cpp
	PluginProcessor.cpp
	PluginProcessorUtils.cpp
	ear_theming/binary_data.cpp
	ear_theming/components/customised_ear_combo_box.cpp
	)

set(VST_HEADERS
	${EPS_SHARED_DIR}/helper/common_definition_helper.h
	${EPS_SHARED_DIR}/helper/nng_wrappers.h
	
	JuceHeader_Wrapper.h
	PluginEditor.h
	PluginProcessor.h
	PluginProcessorUtils.h
	ear_theming/binary_data.hpp
	ear_theming/components/customised_ear_combo_box.hpp
	ear_theming/components/look_and_feel/colours.hpp
	ear_theming/components/look_and_feel/fonts.hpp
	ear_theming/components/look_and_feel/roboto.hpp
	ear_theming/components/look_and_feel/shadows.hpp
	ear_theming/helper/graphics.hpp
	ear_theming/helper/singleton.hpp
	)

source_group("Header Files" FILES ${VST_HEADERS})

add_juce_vst3_plugin(adm_export_source SOURCES ${VST_SOURCES} ${VST_HEADERS}
                                  DISPLAY_NAME "ADM Export Source"
                                  DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
                                  OUTPUT_NAME "ADM Export Source"
                                  IDE_FOLDER "EPS-AdmExportSourcePlugin"
								  )


target_link_libraries(adm_export_source_VST3 PRIVATE
	nng::nng
	adm
)


target_include_directories(adm_export_source_VST3
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/submodules/readerwriterqueue>
	$<TARGET_PROPERTY:adm,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(adm_export_source_VST3
    PUBLIC JUCE_DISABLE_ASSERTIONS)

install_juce_vst3_plugin(adm_export_source "${EPS_PLUGIN_INSTALL_PREFIX}VST3")

include(DeployVST3)

set(DEP_SEARCH_PATHS "${PROJECT_BINARY_DIR}/lib" "${PROJECT_BINARY_DIR}/VST3" "${PROJECT_BINARY_DIR}/lib/*" "${PROJECT_BINARY_DIR}/VST3/*")
install_all_vst3_dependencies("${EPS_PLUGIN_INSTALL_PREFIX}VST3" "${DEP_SEARCH_PATHS}")
