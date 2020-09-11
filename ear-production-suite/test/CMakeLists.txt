include(${CMAKE_SOURCE_DIR}/submodules/catch2.cmake)

include(plugin_test_tools.cmake)

# --- unit tests ---
function(add_ear_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name}
    PRIVATE
    ear-plugin-base
    catch2
  )
  target_include_directories(${name} PRIVATE
    ${PROJECT_SOURCE_DIR}/submodules
  )
  add_test(
    NAME ${name}
    COMMAND $<TARGET_FILE:${name}>
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/test_data"
  )
endfunction()


add_executable(test_host_runner test_host_runner.cpp)
target_include_directories(test_host_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin_test_tools)
target_link_libraries(test_host_runner plugin_test_tools)

# copy test files so unit test can find them relative to their running location
# when executed as "test" target
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data" DESTINATION ${PROJECT_BINARY_DIR})

add_ear_test("protobuf_tests")
add_ear_test("message_coding_tests")
add_ear_test("connection_manager_tests")
add_ear_test("connection_id_tests")
add_ear_test("nng_tests")
add_ear_test("scene_tests")
target_include_directories(scene_tests PRIVATE ${PROJECT_BINARY_DIR}/juce_core_resources) # JuceHeader.h
add_ear_test("scene_gains_calculator_tests")
add_ear_test("variable_block_adapter_tests")
add_ear_test("monitoring_audio_processor_tests")
add_ear_test("programme_store_adm_serializer_tests")
add_ear_test("programme_store_adm_populator_tests")
