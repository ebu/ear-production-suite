set(SOURCES_DIRECT_SPEAKERS_INPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_combo_box.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.cpp
    src/direct_speakers_frontend_connector.cpp
    src/direct_speakers_plugin_editor.cpp
    src/direct_speakers_plugin_processor.cpp
)

set(HEADERS_DIRECT_SPEAKERS_INPUT
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/detail/constants.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/direct_speakers_backend.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/speaker_setups.hpp
	
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
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/segment_progress_bar.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/speaker_layer.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/colours.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/fonts.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/shadows.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/graphics.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/iso_lang_codes.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/multi_async_updater.h
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/properties_file.hpp

    src/direct_speakers_frontend_connector.hpp
    src/direct_speakers_plugin_editor.hpp
    src/direct_speakers_plugin_processor.hpp
    src/direct_speakers_component.hpp
	src/channel_gains_box.hpp
	src/channel_gain.hpp
	src/value_box_main.hpp
	src/value_box_metadata.hpp
	src/value_box_channel_gain.hpp
	src/value_box_speaker_layer.hpp
)

source_group("Header Files" FILES ${HEADERS_DIRECT_SPEAKERS_INPUT})

# Input plugin code suffixes to start from 10
add_juce_vst3_plugin(direct_speakers
  SOURCES ${SOURCES_DIRECT_SPEAKERS_INPUT} ${HEADERS_DIRECT_SPEAKERS_INPUT}
  CODE_SUFFIX "10"
  DISPLAY_NAME "EAR DirectSpeakers" 
  DESCRIPTION "The metadata input plugin for channel-based audio"
  OUTPUT_NAME "EAR DirectSpeakers")
  
target_link_libraries(direct_speakers_VST3 PRIVATE ear-plugin-base)

install_juce_vst3_plugin(direct_speakers "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
