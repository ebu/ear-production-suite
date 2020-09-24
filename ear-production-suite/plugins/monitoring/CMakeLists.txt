set(SOURCES_MONITORING
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.cpp
    src/monitoring_plugin_editor.cpp
    src/monitoring_plugin_processor.cpp
)


function(add_monitoring_plugin SPEAKER_LAYOUT SPEAKER_LAYOUT_NAME)
  add_juce_vst3_plugin(
    ear_monitoring_${SPEAKER_LAYOUT}
    SOURCES ${SOURCES_MONITORING}
    DISPLAY_NAME "EAR Monitoring ${SPEAKER_LAYOUT}"
    DESCRIPTION "The monitoring plugin (${SPEAKER_LAYOUT})"
  OUTPUT_NAME "EAR Monitoring ${SPEAKER_LAYOUT}")
  target_compile_definitions(
    ear_monitoring_${SPEAKER_LAYOUT}_VST3 PRIVATE
    SPEAKER_LAYOUT="${SPEAKER_LAYOUT}"
    SPEAKER_LAYOUT_NAME="${SPEAKER_LAYOUT_NAME}"
    )
  target_link_libraries(ear_monitoring_${SPEAKER_LAYOUT}_VST3 PRIVATE ear-plugin-base)
  install(TARGETS ear_monitoring_${SPEAKER_LAYOUT}_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
endfunction()

add_monitoring_plugin("0+2+0" "2.0")
if(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS)
  add_monitoring_plugin("0+5+0" "5.1")
  add_monitoring_plugin("2+5+0" "5.1+2H")
  add_monitoring_plugin("4+5+0" "5.1+4H")
  add_monitoring_plugin("4+5+1" "")
  add_monitoring_plugin("3+7+0" "7.2+3H")
  add_monitoring_plugin("4+9+0" "9.1+4H")
  add_monitoring_plugin("9+10+3" "22.2")
  add_monitoring_plugin("0+7+0" "7.1")
  add_monitoring_plugin("4+7+0" "7.1+4H")
endif()
