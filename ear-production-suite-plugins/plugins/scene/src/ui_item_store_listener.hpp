//
// Created by Richard Bailey on 25/02/2022.
//

#ifndef EAR_PRODUCTION_SUITE_UI_ITEM_STORE_LISTENER_HPP
#define EAR_PRODUCTION_SUITE_UI_ITEM_STORE_LISTENER_HPP
#include <memory>
#include "item_store.hpp"
#include "JuceHeader.h"
#include "helper/multi_async_updater.h"

namespace ear::plugin::ui {
class UIItemStoreListener : public ItemStore::Listener {
 protected:
  void dispatch(std::function<void()> event) override {
    updater_.callOnMessageThread(std::move(event));
  }
 private:
  MultiAsyncUpdater updater_;
};
}

#endif  // EAR_PRODUCTION_SUITE_UI_ITEM_STORE_LISTENER_HPP
