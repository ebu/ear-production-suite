if(MSVC)
    add_compile_options(/bigobj)
endif()

find_package(JUCE REQUIRED QUIET)

set(MACOSX_BUNDLE_BUNDLE_NAME "Setup EAR Production Suite")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "ch.ebu.eps_setup")
set(MACOSX_BUNDLE_COPYRIGHT "(C) 2019-2023 BBC, (C) 2020-2021 EBU, (C) 2019-2021 IRT")
add_executable(setup WIN32 MACOSX_BUNDLE)

set(HEADERS_SETUP
	MainComponent.h
	WindowBody.h
	install_phases/component_initial.h
	helpers/manifests.h
)

set(SOURCES_SETUP
	Main.cpp
	MainComponent.cpp
	WindowBody.cpp
	install_phases/component_initial.cpp
	helpers/manifests.cpp
	${EPS_SHARED_DIR}/binary_data.cpp
)

source_group("Header Files" FILES ${HEADERS_SETUP})

target_sources(setup
	PRIVATE
	${HEADERS_SETUP}
	${SOURCES_SETUP}
)

if(APPLE)
	set_target_properties(setup PROPERTIES
        OUTPUT_NAME "Setup EAR Production Suite"
        XCODE_ATTRIBUTE_EXECUTABLE_NAME "Setup EAR Production Suite"
        MACOSX_BUNDLE TRUE)
endif()
if(WIN32)
	set_target_properties(setup PROPERTIES
        OUTPUT_NAME "setup"
        WIN32_EXECUTABLE TRUE)
endif()

add_custom_command(TARGET setup POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/packaging/install_list.xml $<TARGET_FILE_DIR:setup>/install_list.xml
)

set_target_properties(setup PROPERTIES FOLDER tools)		
target_link_libraries(setup PRIVATE Juce::core ear-version)
target_include_directories(setup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${EPS_SHARED_DIR})