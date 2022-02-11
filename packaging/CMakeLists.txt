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

check_commit_timestamp_not_after(LICENSE.md LICENSE.pdf)
check_commit_timestamp_not_after(${PROJECT_SOURCE_DIR}/CHANGELOG.md README.pdf)
check_commit_timestamp_not_after(README.md.in README.pdf)

add_custom_target(generate-package-docs
        COMMAND
            pandoc "${CMAKE_CURRENT_BINARY_DIR}/README.md" --output="${CMAKE_CURRENT_LIST_DIR}/README.pdf" -V colorlinks=true -V linkcolor=blue -V urlcolor=red -V toccolor=grey
        COMMAND
            pandoc "${CMAKE_CURRENT_LIST_DIR}/LICENSE.md" --output="${CMAKE_CURRENT_LIST_DIR}/LICENSE.pdf" -V colorlinks=true -V linkcolor=blue -V urlcolor=red -V toccolor=grey
        BYPRODUCTS
            "${CMAKE_CURRENT_LIST_DIR}/README.pdf"
            "${CMAKE_CURRENT_LIST_DIR}/LICENSE.pdf"
        )

install(FILES
        EAR-Production-Suite_Session-Template.RPP
        README.pdf
        LICENSE.pdf
        DESTINATION
        .)

install(TARGETS
        project_upgrade
        project_upgrade_gui
        DESTINATION
        tools)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "EAR Production Suite")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "EBU")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
