cmake_minimum_required(VERSION 3.10.0)

project(reaper_adm VERSION ${EPS_VERSION_BASE}
                   DESCRIPTION "Extension to provide ADM Import and Export capabilities in REAPER"
                   LANGUAGES CXX)
							 
message(STATUS "Project reaper_adm version: " ${CMAKE_PROJECT_VERSION})

include(CTest)
set(Boost_USE_STATIC_LIBS ON)
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
