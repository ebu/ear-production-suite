#include "ui_text.h"
#include <unordered_map>
#include <cassert>

std::string eps::uiText(eps::TextID item) {
  static std::unordered_map<TextID, std::string> dictionary {
      {TextID::EXTENSION_ERROR_TITLE, "ADM Extension Error"},
      {TextID::CONTEXT_MENU, "ADM"},
      {TextID::EXPLODE_ACTION_PREFIX, "Explode using "},
      {TextID::EXPLODE_ERROR_TITLE, "ADM: Explode to Takes"},
      {TextID::EXPLODE_ERROR_DESCRIPTION, "Please select a source before running this action."},
      {TextID::CREATE_PROJECT_MENU, "Create project from ADM file"},
      {TextID::CREATE_PROJECT_ACTION_PREFIX, "Create from ADM using "},
      {TextID::CREATE_PROJECT_FILE_PROMPT, "ADM File to Open"},
      {TextID::CREATE_PROJECT_ERROR_TITLE, "Import ADM file in to current project"},
      {TextID::CREATE_PROJECT_ERROR_DESCRIPTION, "Error: This file can not be imported."},
      {TextID::IMPORT_MENU, "ADM File to Import"},
      {TextID::IMPORT_ACTION_PREFIX, "Import ADM file using "},
      {TextID::IMPORT_ERROR_TITLE, "Error: This file can not be imported."},
      {TextID::IMPORT_ERROR_DESCRIPTION, "ADM Import"},
      {TextID::RENDERER_VALIDATING_DIALOG, "\r\nValidating Project Structure...\r\n\r\nPlease Wait..."},
      {TextID::RENDERER_EXPORT_SOURCE_PREFIX, "Using export source: \""},
      {TextID::RENDERER_ERROR_NO_EXPORT_SOURCES, ""},
      {TextID::RENDER_LOG_ERROR_HEADER, "\r\n\r\nERRORS:"},
      {TextID::RENDER_LOG_WARNING_HEADER, "\r\n\r\nWARNINGS:"},
      {TextID::RENDER_LOG_INFO_HEADER, "\r\nINFO:"},
      {TextID::RENDERER_ERROR_FAILED_CONTROL_OVERRIDE, "WARNING: Unable to takeover all render controls. REAPER version may be unsupported and render may fail.\r\n\r\n"},
      {TextID::EXPORT_DIALOG_TAKEOVER_TEXT_FOR_CHANNEL_COUNT, "Auto"},
      {TextID::EXPORT_DIALOG_FALLBACK_VALUE_FOR_CHANNEL_COUNT, "Stereo"},
      {TextID::EXPORT_HANDLER_ERROR_NO_VALID_SOURCES, "No Valid Export Sources!"},
      {TextID::EXPORT_HANDLER_ERROR_NO_CHANNELS, "Current configuration of export sources provides 0 channels of audio."},
      {TextID::EXPORT_HANDLER_WARNING_MULTIPLE_SOURCE_TYPES, "Multiple Types of Export Source!"},
      {TextID::EXPORT_SINK_ERROR_NO_SOURCES, "No suitable export sources within the current project.\r\nCan not continue with render."},
      {TextID::EXPORT_SINK_ERROR_NO_SOURCES_TITLE, "No Export Sources"},
      {TextID::EXPORT_SINK_ERROR_NO_CHANNELS, "The current configuration of export sources will export 0 channels of audio.\r\nCan not continue with render."},
      {TextID::EXPORT_SINK_ERROR_NO_CHANNELS_TITLE, "No Audio To Export"},
      {TextID::EXPORT_SINK_ERROR_SAMPLE_RATE, "Error: Sink sample rate ({}) does not match VST sample rate ({})!\r\nThis is likely an internal issue.\r\nCan not continue with render."},
      {TextID::EXPORT_SINK_ERROR_SAMPLE_RATE_TITLE, "Sample Rate Mismatch"},
      {TextID::EXPORT_SINK_ISSUES_HEADER, "Can not render due to the following issues:\r\n"},
      {TextID::EXPORT_SINK_ISSUES_TITLE, "Render Issues"},
      {TextID::EXPORT_SINK_ERROR_NO_ADM, "Error: No ADM metadata available for export.\r\nThis is likely an internal issue.\r\nCan not continue with render."},
      {TextID::EXPORT_SINK_ERROR_NO_ADM_TITLE, "No ADM Metadata"},
      {TextID::EXPORT_SINK_WARNING_FRAMES, "Warning:\r\nReceived {} frames from export source \"{}\".\r\nExpected {} frames."},
      {TextID::EXPORT_SINK_WARNING_FRAMES_TITLE, "Render"},
      {TextID::EXPORT_SINK_INFO_RENDERING,"Rendering ADM tracks..."},
      {TextID::EXPORT_SINK_WARNING_BAD_STRUCTURE, "WARNING: Unable to render - invalid project structure!"}

  };
  assert(dictionary.find(item) != dictionary.end());
  return(dictionary.find(item) == dictionary.end()) ? "" : dictionary[item];
}
