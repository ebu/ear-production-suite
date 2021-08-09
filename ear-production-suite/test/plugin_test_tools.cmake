
find_package(Boost COMPONENTS filesystem REQUIRED QUIET)

add_library(plugin_test_tools STATIC plugin_test_tools/mockup_host.cpp plugin_test_tools/mockup_scene.cpp plugin_test_tools/mockup_monitoring.cpp plugin_test_tools/mockup_input_plugin.cpp)
target_include_directories(plugin_test_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/plugin_test_tools)
target_link_libraries(plugin_test_tools PUBLIC ear-plugin-base Boost::boost Boost::filesystem)

add_library(scene_mockup MODULE plugin_test_tools/scene_plugin.cpp)
set_target_properties(scene_mockup PROPERTIES PREFIX "")
target_include_directories(scene_mockup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin_test_tools)
target_link_libraries(scene_mockup plugin_test_tools )

add_library(input_plugin_mockup MODULE plugin_test_tools/input_plugin.cpp)
set_target_properties(input_plugin_mockup PROPERTIES PREFIX "")
target_include_directories(input_plugin_mockup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin_test_tools)
target_link_libraries(input_plugin_mockup plugin_test_tools )

add_library(monitoring_mockup MODULE plugin_test_tools/monitoring_plugin.cpp)
set_target_properties(monitoring_mockup PROPERTIES PREFIX "")
target_include_directories(monitoring_mockup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin_test_tools)
target_link_libraries(monitoring_mockup plugin_test_tools )

