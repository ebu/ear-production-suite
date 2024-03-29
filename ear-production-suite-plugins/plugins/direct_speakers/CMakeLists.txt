set(SOURCES_DIRECT_SPEAKERS_INPUT
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/slider.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.cpp
    ${EPS_SHARED_DIR}/components/ear_combo_box.cpp
    ${EPS_SHARED_DIR}/components/ear_slider_label.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
    ${EPS_SHARED_DIR}/components/speaker_layer.cpp
    ${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.cpp
    ${EPS_SHARED_DIR}/helper/cartesianspeakerlayouts.cpp
    src/direct_speakers_frontend_connector.cpp
    src/direct_speakers_plugin_editor.cpp
    src/direct_speakers_plugin_processor.cpp
)

set(HEADERS_DIRECT_SPEAKERS_INPUT
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/direct_speakers_backend.hpp
	
	${EPS_SHARED_DIR}/binary_data.hpp
	${EPS_SHARED_DIR}/speaker_setups.hpp
	
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
	${EPS_SHARED_DIR}/components/routing_info_icon.hpp
	${EPS_SHARED_DIR}/components/segment_progress_bar.hpp
	${EPS_SHARED_DIR}/components/speaker_layer.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp
	
	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp
	
	${EPS_SHARED_DIR}/helper/adm_preset_definitions_helper.h
	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/iso_lang_codes.hpp
	${EPS_SHARED_DIR}/helper/multi_async_updater.h
	${EPS_SHARED_DIR}/helper/properties_file.hpp

    src/direct_speakers_frontend_connector.hpp
    src/direct_speakers_plugin_editor.hpp
    src/direct_speakers_plugin_processor.hpp
    src/direct_speakers_component.hpp
		src/speaker_meters_box.hpp
		src/speaker_level.hpp
    src/value_box_main.hpp
    src/value_box_metadata.hpp
		src/channel_meter_layout.hpp
    src/value_box_speaker_layer.hpp
)

source_group("Header Files" FILES ${HEADERS_DIRECT_SPEAKERS_INPUT})

# Input plugin code suffixes to start from 10
add_juce_vst3_plugin(direct_speakers
  SOURCES ${SOURCES_DIRECT_SPEAKERS_INPUT} ${HEADERS_DIRECT_SPEAKERS_INPUT}
  IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  CODE_SUFFIX "10"
  DISPLAY_NAME "EAR DirectSpeakers" 
  DESCRIPTION "The metadata input plugin for channel-based audio"
  OUTPUT_NAME "EAR DirectSpeakers"
  IS_EPS_PLUGIN ON)

target_link_libraries(direct_speakers_VST3 PRIVATE ear-plugin-base ear-version)
target_compile_definitions(direct_speakers_VST3 PRIVATE USE_NEW_DESIGN)

install_juce_vst3_plugin(direct_speakers "${EPS_PLUGIN_INSTALL_PREFIX}ear-production-suite")
