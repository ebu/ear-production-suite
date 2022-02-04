find_package(Git)
function(git_get_last_timestamp)
    set(one_value_args FILE_NAME OUTPUT_VAR)
    cmake_parse_arguments(TS "" "${one_value_args}" "" ${ARGN} )
    execute_process(COMMAND "${GIT_EXECUTABLE}" --no-pager log -1 --format=%ct -- "${TS_FILE_NAME}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            OUTPUT_VARIABLE OUT
            ERROR_VARIABLE OUT)
    set(${TS_OUTPUT_VAR} ${OUT} PARENT_SCOPE)
endfunction()