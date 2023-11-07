set(VST_SOURCES
	${EPS_SHARED_DIR}/binary_data.cpp
	${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.cpp
	
	PluginEditor.cpp
	PluginProcessor.cpp
	PluginProcessorUtils.cpp
	customised_ear_combo_box.cpp
	)

set(VST_HEADERS
	${EPS_SHARED_DIR}/binary_data.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/roboto.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/singleton.hpp
	${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.h
	${EPS_SHARED_DIR}/helper/nng_wrappers.h
	
	JuceHeader_Wrapper.h
	PluginEditor.h
	PluginProcessor.h
	PluginProcessorUtils.h
	customised_ear_combo_box.hpp
	)

source_group("Header Files" FILES ${VST_HEADERS})

add_juce_vst3_plugin(adm_export_source SOURCES ${VST_SOURCES} ${VST_HEADERS}
                                  DISPLAY_NAME "ADM Export Source"
                                  DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
                                  OUTPUT_NAME "ADM Export Source"
                                  IDE_FOLDER "EPS-AdmExportSourcePlugin"
                                  CODE_PREFIX "41444D" # Hex for "ADM"
                                  CODE_SUFFIX "65")    # Hex for "e"

target_link_libraries(adm_export_source_VST3 PRIVATE
	nng::nng
	adm
	ear-version
)


target_include_directories(adm_export_source_VST3
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/submodules/readerwriterqueue>
	$<TARGET_PROPERTY:adm,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(adm_export_source_VST3
    PUBLIC JUCE_DISABLE_ASSERTIONS)

install_juce_vst3_plugin(adm_export_source "${EPS_PLUGIN_INSTALL_PREFIX}")

include(DeployVST3)

set(DEP_SEARCH_PATHS "${PROJECT_BINARY_DIR}/lib" "${PROJECT_BINARY_DIR}/VST3" "${PROJECT_BINARY_DIR}/lib/*" "${PROJECT_BINARY_DIR}/VST3/*")
install_all_vst3_dependencies("${EPS_PLUGIN_INSTALL_PREFIX}" "${DEP_SEARCH_PATHS}")
