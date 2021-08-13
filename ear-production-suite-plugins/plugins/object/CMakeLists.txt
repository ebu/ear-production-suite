set(SOURCES_OBJECT_INPUT
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/slider.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.cpp
    ${EPS_SHARED_DIR}/components/ear_combo_box.cpp
    ${EPS_SHARED_DIR}/components/ear_header.cpp
    ${EPS_SHARED_DIR}/components/ear_slider_label.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
    src/object_frontend_connector.cpp
    src/object_plugin_editor.cpp
    src/object_plugin_processor.cpp
)

set(HEADERS_OBJECT_INPUT
	${EPS_PLUGIN_BASE_DIR}/include/communication/common_types.hpp
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/object_backend.hpp
	${EPS_PLUGIN_BASE_DIR}/include/ui/object_frontend_backend_connector.hpp
	
	${EPS_SHARED_DIR}/binary_data.hpp
	
	${EPS_SHARED_DIR}/components/ear_button.hpp
	${EPS_SHARED_DIR}/components/ear_combo_box.hpp
	${EPS_SHARED_DIR}/components/ear_header.hpp
	${EPS_SHARED_DIR}/components/ear_inverted_slider.hpp
	${EPS_SHARED_DIR}/components/ear_name_text_editor.hpp
	${EPS_SHARED_DIR}/components/ear_slider.hpp
	${EPS_SHARED_DIR}/components/ear_slider_label.hpp
	${EPS_SHARED_DIR}/components/level_meter.hpp
	${EPS_SHARED_DIR}/components/level_meter_calculator.hpp
	${EPS_SHARED_DIR}/components/non_automatable_parameter.hpp
	${EPS_SHARED_DIR}/components/onboarding.hpp
	${EPS_SHARED_DIR}/components/onboarding_slide.hpp
	${EPS_SHARED_DIR}/components/overlay.hpp
	${EPS_SHARED_DIR}/components/panner_side_view.hpp
	${EPS_SHARED_DIR}/components/panner_top_view.hpp
	${EPS_SHARED_DIR}/components/segment_progress_bar.hpp
	
	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/roboto.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp
	
	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/iso_lang_codes.hpp
	${EPS_SHARED_DIR}/helper/multi_async_updater.h
	${EPS_SHARED_DIR}/helper/properties_file.hpp
	
	src/object_component.hpp
	src/object_frontend_connector.hpp
	src/object_plugin_editor.hpp
	src/object_plugin_processor.hpp
	src/value_box_extent.hpp
	src/value_box_gain.hpp
	src/value_box_main.hpp
	src/value_box_metadata.hpp
	src/value_box_panning.hpp
	src/value_box_panning_view.hpp
)

source_group("Header Files" FILES ${HEADERS_OBJECT_INPUT})

# Input plugin code suffixes to start from 10
add_juce_vst3_plugin(object
  SOURCES ${SOURCES_OBJECT_INPUT} ${HEADERS_OBJECT_INPUT}
  IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  CODE_SUFFIX "11"
  DISPLAY_NAME "EAR Object"
  DESCRIPTION "The metadata input plugin for objects"
  OUTPUT_NAME "EAR Object")
  
target_include_directories(object_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(object_VST3 PRIVATE ear-plugin-base)
install_juce_vst3_plugin(object "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
