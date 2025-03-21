set(SOURCES_HOA_INPUT
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/slider.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.cpp
    ${EPS_SHARED_DIR}/components/ear_combo_box.cpp
    ${EPS_SHARED_DIR}/components/ear_slider_label.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
    ${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.cpp
    ${EPS_SHARED_DIR}/helper/cartesianspeakerlayouts.cpp
    src/hoa_frontend_connector.cpp
    src/hoa_plugin_editor.cpp
    src/hoa_plugin_processor.cpp
    src/order_box.cpp
    src/order_display_box.cpp
    src/value_box_order_display.cpp
    src/pyramid_box.cpp
)

set(HEADERS_HOA_INPUT
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/hoa_backend.hpp
	
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
    ${EPS_SHARED_DIR}/components/routing_info_icon.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp
	
	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp
	
	${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.h
	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/multi_async_updater.h
	${EPS_SHARED_DIR}/helper/properties_file.hpp
	
	src/hoa_component.hpp
	src/hoa_frontend_connector.hpp
	src/hoa_plugin_editor.hpp
	src/hoa_plugin_processor.hpp
	src/value_box_order_display.hpp
	src/order_box.hpp
	src/order_display_box.hpp
	src/pyramid_box.hpp
	src/value_box_main.hpp
)

source_group("Header Files" FILES ${HEADERS_HOA_INPUT})

# Input plugin code suffixes to start from 10
add_juce_vst3_plugin(hoa
  SOURCES ${SOURCES_HOA_INPUT} ${HEADERS_HOA_INPUT}
  IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  CODE_SUFFIX "12"
  DISPLAY_NAME "EAR HOA" 
  DESCRIPTION "The metadata input plugin for scene-based audio"
  OUTPUT_NAME "EAR HOA"
  IS_EPS_PLUGIN ON)

target_link_libraries(hoa_VST3 PRIVATE ear-plugin-base ear-version)

install_juce_vst3_plugin(hoa "${EPS_PLUGIN_INSTALL_PREFIX}ear-production-suite")
