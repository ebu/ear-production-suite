add_executable(ear_logging_server main.cpp)
target_link_libraries(ear_logging_server ear-plugin-base)
set_target_properties(ear_logging_server PROPERTIES FOLDER ${IDE_FOLDER_PLUGINS})

add_executable(test_sender test_sender.cpp)
target_link_libraries(test_sender ear-plugin-base)
set_target_properties(test_sender PROPERTIES FOLDER ${IDE_FOLDER_TESTS})