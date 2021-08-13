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
      {TextID::IMPORT_ERROR_DESCRIPTION, "ADM Import"}
  };
  assert(dictionary.find(item) != dictionary.end());
  return(dictionary.find(item) == dictionary.end()) ? "" : dictionary[item];
}
