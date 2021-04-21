#pragma once

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <string>
#include <functional>
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
class EAR_PLUGIN_BASE_EXPORT BinauralMonitoringFrontendBackendConnector {
 public:
  using ParameterValue = boost::variant<float, bool, int, uint32_t,
                                        boost::optional<float>, std::string>;
  enum class ParameterId {
    BYPASS,
    YAW,
    PITCH,
    ROLL,
    OSC_ENABLE,
    OSC_PORT,
  };

  using ParameterChangedCallback =
      std::function<void(ParameterId, ParameterValue)>;

  virtual ~BinauralMonitoringFrontendBackendConnector();

  /**
   * Register a callback to be called whenever one of the plugin parameters
   * changes.
   *
   * If a callback has already been registered it will be replaced by the new
   * one.
   *
   * The callback might be called from any thread, most likely from the audio
   * real-time thread and the juce plugin messaging thread.
   */
  void onParameterChanged(ParameterChangedCallback callback);

  BinauralMonitoringFrontendBackendConnector(const BinauralMonitoringFrontendBackendConnector&) =
      delete;
  BinauralMonitoringFrontendBackendConnector& operator=(
      const BinauralMonitoringFrontendBackendConnector&) = delete;
  BinauralMonitoringFrontendBackendConnector(BinauralMonitoringFrontendBackendConnector&&) = delete;
  BinauralMonitoringFrontendBackendConnector& operator=(
    BinauralMonitoringFrontendBackendConnector&&) = delete;

 protected:
   BinauralMonitoringFrontendBackendConnector();
  void notifyParameterChanged(ParameterId parameterID,
                              const ParameterValue& newValue);

 private:
  ParameterChangedCallback paramCallback_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
