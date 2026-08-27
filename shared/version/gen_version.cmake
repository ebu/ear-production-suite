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
  		OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(ret EQUAL 0)
    set(${varname} ${out} PARENT_SCOPE)
  else()
    message(STATUS "GIT_EXECUTABLE: " ${GIT_EXECUTABLE})
    message(STATUS "ARGN: " ${ARGN})
    message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
endfunction()


# note CMAKE_CURRENT_SOURCE_DIR is current working directory when run as a script!
set(TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/eps_version.cpp.in)
set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/eps_version.cpp)
set(VERSION_INFO_AVAILABLE false)

# The script is included during configuration and also run standalone by the
# version generator target. In the latter case, derive the project version
# components from the value passed by the target.
if(NOT DEFINED CMAKE_PROJECT_VERSION OR CMAKE_PROJECT_VERSION STREQUAL "")
  if(DEFINED PROJECT_VERSION AND NOT PROJECT_VERSION STREQUAL "")
    set(CMAKE_PROJECT_VERSION "${PROJECT_VERSION}")
  endif()
endif()

set(_PROJECT_VERSION_AVAILABLE false)
if(DEFINED CMAKE_PROJECT_VERSION AND NOT CMAKE_PROJECT_VERSION STREQUAL "")
  if(NOT DEFINED CMAKE_PROJECT_VERSION_MAJOR
     OR CMAKE_PROJECT_VERSION_MAJOR STREQUAL ""
     OR NOT DEFINED CMAKE_PROJECT_VERSION_MINOR
     OR CMAKE_PROJECT_VERSION_MINOR STREQUAL ""
     OR NOT DEFINED CMAKE_PROJECT_VERSION_PATCH
     OR CMAKE_PROJECT_VERSION_PATCH STREQUAL "")
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" _PROJECT_VERSION_MATCH
      "${CMAKE_PROJECT_VERSION}")
    if(_PROJECT_VERSION_MATCH)
      set(CMAKE_PROJECT_VERSION_MAJOR "${CMAKE_MATCH_1}")
      set(CMAKE_PROJECT_VERSION_MINOR "${CMAKE_MATCH_2}")
      set(CMAKE_PROJECT_VERSION_PATCH "${CMAKE_MATCH_3}")
    endif()
  endif()
  if(CMAKE_PROJECT_VERSION_MAJOR MATCHES "^[0-9]+$"
     AND CMAKE_PROJECT_VERSION_MINOR MATCHES "^[0-9]+$"
     AND CMAKE_PROJECT_VERSION_PATCH MATCHES "^[0-9]+$")
    set(_PROJECT_VERSION_AVAILABLE true)
  endif()
endif()

function(use_project_version_fallback)
  if(NOT _PROJECT_VERSION_AVAILABLE)
    message(FATAL_ERROR "CMake project version is not available or invalid")
  endif()
  set(VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}" PARENT_SCOPE)
  set(VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}" PARENT_SCOPE)
  set(VERSION_PATCH "${CMAKE_PROJECT_VERSION_PATCH}" PARENT_SCOPE)
  set(VERSION_TWEAK "${CMAKE_PROJECT_VERSION_TWEAK}" PARENT_SCOPE)
  set(DESCRIPTIVE_VERSION "${CMAKE_PROJECT_VERSION}-no-tags" PARENT_SCOPE)
  set(NUMERIC_VERSION "${CMAKE_PROJECT_VERSION}" PARENT_SCOPE)
endfunction()

find_package(Git)
if(Git_FOUND)

  git_describe(_GIT_REVISION --tags --abbrev=4 --dirty --match v[0-9]*)
  message(STATUS "Version info from Git: " ${_GIT_REVISION})
 
  if(_GIT_REVISION MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)$")
    set(VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(VERSION_MINOR "${CMAKE_MATCH_2}")
    set(VERSION_PATCH "${CMAKE_MATCH_3}")
    set(VERSION_TWEAK "${CMAKE_MATCH_4}")
  
    #TODO: This is very similar to code in cmake_modules/get_git_version.cmake - consolidate!

    set(DESCRIPTIVE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
    set(NUMERIC_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
	set(VERSION_INFO_AVAILABLE true)

    if(NOT VERSION_TWEAK STREQUAL _GIT_REVISION)
      # Tweak is present
      set(DESCRIPTIVE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_TWEAK}" )		
      if(VERSION_TWEAK MATCHES "^[0-9]+$")	  
        # Tweak is numeric
        set(NUMERIC_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}" )
      endif()
    endif()
	
  else()
    use_project_version_fallback()
    git_describe(_GIT_COMMIT --always --abbrev=4 --dirty --match __no_matching_tag__)
    if(_GIT_COMMIT)
      set(DESCRIPTIVE_VERSION "${CMAKE_PROJECT_VERSION}-no-tags-${_GIT_COMMIT}")
    endif()
    message(WARNING "Git did not provide a matching version tag; using ${DESCRIPTIVE_VERSION}")
  endif()
  
else()  
  use_project_version_fallback()
  message(WARNING "Unable to find Git for version information; using ${DESCRIPTIVE_VERSION}")
endif()

configure_file(${TEMPLATE_FILE} ${VERSION_FILE})