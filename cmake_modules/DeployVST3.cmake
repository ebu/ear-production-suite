# A few functions to create standalone plugin bundles by detecting and copying dependencies
#
# This is a two step process.
# The first step ('install_vst3_dependencies') is runs at cmake configuration time
# while the second step has to be run at _install_ time ('fixup_vst3_plugin_bundle').
#
# The reason is that some information is required to configure the fixup step (like variables, paths, target names)
# that are only available during configuration time.
# On the other hand, we can only detect and fix potential dependencies _after_ the bundles have been build
# and installed/copied to the final destination.
#
# To achieve this, the first step "install_vst3_dependencies" automatically registeres a cmake script
# which will be run during installation and calls "fixup_vst3_plugin_bundle" with the correct arguments and variables.
#
# NOTE: the normal cmake BundleUtilities won't do the trick here, as they refuse to work on non ".app" bundles
# without an executable in it (i.e. doesn't work with out .vst3 bundles containing plugins)
#
#include(BundleUtilities)

set(DeployVST3_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")

function(install_all_vst3_dependencies bundlepath dirs)
  set(component ${ARGV3})
  install(CODE 
       "include(\"${DeployVST3_cmake_dir}/DeployVST3.cmake\")
        do_install_all_vst3_dependencies(${bundlepath} ${dirs} ${component})")
endfunction()
  
function(do_install_all_vst3_dependencies bundlepath dirs)
  set(component ${ARGV3})

  if(IS_ABSOLUTE "${bundlepath}")
    file(GLOB BUNDLES "${bundlepath}/*.vst3")
  else()
    file(GLOB BUNDLES "$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/${bundlepath}/*.vst3")
  endif()
  foreach(bundle ${BUNDLES})
    fixup_vst3_plugin_bundle(${bundle} ${dirs} ${component})
  endforeach()
endfunction()

function(install_vst3_dependencies bundle dirs)
  set(component ${ARGV3})

  install(CODE
    "include(\"${DeployVST3_cmake_dir}/DeployVST3.cmake\")
    fixup_vst3_plugin_bundle(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${bundle}\" \"${dirs}\")"
                  ${component}
        )
endfunction()

function(fixup_vst3_plugin_bundle bundle dirs)


  if(APPLE)
    get_bundle_main_executable("${bundle}" relative_bundle_executable)
    get_filename_component(bundle_executable "${relative_bundle_executable}" ABSOLUTE)
    get_filename_component(absolute_bundle_path "${bundle}" ABSOLUTE)
    set(dependencies_target_path "${absolute_bundle_path}/Contents/Frameworks/")
  elseif(WIN32)
    get_filename_component(bundle_executable ${bundle} ABSOLUTE)
    get_filename_component(absolute_bundle_path "${bundle_executable}" DIRECTORY)
    set(dependencies_target_path "${absolute_bundle_path}")
  else()
    message(FATAL_ERROR "Don't know how to handle bundling on this system.")
  endif()

  message(STATUS "Fixing up: ${bundle}")
  message(STATUS "  Bundle path:  ${absolute_bundle_path}")
  message(STATUS "  Embedded item: ${bundle_executable}")
  message(STATUS "  Search dirs: ${dirs}")
  message(STATUS "  Destination for deps: ${dependencies_target_path}")

  get_prerequisites("${bundle_executable}" prereqs 1 1 "${absolute_bundle_path}" "${dirs}")
  list(APPEND files_to_fix_rpaths "${bundle_executable}")

  foreach(prereq  ${prereqs})
    # This resolves any "@rpath/libfoo.dyld" to "/path/to/libfoo.dyld"
    gp_resolve_item("${bundle_executable}" ${prereq} "${absolute_bundle_path}" "${dirs}" resolved_item)
    # follow any symlinks to ensure we copy a real file, not just a symlink to nowhere ...
    get_filename_component(prereq_real_path ${resolved_item} REALPATH)
    get_filename_component(prereq_filename "${prereq_real_path}" NAME)
    message(STATUS " Copying ${prereq_real_path}")

    file(COPY ${prereq_real_path} DESTINATION "${dependencies_target_path}")
    if(APPLE)
      execute_process(COMMAND chmod u+w "${absolute_bundle_path}/Contents/Frameworks/${prereq_filename}")
      # collect all changes that need to be applied to the plugin in the bundle
      set(changes ${changes} "-change" "${prereq}" "@loader_path/../Frameworks/${prereq_filename}")
      list(APPEND files_to_fix_rpaths "${absolute_bundle_path}/Contents/Frameworks/${prereq_filename}")
    endif()
  endforeach()

  if(APPLE AND changes)
    find_program(CMAKE_INSTALL_NAME_TOOL NAMES install_name_tool HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
    foreach(filepath ${files_to_fix_rpaths})
    set(cmd ${CMAKE_INSTALL_NAME_TOOL} ${changes} "${filepath}")
    execute_process(COMMAND ${cmd} RESULT_VARIABLE install_name_tool_result)
    message(STATUS "Fixing rpaths for ${filepath}")
    if(NOT install_name_tool_result EQUAL 0)
      string(REPLACE ";" "' '" msg "'${cmd}'")
      message(FATAL_ERROR "Command failed:\n ${msg}")
    endif()
    endforeach()
  endif()
endfunction()
