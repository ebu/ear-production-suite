#include "backend_setup_timer.hpp"
#include "scene_plugin_processor.hpp"

namespace ear {
namespace plugin {

void BackendSetupTimer::timerCallback() {
  if (processor_) {
    try {
      processor_->setupBackend();
      stopTimer();
    } catch (...) {
    };
  }
};

}  // namespace plugin
}  // namespace ear