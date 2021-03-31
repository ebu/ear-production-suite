set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES_BINAURAL_MONITORING
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
    src/binaural_monitoring_plugin_editor.cpp
    src/binaural_monitoring_plugin_processor.cpp
)

add_juce_vst3_plugin(
	ear_binaural_monitoring
	SOURCES ${SOURCES_BINAURAL_MONITORING}
    IDE_FOLDER ${IDE_FOLDER_PLUGINS}
	CODE_SUFFIX "F0" # Speaker Monitoring suffixes start from A0 and increment. For bin, lets use F0 (Note FF is scene)
	DISPLAY_NAME "EAR Binaural Monitoring"
	DESCRIPTION "The binaural monitoring plugin"
	OUTPUT_NAME "EAR Binaural Monitoring")
	
target_link_libraries(ear_binaural_monitoring_VST3 PRIVATE ear-plugin-base bear)

install(TARGETS ear_binaural_monitoring_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")

ExternalProject_Get_Property(default_tensorfile DOWNLOADED_FILE)
install(FILES ${DOWNLOADED_FILE} DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")