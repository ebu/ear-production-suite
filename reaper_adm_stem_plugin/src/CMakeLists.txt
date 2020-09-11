set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE cpp_files *.cpp)
file(GLOB_RECURSE c_files *.c)
file(GLOB_RECURSE hpp_files *.hpp)
file(GLOB_RECURSE h_files *.h)

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

set(macos_install_dir "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
set(win_install_dir "$ENV{programfiles}/Common Files/VST3")
set(linux_install_dir "share/REAPER/UserPlugins") # TODO - don't know the correct location

install(TARGETS
    adm_export_source_VST3
     DESTINATION
        $<$<PLATFORM_ID:Windows>:${win_install_dir}>$<$<PLATFORM_ID:Darwin>:${macos_install_dir}>$<$<PLATFORM_ID:Linux>:${linux_install_dir}>
    )
