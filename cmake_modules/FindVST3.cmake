if(NOT VST3_FOUND)

# Note: this is obviously not a complete implementation, 
# it's currently limited to what we need to build VST3 plugins with JUCE

find_path(VST3_SDK_DIR NAMES pluginterfaces PATHS ${VST3_ROOT_DIR} ENV VST3_ROOT_DIR HINTS "${CMAKE_SOURCE_DIR}/submodules/VST3")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VST3
  FOUND_VAR VST3_FOUND
  REQUIRED_VARS VST3_SDK_DIR
  FAIL_MESSAGE "Could NOT find VST3 SDK. Consider setting VST3_ROOT_DIR to the VST3 SDK root directory"
)

add_library(VST3::pluginterfaces INTERFACE IMPORTED)
target_include_directories(VST3::pluginterfaces INTERFACE ${VST3_SDK_DIR})

endif()
