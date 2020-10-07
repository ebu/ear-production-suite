set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES_SCENE_MASTER
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_combo_box.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_label.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_tab_button.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_tab_button_bar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_tabbed_component.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.cpp
  src/scene_frontend_connector.cpp
  src/backend_setup_timer.cpp
  src/scene_plugin_editor.cpp
  src/scene_plugin_processor.cpp
)

add_juce_vst3_plugin(scene
  SOURCES ${SOURCES_SCENE_MASTER}
  CODE_SUFFIX "FF"  
  DISPLAY_NAME "EAR Scene"
  DESCRIPTION "The scene plugin"
  OUTPUT_NAME "EAR Scene")
  
target_link_libraries(scene_VST3 PRIVATE ear-plugin-base)
install(TARGETS scene_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")


