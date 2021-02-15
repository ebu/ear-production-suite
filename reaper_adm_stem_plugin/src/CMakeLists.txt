set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE cpp_files *.cpp)
file(GLOB_RECURSE c_files *.c)
file(GLOB_RECURSE hpp_files *.hpp)
file(GLOB_RECURSE h_files *.h)

list(FILTER cpp_files EXCLUDE REGEX "\\._+")
list(FILTER c_files EXCLUDE REGEX "\\._+")
list(FILTER hpp_files EXCLUDE REGEX "\\._+")
list(FILTER h_files EXCLUDE REGEX "\\._+")

source_group("Header Files" FILES ${h_files} ${hpp_files})

set(VST_SOURCES
		${h_files}
		${hpp_files}
		${c_files}
		${cpp_files}
)

add_juce_vst3_plugin(adm_export_source SOURCES ${VST_SOURCES}
                                  DISPLAY_NAME "ADM Export Source"
                                  DESCRIPTION "Plug-in to describe and stream audio from a REAPER track to the REAPER ADM Extension during export"
                                  OUTPUT_NAME "ADM Export Source")


target_link_libraries(adm_export_source_VST3 PRIVATE
	nng::nng
	adm
)


target_include_directories(adm_export_source_VST3
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/submodules/readerwriterqueue>
	$<TARGET_PROPERTY:adm,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(adm_export_source_VST3
    PUBLIC JUCE_DISABLE_ASSERTIONS)

install(TARGETS
    adm_export_source_VST3
    COMPONENT
      Plugins
    DESTINATION
      ${EPS_PLUGIN_INSTALL_PREFIX}VST3
    )

include(DeployVST3)

set(DEP_SEARCH_PATHS "${PROJECT_BINARY_DIR}/lib" "${PROJECT_BINARY_DIR}/VST3" "${PROJECT_BINARY_DIR}/lib/*" "${PROJECT_BINARY_DIR}/VST3/*")
install_all_vst3_dependencies("${EPS_PLUGIN_INSTALL_PREFIX}VST3" "${DEP_SEARCH_PATHS}")
