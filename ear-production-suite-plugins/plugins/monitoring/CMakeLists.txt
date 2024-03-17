set(SOURCES_MONITORING
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
    src/monitoring_plugin_editor.cpp
    src/monitoring_plugin_processor.cpp
)

set(HEADERS_MONITORING
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/monitoring_audio_processor.hpp
	${EPS_PLUGIN_BASE_DIR}/include/monitoring_backend.hpp
	${EPS_PLUGIN_BASE_DIR}/include/variable_block_adapter.hpp

	${EPS_SHARED_DIR}/binary_data.hpp
	${EPS_SHARED_DIR}/speaker_setups.hpp

	${EPS_SHARED_DIR}/components/ear_button.hpp
	${EPS_SHARED_DIR}/components/ear_header.hpp
	${EPS_SHARED_DIR}/components/level_meter.hpp
	${EPS_SHARED_DIR}/components/level_meter_calculator.hpp
	${EPS_SHARED_DIR}/components/onboarding.hpp
	${EPS_SHARED_DIR}/components/onboarding_slide.hpp
	${EPS_SHARED_DIR}/components/overlay.hpp
	${EPS_SHARED_DIR}/components/segment_progress_bar.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp

	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp

	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/properties_file.hpp

	src/monitoring_plugin_editor.hpp
	src/monitoring_plugin_processor.hpp
	src/speaker_meter.hpp
	src/speaker_meter_box.hpp
)

source_group("Header Files" FILES ${HEADERS_MONITORING})


function(add_monitoring_plugin SPEAKER_LAYOUT SPEAKER_LAYOUT_NAME AUDIO_PACK_FORMAT_ID PLUGIN_CODE_SUFFIX)
  add_juce_vst3_plugin(
    ear_monitoring_${SPEAKER_LAYOUT}
    SOURCES ${SOURCES_MONITORING} ${HEADERS_MONITORING}
	CODE_SUFFIX ${PLUGIN_CODE_SUFFIX}
    DISPLAY_NAME "EAR Monitoring ${SPEAKER_LAYOUT}"
    DESCRIPTION "The monitoring plugin (${SPEAKER_LAYOUT})"
    OUTPUT_NAME "EAR Monitoring ${SPEAKER_LAYOUT}"
    IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  )
  set(EPS_PLUGIN_TARGETS ${EPS_PLUGIN_TARGETS} PARENT_SCOPE)
  target_compile_definitions(
    ear_monitoring_${SPEAKER_LAYOUT}_VST3 PRIVATE
    SPEAKER_LAYOUT="${SPEAKER_LAYOUT}"
    SPEAKER_LAYOUT_NAME="${SPEAKER_LAYOUT_NAME}"
	AUDIO_PACK_FORMAT_ID="${AUDIO_PACK_FORMAT_ID}"
    )
  target_link_libraries(ear_monitoring_${SPEAKER_LAYOUT}_VST3 PRIVATE ear-plugin-base ear-version)
  install_juce_vst3_plugin(ear_monitoring_${SPEAKER_LAYOUT} "${EPS_PLUGIN_INSTALL_PREFIX}ear-production-suite")
endfunction()


add_monitoring_plugin("0+2+0" "2.0" "AP_00010002" "A0") # Let's start monitoring suffixes from A0 and increment
if(EAR_PLUGINS_BUILD_ALL_MONITORING_PLUGINS)
  add_monitoring_plugin("0+5+0" "5.1" "AP_00010003" "A1")
  add_monitoring_plugin("2+5+0" "5.1+2H" "AP_00010004" "A2")
  add_monitoring_plugin("4+5+0" "5.1+4H" "AP_00010005" "A3")
  add_monitoring_plugin("4+5+1" "" "AP_00010010" "A4")
  add_monitoring_plugin("3+7+0" "7.2+3H" "AP_00010007" "A5")
  add_monitoring_plugin("4+9+0" "9.1+4H" "AP_00010008" "A6")
  add_monitoring_plugin("9+10+3" "22.2" "AP_00010009" "A7")
  add_monitoring_plugin("0+7+0" "7.1" "AP_0001000f" "A8")
  add_monitoring_plugin("4+7+0" "7.1+4H" "AP_00010017" "A9")
  add_monitoring_plugin("2+7+0" "7.1+2H" "AP_00010016" "AA")
endif()
set(EPS_PLUGIN_TARGETS ${EPS_PLUGIN_TARGETS} PARENT_SCOPE)
