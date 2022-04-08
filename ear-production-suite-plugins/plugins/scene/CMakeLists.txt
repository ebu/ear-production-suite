set(SOURCES_SCENE
  ${EPS_SHARED_DIR}/binary_data.cpp
  ${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.cpp
  ${EPS_SHARED_DIR}/components/look_and_feel/slider.cpp
  ${EPS_SHARED_DIR}/components/ear_combo_box.cpp
  ${EPS_SHARED_DIR}/components/ear_slider_label.cpp
  ${EPS_SHARED_DIR}/components/ear_tab_button.cpp
  ${EPS_SHARED_DIR}/components/ear_tab_button_bar.cpp
  ${EPS_SHARED_DIR}/components/ear_tabbed_component.cpp
  ${EPS_SHARED_DIR}/components/level_meter_calculator.cpp
  src/backend_setup_timer.cpp
  src/scene_plugin_editor.cpp
  src/scene_plugin_processor.cpp
  src/elements_container.cpp
  src/element_view_list.cpp
  src/element_view.cpp
  src/ui_event_dispatcher.cpp
  src/metadata_event_dispatcher.cpp
  src/programmes_container.cpp
  src/items_container.cpp
        )

set(HEADERS_SCENE
	${EPS_PLUGIN_BASE_DIR}/include/communication/common_types.hpp
	${EPS_PLUGIN_BASE_DIR}/include/detail/constants.hpp
	${EPS_PLUGIN_BASE_DIR}/include/helper/move.hpp
	${EPS_PLUGIN_BASE_DIR}/include/programme_store_adm_populator.hpp
	${EPS_PLUGIN_BASE_DIR}/include/programme_store_adm_serializer.hpp
	${EPS_PLUGIN_BASE_DIR}/include/scene_backend.hpp
	${EPS_SHARED_DIR}/binary_data.hpp
	${EPS_SHARED_DIR}/speaker_setups.hpp

	${EPS_SHARED_DIR}/components/ear_bounds_constrainer.hpp
	${EPS_SHARED_DIR}/components/ear_button.hpp
	${EPS_SHARED_DIR}/components/ear_colour_indicator.hpp
	${EPS_SHARED_DIR}/components/ear_combo_box.hpp
	${EPS_SHARED_DIR}/components/ear_expansion_panel.hpp
	${EPS_SHARED_DIR}/components/ear_header.hpp
	${EPS_SHARED_DIR}/components/ear_inverted_slider.hpp
	${EPS_SHARED_DIR}/components/ear_name_text_editor.hpp
	${EPS_SHARED_DIR}/components/ear_slider.hpp
	${EPS_SHARED_DIR}/components/ear_slider_label.hpp
	${EPS_SHARED_DIR}/components/ear_slider_range.hpp
	${EPS_SHARED_DIR}/components/ear_tab_button_bar.hpp
	${EPS_SHARED_DIR}/components/ear_tabbed_component.hpp
	${EPS_SHARED_DIR}/components/level_meter.hpp
	${EPS_SHARED_DIR}/components/level_meter_calculator.hpp
	${EPS_SHARED_DIR}/components/onboarding.hpp
	${EPS_SHARED_DIR}/components/onboarding_slide.hpp
	${EPS_SHARED_DIR}/components/overlay.hpp
	${EPS_SHARED_DIR}/components/read_only_audio_parameter_int.hpp
	${EPS_SHARED_DIR}/components/segment_progress_bar.hpp
	${EPS_SHARED_DIR}/components/version_label.hpp

	${EPS_SHARED_DIR}/components/look_and_feel/colours.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/fonts.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/name_text_editor.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/roboto.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/shadows.hpp
	${EPS_SHARED_DIR}/components/look_and_feel/slider.hpp

	${EPS_SHARED_DIR}/helper/graphics.hpp
	${EPS_SHARED_DIR}/helper/iso_lang_codes.hpp
	${EPS_SHARED_DIR}/helper/multi_async_updater.h
	${EPS_SHARED_DIR}/helper/nng_wrappers.h
	${EPS_SHARED_DIR}/helper/properties_file.hpp

	src/auto_mode_overlay.hpp
	src/backend_setup_timer.hpp
	src/ear_drop_indicator.hpp
	src/element_overview.hpp
	src/element_view.hpp
	src/element_view_list.hpp
	src/elements_container.hpp
	src/gain_interaction_settings.hpp
	src/item_view.hpp
	src/item_view_list.hpp
	src/items_container.hpp
	src/multiple_scene_plugins_overlay.hpp
	src/object_view.hpp
	src/position_interaction_settings.hpp
	src/programme_view.hpp
	src/programmes_container.hpp
        src/scene_plugin_editor.hpp
	src/scene_plugin_processor.hpp
        src/ui_event_dispatcher.hpp
        )

source_group("Header Files" FILES ${HEADERS_SCENE})

add_juce_vst3_plugin(scene
  SOURCES ${SOURCES_SCENE} ${HEADERS_SCENE}
  IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  CODE_SUFFIX "FF"  
  DISPLAY_NAME "EAR Scene"
  DESCRIPTION "The scene plugin"
  OUTPUT_NAME "EAR Scene")
  
target_link_libraries(scene_VST3 PRIVATE ear-plugin-base ear-version)
install_juce_vst3_plugin(scene "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")


