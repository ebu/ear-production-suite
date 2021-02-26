set(SOURCES_BINAURAL_MONITORING
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.cpp
    src/binaural_monitoring_plugin_editor.cpp
    src/binaural_monitoring_plugin_processor.cpp
)

add_juce_vst3_plugin(
	ear_binaural_monitoring
	SOURCES ${SOURCES_BINAURAL_MONITORING}
	CODE_SUFFIX "F0" # Speaker Monitoring suffixes start from A0 and increment. For bin, lets use F0 (Note FF is scene)
	DISPLAY_NAME "EAR Binaural Monitoring"
	DESCRIPTION "The binaural monitoring plugin"
	OUTPUT_NAME "EAR Binaural Monitoring")
target_link_libraries(ear_binaural_monitoring_VST3 PRIVATE ear-plugin-base bear)
install(TARGETS ear_binaural_monitoring_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
