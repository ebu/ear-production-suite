set(SOURCES_OBJECT_INPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_combo_box.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.cpp
    src/object_frontend_connector.cpp
    src/object_plugin_editor.cpp
    src/object_plugin_processor.cpp
)

set(HEADERS_OBJECT_INPUT
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/communication/common_types.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/detail/constants.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/object_backend.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/ui/object_frontend_backend_connector.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_button.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_combo_box.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_header.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_inverted_slider.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_name_text_editor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_label.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/non_automatable_parameter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/onboarding.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/onboarding_slide.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/overlay.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/panner_side_view.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/panner_top_view.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/segment_progress_bar.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/colours.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/fonts.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/roboto.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/shadows.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/graphics.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/iso_lang_codes.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/multi_async_updater.h
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/properties_file.hpp
	
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
