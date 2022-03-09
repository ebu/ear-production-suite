//
// Created by Richard Bailey on 09/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_UI_EVENT_DISPATCHER_HPP
#define EAR_PRODUCTION_SUITE_UI_EVENT_DISPATCHER_HPP

#include "store_metadata.hpp"
#include "JuceHeader.h"
#include "helper/multi_async_updater.h"

namespace ear {
namespace plugin {
namespace ui {

class UIEventDispatcher : public EventDispatcher {
 protected:
  void doDispatch(std::function<void()> event) override {
    updater_.callOnMessageThread(std::move(event));
  }
 private:
  MultiAsyncUpdater updater_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear

#endif  // EAR_PRODUCTION_SUITE_UI_EVENT_DISPATCHER_HPP
