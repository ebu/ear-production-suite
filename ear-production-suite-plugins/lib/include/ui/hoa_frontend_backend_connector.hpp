#pragma once

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <string>
#include <mutex>
#include <functional>
#include "item_colour.hpp"
#include "ear-plugin-base/export.h"

namespace ear {
namespace plugin {
namespace ui {

/**
 * This class bridges the gap between the
 * input plugin backend and the frontend (i.e. juce Processor and Editor).
 *
 * Its API hides the UI implementation required to communicate state and data
 * changes into both
 * directions.
 *
 * Essentially, this is the sole interface between the juce part and the
 * non-juce parts of the plugin.
 *
 */
class EAR_PLUGIN_BASE_EXPORT HoaFrontendBackendConnector {
 public:
  using ParameterValue = boost::variant<float, bool, int, uint32_t,
                                        boost::optional<float>, std::string>;
  enum class ParameterId {
    ROUTING,
    NAME,
    COLOUR,
    HOA_TYPE//ME added this
    /* Old DS Code
    // Other params to be added here. Will need one for HOA type
    SPEAKER_SETUP_INDEX,
    */
  };

  using ParameterChangedCallback =
      std::function<void(ParameterId, ParameterValue)>;

  virtual ~HoaFrontendBackendConnector();

  void setStatusBarText(const std::string& text) {
    this->doSetStatusBarText(text);
  }

  /**
   * Register a callback to be called whenever on of the plugin parameters /
   * object metadata changes.
   *
   * If a callback has already been registered it will be replaced by the new
   * one.
   *
   * The callback might be called from any thread, most likely from the audio
   * real-time thread and the juce plugin messaging thread.
   */
  void onParameterChanged(ParameterChangedCallback callback);

  HoaFrontendBackendConnector(const HoaFrontendBackendConnector&) = delete;
  HoaFrontendBackendConnector& operator=(const HoaFrontendBackendConnector&) =
      delete;
  HoaFrontendBackendConnector(HoaFrontendBackendConnector&&) = delete;
  HoaFrontendBackendConnector& operator=(HoaFrontendBackendConnector&&) =
      delete;

 protected:
  HoaFrontendBackendConnector();
  virtual void doSetStatusBarText(const std::string& text) = 0;
  void notifyParameterChanged(ParameterId parameterID,
                              const ParameterValue& newValue);

 private:
  ParameterChangedCallback paramCallback_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
