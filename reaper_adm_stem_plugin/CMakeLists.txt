cmake_minimum_required(VERSION 3.8)

project(adm-export-source-plugin VERSION 0.6.0.0
                             DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
                             LANGUAGES CXX)

if(APPLE)
  option(CMAKE_INSTALL_PREFIX "Install prefix path" "~/Library/Audio/Plug-Ins/VST3")
endif()
if(WIN32)
    option(CMAKE_INSTALL_PREFIX "Install prefix path" "C:\\Program Files\\Common Files\\VST3")
    SET(CMAKE_INSTALL_PREFIX "C:\\Program Files\\Common Files\\VST3" CACHE PATH "Install prefix path" FORCE)
endif()

find_package(JUCE REQUIRED)
find_package(VST3 REQUIRED)

add_subdirectory(src)
