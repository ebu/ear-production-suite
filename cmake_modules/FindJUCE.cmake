if(NOT JUCE_FOUND)

find_path(JUCE_SDK_DIR
  NAMES modules/juce_audio_basics
  PATHS ${JUCE_ROOT_DIR}
        ENV JUCE_ROOT_DIR
  HINTS
        "${CMAKE_SOURCE_DIR}/submodules/JUCE"
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JUCE
  FOUND_VAR JUCE_FOUND
  REQUIRED_VARS JUCE_SDK_DIR
  FAIL_MESSAGE "Could NOT find JUCE. Consider setting JUCE_ROOT_DIR to the JUCE SDK root directory"
)

  if(JUCE_FOUND)
    include(juce_helpers)


    _generate_juce_sources(JUCE_GENERATED_SOURCES)
    # Main/core/shared JUCE library target
    ############
    add_library(Juce::core INTERFACE IMPORTED)
    target_sources(Juce::core INTERFACE ${JUCE_GENERATED_SOURCES})
    # set(_SUPPORT_PATH ${CMAKE_CURRENT_BINARY_DIR}/juce_core_resources/)
    # configure_file(${_JUCE_SUPPORT_RESOURCES}/juce/AppConfig.h.in ${_SUPPORT_PATH}/AppConfig.h)
    # configure_file(${_JUCE_SUPPORT_RESOURCES}/juce/JuceHeader.h.in ${_SUPPORT_PATH}/JuceHeader.h)
    # target_include_directories(Juce_core PRIVATE ${_SUPPORT_PATH})
    target_include_directories(Juce::core INTERFACE ${JUCE_SDK_DIR}/modules)
    target_compile_features(Juce::core INTERFACE cxx_std_14)
    target_compile_definitions(Juce::core INTERFACE
                    $<$<CONFIG:DEBUG>:DEBUG>
                    $<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>)

    if(APPLE)
      find_package( OpenGL REQUIRED )
      find_library(AUDIOUNIT AudioUnit)
      find_library(AUDIOTOOLBOX AudioToolbox)
      find_library(AVFOUNDATION AVFoundation)
      find_library(AVKIT AVKit)
      find_library(CARBON Carbon)
      find_library(COCOA Cocoa)
      find_library(COREAUDIO CoreAudio)
      find_library(COREFOUNDATION CoreFoundation)
      find_library(COREMEDIA CoreMedia)
      find_library(COREMIDI CoreMidi)
      find_library(QUARTZCORE QuartzCore)
      find_library(IOKIT IOKit)
      find_library(AGL AGL)
      find_library(ACCELERATE Accelerate)
      find_library(WEBKIT WebKit)
      find_library(OBJC objc)
      find_library(COREAUDIOKIT CoreAudioKit)

      target_link_libraries(Juce::core INTERFACE
          ${OPENGL_gl_LIBRARY}
          ${AUDIOUNIT}
          ${AUDIOTOOLBOX}
          ${AVFOUNDATION}
          ${AVKIT}
          ${CARBON}
          ${COCOA}
          ${COREAUDIO}
          ${COREFOUNDATION}
          ${COREMEDIA}
          ${COREMIDI}
          ${QUARTZCORE}
          ${IOKIT}
          ${AGL}
          ${ACCELERATE}
          ${WEBKIT}
          ${OBJC}
          ${COREAUDIOKIT})
        target_compile_options(Juce::core INTERFACE -x objective-c++)
    elseif(WIN32)
      target_link_libraries(Juce::core INTERFACE
        advapi32.lib
        comdlg32.lib
        gdi32.lib
        GlU32.Lib
        kernel32.lib
        ole32.lib
        OpenGL32.Lib
        rpcrt4.lib
        shell32.lib
        user32.lib
        vfw32.lib
        wininet.lib
        winmm.lib
        ws2_32.lib
      )
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
      find_package(OpenGL REQUIRED)
      find_package(Freetype REQUIRED)
      target_link_libraries(Juce::core INTERFACE
        OpenGL::GL
        Freetype::Freetype
      )
    endif(APPLE)

    # VST3
    ############
    add_library(Juce::VST3 INTERFACE IMPORTED)
    target_compile_definitions(Juce::VST3 INTERFACE JucePlugin_Build_VST3=1)
    target_link_libraries(Juce::VST3 INTERFACE pluginterfaces sdk Juce::core)
    target_sources(Juce::VST3 INTERFACE ${JUCE_GENERATED_SOURCES_VST3})
  endif()

endif()
