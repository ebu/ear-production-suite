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

add_juce_vst3_plugin(direct_speakers SOURCES ${SOURCES_DIRECT_SPEAKERS_INPUT} 
                                     DISPLAY_NAME "EAR DirectSpeakers" 
                                     DESCRIPTION "The metadata input plugin for channel-based audio"
                                     OUTPUT_NAME "EAR DirectSpeakers")
target_link_libraries(direct_speakers_VST3 PRIVATE ear-plugin-base)
install(TARGETS direct_speakers_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
