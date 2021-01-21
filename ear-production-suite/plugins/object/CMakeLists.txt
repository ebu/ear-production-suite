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

# Input plugin code suffixes to start from 10
add_juce_vst3_plugin(object
  SOURCES ${SOURCES_OBJECT_INPUT}
  CODE_SUFFIX "11"
  DISPLAY_NAME "EAR Object"
  DESCRIPTION "The metadata input plugin for objects"
  OUTPUT_NAME "EAR Object")
  
target_include_directories(object_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(object_VST3 PRIVATE ear-plugin-base)
install_juce_vst3_plugin(object "${EAR_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
