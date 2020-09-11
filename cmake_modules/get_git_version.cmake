function(git_describe varname)
  execute_process(COMMAND
  		"${GIT_EXECUTABLE}"
  		describe
  		${ARGN}
  		WORKING_DIRECTORY
  		"${CMAKE_CURRENT_SOURCE_DIR}"
  		RESULT_VARIABLE
  		ret
  		OUTPUT_VARIABLE
  		out
  		ERROR_QUIET
  		OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(ret EQUAL 0)
		set(${varname} ${out} PARENT_SCOPE)
  endif()
endfunction()

function(update_version_from_git VARBASE)
  find_package(Git)
  if(Git_FOUND)

  git_describe(_GIT_REVISION --tags --abbrev=4 --dirty --match v[0-9]*)
  string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" VERSION_TWEAK "${_GIT_REVISION}")
  if(_GIT_REVISION)
    set(${VARBASE}_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
    set(${VARBASE}_MINOR ${VERSION_MINOR} PARENT_SCOPE)
    set(${VARBASE}_PATCH ${VERSION_PATCH} PARENT_SCOPE)
    if(NOT VERSION_TWEAK STREQUAL _GIT_REVISION)
      set(${VARBASE}_TWEAK ${VERSION_TWEAK} PARENT_SCOPE)
      set(${VARBASE} "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}" PARENT_SCOPE)
    else()
      set(${VARBASE} "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" PARENT_SCOPE)
    endif()

  endif()
endif()
endfunction()
