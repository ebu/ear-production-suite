set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES_SCENE
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
  src/elements_container.cpp
  src/element_view_list.cpp
  src/element_view.cpp
)

set(HEADERS_SCENE
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/communication/common_types.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/detail/constants.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/helper/move.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/item_store.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/programme_store_adm_populator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/programme_store_adm_serializer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/scene_backend.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/speaker_setups.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include/ui/scene_frontend_backend_connector.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/../shared/binary_data.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_bounds_constrainer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_button.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_colour_indicator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_combo_box.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_expansion_panel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_header.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_inverted_slider.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_name_text_editor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_label.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_slider_range.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_tab_button_bar.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/ear_tabbed_component.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/level_meter_calculator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/onboarding.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/onboarding_slide.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/overlay.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/read_only_audio_parameter_int.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/segment_progress_bar.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/colours.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/fonts.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/name_text_editor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/roboto.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/shadows.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/components/look_and_feel/slider.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/graphics.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/iso_lang_codes.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/multi_async_updater.h
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/helper/properties_file.hpp

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
	src/nng_wrappers.h
	src/object_view.hpp
	src/position_interaction_settings.hpp
	src/programme_view.hpp
	src/programmes_container.hpp
	src/scene_frontend_connector.hpp
	src/scene_plugin_editor.hpp
	src/scene_plugin_processor.hpp
)

source_group("Header Files" FILES ${HEADERS_SCENE})

add_juce_vst3_plugin(scene
  SOURCES ${SOURCES_SCENE} ${HEADERS_SCENE}
  IDE_FOLDER ${IDE_FOLDER_PLUGINS}
  CODE_SUFFIX "FF"  
  DISPLAY_NAME "EAR Scene"
  DESCRIPTION "The scene plugin"
  OUTPUT_NAME "EAR Scene")
  
target_link_libraries(scene_VST3 PRIVATE ear-plugin-base)
install_juce_vst3_plugin(scene "${EPS_PLUGIN_INSTALL_PREFIX}VST3/ear-production-suite")


