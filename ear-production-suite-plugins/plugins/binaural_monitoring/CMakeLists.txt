set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES_BINAURAL_MONITORING
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/look_and_feel/slider.cpp
    ${EPS_SHARED_DIR}/components/ear_header.cpp
    ${EPS_SHARED_DIR}/components/ear_slider_label.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
	src/binaural_monitoring_frontend_connector.cpp
    src/binaural_monitoring_plugin_editor.cpp
    src/binaural_monitoring_plugin_processor.cpp
	src/orientation_osc.cpp
)

set(HEADERS_BINAURAL_MONITORING
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/ui/binaural_monitoring_frontend_backend_connector.hpp
	${EPS_PLUGIN_BASE_DIR}/include/binaural_monitoring_audio_processor.hpp
	${EPS_PLUGIN_BASE_DIR}/include/binaural_monitoring_backend.hpp
	${EPS_PLUGIN_BASE_DIR}/include/listener_orientation.hpp
	${EPS_PLUGIN_BASE_DIR}/include/variable_block_adapter.hpp

	${EPS_SHARED_DIR}/binary_data.hpp
	
	${EPS_SHARED_DIR}/components/ear_button.hpp
	${EPS_SHARED_DIR}/components/ear_header.hpp
	${EPS_SHARED_DIR}/components/ear_inverted_slider.hpp
	${EPS_SHARED_DIR}/components/ear_slider.hpp
	${EPS_SHARED_DIR}/components/ear_slider_label.hpp
	${EPS_SHARED_DIR}/components/level_meter.hpp
	${EPS_SHARED_DIR}/components/level_meter_calculator.hpp
	${EPS_SHARED_DIR}/components/onboarding.hpp
	${EPS_SHARED_DIR}/components/orientation.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp
	
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp
	
	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/multi_async_updater.h
	${EPS_SHARED_DIR}/helper/properties_file.hpp

	src/binaural_monitoring_frontend_connector.hpp
	src/binaural_monitoring_plugin_editor.hpp
	src/binaural_monitoring_plugin_processor.hpp
	src/error_overlay.hpp
	src/headphone_channel_meter.hpp
	src/headphone_channel_meter_box.hpp
	src/orientation_osc.hpp
	src/value_box_orientation.hpp
	src/value_box_osc.hpp
)

source_group("Header Files" FILES ${HEADERS_BINAURAL_MONITORING})

add_juce_vst3_plugin(
	ear_binaural_monitoring
	SOURCES ${SOURCES_BINAURAL_MONITORING} ${HEADERS_BINAURAL_MONITORING}
    IDE_FOLDER ${IDE_FOLDER_PLUGINS}
	CODE_SUFFIX "F0" # Speaker Monitoring suffixes start from A0 and increment. For bin, lets use F0 (Note FF is scene)
	DISPLAY_NAME "EAR Binaural Monitoring"
	DESCRIPTION "The binaural monitoring plugin"
	OUTPUT_NAME "EAR Binaural Monitoring")
	
target_link_libraries(ear_binaural_monitoring_VST3 PRIVATE ear-plugin-base ear-version bear)
install(TARGETS ear_binaural_monitoring_VST3 COMPONENT Plugins DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")

set(TENSORFILE_TARGET tensorfile_default_small)

ExternalProject_Get_Property(${TENSORFILE_TARGET} DOWNLOADED_FILE)
set(TENSORFILE_FULLPATH ${DOWNLOADED_FILE})
get_filename_component(TENSORFILE_FILENAME ${TENSORFILE_FULLPATH} NAME)
add_dependencies(ear_binaural_monitoring_VST3 ${TENSORFILE_TARGET})
target_compile_definitions(ear_binaural_monitoring_VST3 PRIVATE BEAR_DATA_FILE="${TENSORFILE_FILENAME}")

add_custom_command(TARGET ear_binaural_monitoring_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TENSORFILE_FULLPATH} "$<TARGET_FILE_DIR:ear_binaural_monitoring_VST3>/${TENSORFILE_FILENAME}"
)

if(APPLE)
    install(FILES ${TENSORFILE_FULLPATH} DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite/EAR Binaural Monitoring.vst3/Contents/Resources")
else()
	install(FILES ${TENSORFILE_FULLPATH} DESTINATION "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")
endif()