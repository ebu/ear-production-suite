function(git_describe varname)
  message(STATUS "GIT_EXECUTABLE: " ${GIT_EXECUTABLE})
  message(STATUS "ARGN: " ${ARGN})
  message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
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
  		#ERROR_QUIET
  		OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(ret EQUAL 0)
		set(${varname} ${out} PARENT_SCOPE)
  endif()
endfunction()


# note CMAKE_CURRENT_SOURCE_DIR is current working directory when run as a script!
set(TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/eps_version.cpp.in)
set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/eps_version.cpp)
set(VERSION_INFO_AVAILABLE false)

find_package(Git)
if(Git_FOUND)

  git_describe(_GIT_REVISION --tags --abbrev=4 --dirty --match v[0-9]*)
  string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${_GIT_REVISION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" VERSION_TWEAK "${_GIT_REVISION}")
  message(STATUS "Version info from Git: " ${_GIT_REVISION})
 
  if(_GIT_REVISION)

    set(DESCRIPTIVE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
    set(NUMERIC_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
	set(VERSION_INFO_AVAILABLE true)

    if(NOT VERSION_TWEAK STREQUAL _GIT_REVISION)
      # Tweak is present
      set(DESCRIPTIVE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}" )		
      if(VERSION_TWEAK MATCHES "^[0-9]+$")	  
        # Tweak is numeric
        set(NUMERIC_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}" )
      endif()
    endif()
	
  else()
    set(WARNING_MESSAGE "!!! Git failed to provide any version information !!!")
    message(WARNING ${WARNING_MESSAGE})
	set(DESCRIPTIVE_VERSION ${WARNING_MESSAGE})
    set(NUMERIC_VERSION ${WARNING_MESSAGE})
  endif()
  
else()  
  set(WARNING_MESSAGE "Unable to find Git for version information!")
  message(WARNING ${WARNING_MESSAGE})
  set(DESCRIPTIVE_VERSION ${WARNING_MESSAGE})
  set(NUMERIC_VERSION ${WARNING_MESSAGE})
endif()

configure_file(${TEMPLATE_FILE} ${VERSION_FILE})