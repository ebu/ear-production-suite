function(add_juce_vst3_plugin PLUGIN_NAME)
  set(options)
  set(oneValueArgs IDE_FOLDER DESCRIPTION DISPLAY_NAME OUTPUT_NAME CODE_PREFIX CODE_SUFFIX)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(PLUGIN "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

  if(NOT PLUGIN_CODE_PREFIX)
    set(PLUGIN_CODE_PREFIX "455053") # Hex for "EPS"
  endif()

  if(NOT PLUGIN_CODE_SUFFIX)
    set(PLUGIN_CODE_SUFFIX "00")
  endif()
  
  if(NOT PLUGIN_DISPLAY_NAME)
    set(PLUGIN_DISPLAY_NAME "${PLUGIN_NAME}")
  endif()

  if(NOT PLUGIN_OUTPUT_NAME)
    set(PLUGIN_OUTPUT_NAME "${PLUGIN_NAME}")
  endif()

  set(PRODUCT_BUNDLE_IDENTIFIER "ear.${PLUGIN_NAME}")
  set(_SUPPORT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}_resources/)
  configure_file(${JUCE_SUPPORT_RESOURCES}/juce/AppConfig.h.in ${_SUPPORT_PATH}/AppConfig.h)
  configure_file(${JUCE_SUPPORT_RESOURCES}/juce/JuceHeader.h.in ${_SUPPORT_PATH}/JuceHeader.h)
  configure_file(${JUCE_SUPPORT_RESOURCES}/osx/VST-Info.plist.in ${_SUPPORT_PATH}/Info.plist)
  file(COPY ${JUCE_SUPPORT_RESOURCES}/osx/PkgInfo DESTINATION ${_SUPPORT_PATH})
  add_library(${PLUGIN_NAME}_VST3 MODULE ${PLUGIN_SOURCES} ${_SUPPORT_PATH}/AppConfig.h  ${_SUPPORT_PATH}/JuceHeader.h  ${_SUPPORT_PATH}/Info.plist  ${_SUPPORT_PATH}/PkgInfo)
  target_include_directories(${PLUGIN_NAME}_VST3 PRIVATE ${_SUPPORT_PATH}/ ${EPS_SHARED_DIR})
  target_link_libraries(${PLUGIN_NAME}_VST3 PRIVATE Juce::VST3)

  set_target_properties(${PLUGIN_NAME}_VST3 PROPERTIES
    BUNDLE TRUE
    OUTPUT_NAME "${PLUGIN_OUTPUT_NAME}"
    BUNDLE_EXTENSION "vst3"
    SUFFIX ".vst3"
    XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
    XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES"
    MACOSX_BUNDLE_INFO_PLIST "${_SUPPORT_PATH}/Info.plist"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/VST3"
    COMPILE_PDB_NAME "${PLUGIN_OUTPUT_NAME}"
    COMPILE_PDB_OUTPUT_DIR "${PROJECT_BINARY_DIR}/VST3"
  )
  
  if(PLUGIN_IDE_FOLDER)
    set_target_properties(${PLUGIN_NAME}_VST3 PROPERTIES FOLDER ${PLUGIN_IDE_FOLDER})
  endif()
  
  set_source_files_properties( ${_SUPPORT_PATH}/PkgInfo PROPERTIES
    MACOSX_PACKAGE_LOCATION .
  )

endfunction()

function(install_juce_vst3_plugin PLUGIN_NAME DESTINATION)
install(TARGETS ${PLUGIN_NAME}_VST3 COMPONENT Plugins DESTINATION "${DESTINATION}")
if(WIN32)
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}_VST3>
            DESTINATION "${DESTINATION}" OPTIONAL)
endif()
endfunction()

function (_generate_juce_module_stub MODULE_FILENAME DEST_PATH OUTVAR)
    get_filename_component(MODULE_NAME ${MODULE_FILENAME} NAME_WE)
    if(MODULE_NAME MATCHES "juce_audio_plugin_client_(utils|VST3|VST|AU|AAX)")
        set(MODULE_NAME "juce_audio_plugin_client")
    endif()
    set(FILENAME "${DEST_PATH}/include_${MODULE_FILENAME}")
    set(${OUTVAR} ${FILENAME} PARENT_SCOPE)
    if(NOT EXISTS "${FILENAME}")
      file(WRITE "${FILENAME}" "#include \"AppConfig.h\"\n#include <${MODULE_NAME}/${MODULE_FILENAME}>\n")
    endif()
endfunction()


function (_generate_juce_sources VAR)
    set(DESTINATION "${PROJECT_BINARY_DIR}/support/juce")
    # common modules
    set(MODULES
      juce_audio_basics
      juce_audio_processors
      juce_audio_devices
      juce_dsp
      juce_audio_formats
      juce_audio_utils
      juce_core
      juce_cryptography
      juce_data_structures
      juce_events
      juce_graphics
      juce_gui_basics
      juce_gui_extra
      juce_opengl
      juce_video
      juce_osc
    )
    foreach(MODULE  ${MODULES})
        _generate_juce_module_stub(${MODULE}.cpp ${DESTINATION} GENERATED_FILE)
        list(APPEND GENERATED_JUCE_FILES ${GENERATED_FILE})
    endforeach()

    # vst3 sources

    _generate_juce_module_stub("juce_audio_plugin_client_VST3.cpp" ${DESTINATION} GENERATED_FILE)
    list(APPEND GENERATED_JUCE_FILES_VST3 ${GENERATED_FILE})
    _generate_juce_module_stub("juce_audio_plugin_client_utils.cpp" ${DESTINATION} GENERATED_FILE)
    list(APPEND GENERATED_JUCE_FILES_VST3 ${GENERATED_FILE})
    if(APPLE)
        _generate_juce_module_stub("juce_audio_plugin_client_VST_utils.mm" ${DESTINATION} GENERATED_FILE)
        list(APPEND GENERATED_JUCE_FILES_VST3 ${GENERATED_FILE})
    endif()

    # au sources
    if(APPLE)
        _generate_juce_module_stub("juce_audio_plugin_client_AU1.mm" ${DESTINATION} GENERATED_FILE)
        list(APPEND GENERATED_JUCE_FILES_AU ${GENERATED_FILE})
        _generate_juce_module_stub("juce_audio_plugin_client_AU2.mm" ${DESTINATION} GENERATED_FILE)
        list(APPEND GENERATED_JUCE_FILES_AU ${GENERATED_FILE})
    endif()

    # aax sources
    _generate_juce_module_stub("juce_audio_plugin_client_AAX.cpp" ${DESTINATION} GENERATED_FILE)
    list(APPEND GENERATED_JUCE_FILES_AAX ${GENERATED_FILE})

    set(${VAR} ${GENERATED_JUCE_FILES} PARENT_SCOPE)
    set(${VAR}_VST3 ${GENERATED_JUCE_FILES_VST3} PARENT_SCOPE)
    set(${VAR}_AU ${GENERATED_JUCE_FILES_AU} PARENT_SCOPE)
    set(${VAR}_AAX ${GENERATED_JUCE_FILES_AAX} PARENT_SCOPE)
endfunction()
