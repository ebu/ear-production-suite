file(READ ${PROJECT_SOURCE_DIR}/CHANGELOG.md CHANGELOG ENCODING UTF-8)
configure_file(README.md.in ${CMAKE_CURRENT_BINARY_DIR}/README.md @ONLY)

include(git_get_last_timestamp)
function(check_commit_timestamp_not_after SOURCE_FILE DEST_FILE)
   git_get_last_timestamp(FILE_NAME ${SOURCE_FILE}
           OUTPUT_VAR FIRST_TS)
   git_get_last_timestamp(FILE_NAME ${DEST_FILE}
           OUTPUT_VAR SECOND_TS)
    if(SECOND_TS LESS FIRST_TS)
        if(EPS_CI)
            set(MSG_LEVEL FATAL_ERROR)
        else()
            set(MSG_LEVEL WARNING)
        endif()
        message(${MSG_LEVEL} "${SOURCE_FILE} has commit timestamp later than ${DEST_FILE}, try regenerating ${DEST_FILE} via the generate-package-docs target and commit the result")
    endif()
endfunction()

if(NOT EPS_CI) # This prevents build failure if the runner generating the docs has a system time behind the system used to modify the MDs
    check_commit_timestamp_not_after(LICENSE.md LICENSE.pdf)
    check_commit_timestamp_not_after(${PROJECT_SOURCE_DIR}/CHANGELOG.md README.pdf)
    check_commit_timestamp_not_after(README.md.in README.pdf)
endif()

add_custom_target(generate-package-docs
        COMMAND
            pandoc "${CMAKE_CURRENT_BINARY_DIR}/README.md" --output="${CMAKE_CURRENT_LIST_DIR}/README.pdf" -V colorlinks=true -V linkcolor=blue -V urlcolor=red -V toccolor=grey
        COMMAND
            pandoc "${CMAKE_CURRENT_LIST_DIR}/LICENSE.md" --output="${CMAKE_CURRENT_LIST_DIR}/LICENSE.pdf" -V colorlinks=true -V linkcolor=blue -V urlcolor=red -V toccolor=grey
        BYPRODUCTS
            "${CMAKE_CURRENT_LIST_DIR}/README.pdf"
            "${CMAKE_CURRENT_LIST_DIR}/LICENSE.pdf"
        )
        
# Common Files (all OS)
# note: '.' is "Setup EAR Production Suite.app/Contents/Resources" on MacOS
install(FILES
        README.pdf
        LICENSE.pdf
        DESTINATION
        .)
install(TARGETS
        project_upgrade
        project_upgrade_gui
        DESTINATION
        ./Tools)
install(DIRECTORY 
        Templates 
        DESTINATION
        .)

if(APPLE)
    # '.' is currectly Resources dir. Need these installing at base dir too.
    install(FILES
            README.pdf
            LICENSE.pdf
            DESTINATION
            ../../../)
    # Back-up to base dir for setup install location
    install(TARGETS
            setup
            DESTINATION
            ../../../)
endif()

if(WIN32)
    # Win tidies manifests in to a subdir. Note MacOS doesn't need as already has manifests in bundle (as a build-time copy step).
    install(FILES
            install_list.xml
            uninstall_list.xml
            DESTINATION
            ./SetupFiles)
    install(TARGETS
            setup
            DESTINATION
            .)
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "EAR Production Suite")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "EBU")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
