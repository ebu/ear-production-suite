cmake_minimum_required(VERSION 3.8)

project(adm-export-source-plugin VERSION ${EPS_VERSION_BASE}
								 DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
								 LANGUAGES CXX)
							 
set(CMAKE_PROJECT_VERSION ${EPS_VERSION_CURRENT})
message(STATUS "Project adm-export-source-plugin version: " ${CMAKE_PROJECT_VERSION})

find_package(JUCE REQUIRED QUIET)
find_package(VST3 REQUIRED QUIET)

add_subdirectory(src)
