cmake_minimum_required(VERSION 3.8)

project(adm-export-source-plugin VERSION 0.6.0.0
                             DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
                             LANGUAGES CXX)

find_package(JUCE REQUIRED)
find_package(VST3 REQUIRED)

add_subdirectory(src)
