//
// Created by Richard Bailey on 01/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#define EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#include <algorithm>
#include "log.hpp"
#include <memory>
#include <vector>
#include "item_store.hpp"
#include "programme_store.hpp"
#include "metadata_listener.hpp"
#include "helper/weak_ptr.hpp"

namespace ear::plugin {

class EventDispatcher {
 public:
  void dispatchEvent(std::function<void()> event);
 protected:
  virtual void doDispatch(std::function<void()> event) {}
};

class Metadata : private ProgrammeStoreListener,
                 private ItemStore::Listener {
 public:
  explicit Metadata(std::unique_ptr<EventDispatcher> uiDispatcher) :
      logger_{createLogger(fmt::format("Metadata Store@{}", (const void*)this))},
      uiDispatcher_{std::move(uiDispatcher)} {
      logger_->set_level(spdlog::level::trace);
    programmeStore.addListener(this);
    itemStore.addListener(this);
  }

  void addUIListener(std::weak_ptr<MetadataListener> listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    uiListeners_.push_back(std::move(listener));
  }

  void addListener(std::weak_ptr<MetadataListener> listener) {
      listeners_.push_back(std::move(listener));
  }

  template <typename F>
  void withProgrammeStore(F&& fn) {
    std::lock_guard<std::mutex> lock(mutex_);
    return fn(programmeStore);
  }

  template <typename F>
  void withItemStore(F&& fn) {
    std::lock_guard<std::mutex> lock(mutex_);
    return fn(itemStore);
  }

  std::pair<ItemMap, proto::ProgrammeStore> stores() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return{itemStore.get(), programmeStore.get()};
  }

  void refreshUI() {
    std::lock_guard<std::mutex> lock(mutex_);
    programmeStore.autoUpdateFrom(itemStore);
    fireEvent(&MetadataListener::notifyDataReset,
              programmeStore.get(),
              itemStore.get());
  }

 private:
  void addItems(ProgrammeStatus status, std::vector<proto::Object> const& items) override;
  void removeItem(ProgrammeStatus status, const proto::Object& element) override;
  void updateItem(ProgrammeStatus status, const proto::Object& element) override;
  void addProgramme(ProgrammeStatus status, const proto::Programme& element) override;
  void moveProgramme(Movement move, proto::Programme const& programme) override;
  void removeProgramme(int index) override;
  void selectProgramme(int index, proto::Programme const&) override;
  void updateProgramme(int index, const proto::Programme& programme) override;
  void changeStore(proto::ProgrammeStore const& store) override;
  void setAutoMode(bool enabled) override;


  template<typename F, typename... Args>
  void fireEvent(F const & fn, Args const&... args) {
    fireEventOnMsgThread(fn, args...);
    // TODO have a specific communication thread for metadata updates and
    // fire the events below on that.
    fireEventOnCurrentThread(fn, args...);
  }

  template<typename F, typename... Args>
  auto fireEventOnMsgThread(F&& f, Args&&... args) {
    // clear out any dead weak_ptrs
    removeDeadListeners(uiListeners_);

    // worth checking before copy as if UI is closed there's nothing to do
    if(!uiListeners_.empty()) {
        auto &listeners = uiListeners_;

        // dispatcher handles actual post of event on JUCE message thread
        // as needs to be in the plugin not library code
        uiDispatcher_->dispatchEvent(
                // copy all arguments and listener list once for thread safety, via
                // lambda capture list.
                // This happens on the thread that fires the event, all methods that
                // fire events hold the metadata lock so the copy is safe.
                [listeners, f, args...]() {
                    // The body of the lambda will be called on message thread
                    for (auto const &weak: listeners) {
                        if (auto listener = weak.lock()) {
                            // other than small parameters, all should be passed by const& to avoid
                            // copy per listener - i.e. the listener interface passes by const&
                            // invoke is just an easy way to generically call member functions
                            // on the listener
                            std::invoke(f, *listener, args...);
                        }
                    }
                }
        );
    }
  }

  template<typename F, typename... Args>
  auto fireEventOnCurrentThread(F const& f, Args const&... args) {
    removeDeadPointersAfter(listeners_,
                            [&f, &args...](auto const& listener) {
                              std::invoke(f, listener, args...);
                            });
  }

  static void removeDeadListeners(std::vector<std::weak_ptr<MetadataListener>>& listeners) {
    listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
                                   [](auto const& listener) {
                                     return listener.expired();
                                   }), listeners.end());
  }

  // ItemStoreListener
  void addItem(const proto::InputItemMetadata& item) override;
  void changeItem(const proto::InputItemMetadata& oldItem,
                  const proto::InputItemMetadata& newItem) override;
  void removeItem(const proto::InputItemMetadata& oldItem) override;
  void clearChanges() override;

  mutable std::mutex mutex_;
  std::shared_ptr<spdlog::logger> logger_;
  std::unique_ptr<EventDispatcher> uiDispatcher_;
  ProgrammeStore programmeStore;
  ItemStore itemStore;
  std::vector<std::weak_ptr<MetadataListener>> listeners_;
  std::vector<std::weak_ptr<MetadataListener>> uiListeners_;
};
}

#endif  // EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
