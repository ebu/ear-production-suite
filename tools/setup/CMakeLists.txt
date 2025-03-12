if(MSVC)
    add_compile_options(/bigobj)
endif()

find_package(JUCE REQUIRED QUIET)

set(MACOSX_BUNDLE_BUNDLE_NAME "Setup EAR Production Suite")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "ch.ebu.eps_setup")
set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
set(MACOSX_BUNDLE_COPYRIGHT "(C) 2019-2024 BBC, (C) 2020-2021 EBU, (C) 2019-2021 IRT")
add_executable(setup WIN32 MACOSX_BUNDLE)

set(HEADERS_SETUP
    MainComponent.h
    WindowBody.h
    install_phases/component_initial.h
    install_phases/component_license.h
    install_phases/component_select_operation.h
    install_phases/component_install_locations.h
    install_phases/component_error_log.h
    install_phases/component_uninstall_confirm.h
    install_phases/component_existing_search.h
    install_phases/component_processing.h
    install_phases/component_complete.h
    helpers/manifests.h
    ${EPS_SHARED_DIR}/helper/resource_paths_juce-file.hpp
    ${EPS_SHARED_DIR}/update_check_settings_file.h
)

set(SOURCES_SETUP
    Main.cpp
    MainComponent.cpp
    WindowBody.cpp
    install_phases/component_initial.cpp
    install_phases/component_license.cpp
    install_phases/component_select_operation.cpp
    install_phases/component_install_locations.cpp
    install_phases/component_error_log.cpp
    install_phases/component_uninstall_confirm.cpp
    install_phases/component_existing_search.cpp
    install_phases/component_processing.cpp
    install_phases/component_complete.cpp
    helpers/manifests.cpp
    ${EPS_SHARED_DIR}/binary_data.cpp
    ${EPS_SHARED_DIR}/update_check_settings_file.cpp
)

source_group("Header Files" FILES ${HEADERS_SETUP})

target_sources(setup
    PRIVATE
    ${HEADERS_SETUP}
    ${SOURCES_SETUP}
)

if(APPLE)
    target_sources(setup PRIVATE icon.icns)
    set_target_properties(setup PROPERTIES
        OUTPUT_NAME "Setup EAR Production Suite"
        XCODE_ATTRIBUTE_EXECUTABLE_NAME "Setup EAR Production Suite"
        MACOSX_BUNDLE TRUE)
    set_source_files_properties(icon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/packaging/install_list.xml $<TARGET_FILE_DIR:setup>/../Resources/install_list.xml
    )
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/packaging/uninstall_list.xml $<TARGET_FILE_DIR:setup>/../Resources/uninstall_list.xml
    )
endif()

if(WIN32)
    target_sources(setup
        PRIVATE
        ${HEADERS_SETUP}
        ${SOURCES_SETUP}
    )
    set_target_properties(setup PROPERTIES
        OUTPUT_NAME "setup"
        WIN32_EXECUTABLE TRUE)
    target_link_options(setup PRIVATE "/MANIFESTUAC:level='requireAdministrator'")
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/packaging/install_list.xml $<TARGET_FILE_DIR:setup>/SetupFiles/install_list.xml
    )
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/packaging/uninstall_list.xml $<TARGET_FILE_DIR:setup>/SetupFiles/uninstall_list.xml
    )
endif()

add_custom_command(TARGET setup PRE_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/helpers/gen_license_header.cmake
)

if(APPLE AND NOT EPS_CI)

    # Setup needs all the bits of the EPS in the Resources of the bundle to work
    # - copy all in to the bundle as a post-build step which will allow for testing without needing packaging
    
    add_dependencies(setup ${EPS_PLUGIN_TARGETS})
    add_dependencies(setup adm_export_source_VST3)
    add_dependencies(setup project_upgrade_gui project_upgrade)
    add_dependencies(setup reaper_adm)

    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:adm_export_source_VST3> $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/VST3/$<TARGET_BUNDLE_DIR_NAME:adm_export_source_VST3>
    )

    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:project_upgrade_gui> $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/Tools/$<TARGET_BUNDLE_DIR_NAME:project_upgrade_gui>
    )

    foreach(TGT IN LISTS EPS_PLUGIN_TARGETS)
        add_custom_command(TARGET setup POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:${TGT}> $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/VST3/ear-production-suite/$<TARGET_BUNDLE_DIR_NAME:${TGT}>
        )
    endforeach()

    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:project_upgrade> $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/Tools/
    )

    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:reaper_adm> $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/UserPlugins/$<TARGET_FILE_NAME:reaper_adm>
    )

    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/packaging/Templates $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/Templates
    )

    # Setup requires LICENSE.pdf and README.pdf but these only get generated during packaging
    # Just put blank files in for now
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E touch $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/LICENSE.pdf
    )
    add_custom_command(TARGET setup POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E touch $<TARGET_BUNDLE_CONTENT_DIR:setup>/Resources/README.pdf
    )

endif()

set_target_properties(setup PROPERTIES FOLDER tools)
target_link_libraries(setup PRIVATE Juce::core ear-version)
target_include_directories(setup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${EPS_SHARED_DIR})
