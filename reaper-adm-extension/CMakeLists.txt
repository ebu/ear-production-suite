cmake_minimum_required(VERSION 3.10.0)
project(reaper_adm VERSION 0.6.0 LANGUAGES CXX)

include(CTest)
set(Boost_USE_STATIC_LIBS ON)
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(${CMAKE_SYSTEM_NAME})

