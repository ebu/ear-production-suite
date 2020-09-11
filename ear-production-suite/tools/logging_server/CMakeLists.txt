add_executable(ear_logging_server main.cpp)
target_link_libraries(ear_logging_server ear-plugin-base)


add_executable(test_sender test_sender.cpp)
target_link_libraries(test_sender ear-plugin-base)
