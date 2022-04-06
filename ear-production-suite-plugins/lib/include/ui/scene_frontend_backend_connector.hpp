#pragma once

#include "communication/common_types.hpp"
#include "ear-plugin-base/export.h"
#include "ui/item_colour.hpp"
#include "programme_store.pb.h"
#include "input_item_metadata.pb.h"

#include <functional>
#include <string>

namespace ear {
namespace plugin {
namespace ui {

/**
 * This class bridges the gap between the
 * scene master plugin backend and the frontend (i.e. juce Processor and
 * Editor).
 *
 * The backend can use the `addItem` and `removeInput` methods
 * to inform the ui about newly registered input plugins.
 * It is ensured that the actual data changing operations (and thus any frontend
 * listeners attached) are called on the message thread.
 *
 * The UI can monitor this by adding a listener to the ValueTree returned by
 * `sceneData()`.
 *
 * If it becomes to confusing to identify which methods are to be used by the
 * backend or by the frontend we may consider splitting the API using to proxy
 * classes.
 *
 * Essentially, this is the sole interface between the juce part and the
 * non-juce parts of the plugin.
 *
 */
class EAR_PLUGIN_BASE_EXPORT SceneFrontendBackendConnector {
 public:

  virtual ~SceneFrontendBackendConnector() = default;
  enum class ItemType { OBJECTS };
  SceneFrontendBackendConnector(const SceneFrontendBackendConnector&) = delete;
  SceneFrontendBackendConnector(SceneFrontendBackendConnector&&) = delete;
  SceneFrontendBackendConnector& operator=(
      const SceneFrontendBackendConnector&) = delete;
  SceneFrontendBackendConnector& operator=(SceneFrontendBackendConnector&&) =
      delete;

 protected:
  SceneFrontendBackendConnector(){};

 private:
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
