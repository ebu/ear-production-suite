//
// Created by Richard Bailey on 01/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#define EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#include <algorithm>
#include "log.hpp"
#include <memory>
#include <vector>
#include "metadata_listener.hpp"

namespace ear::plugin {

class EventDispatcher {
 public:
  void dispatchEvent(std::function<void()> event);
 protected:
  virtual void doDispatch(std::function<void()> event) {}
};

class Metadata {
 public:
    explicit Metadata(std::unique_ptr<EventDispatcher> uiDispatcher,
                      std::unique_ptr<EventDispatcher> backendDispatcher) :
            logger_{createLogger(fmt::format("Metadata Store@{}", (const void*)this))},
            uiDispatcher_{std::move(uiDispatcher)},
            backendDispatcher_{std::move(backendDispatcher)} {
        logger_->set_level(spdlog::level::trace);
        ensureDefaultProgrammePresent();
    }

  std::pair<proto::ProgrammeStore, ItemMap> stores() const;
  void refresh();
  void setDuplicateScene(bool isDuplicate);
  void setExporting(bool exporting);

  // Input item manipulation
  void setInputItemMetadata(communication::ConnectionId const& id,
                          proto::InputItemMetadata const& item);
  void removeInput(communication::ConnectionId const& id);

  // Programme manipulation
  void setStore(proto::ProgrammeStore const& store);
  void addProgramme();
  void removeProgramme(int index);
  void moveProgramme(int oldIndex, int newIndex);
  void selectProgramme(int index);
  void setAutoMode(bool enable);
  void setProgrammeName(int index, std::string const& name);
  void setProgrammeLanguage(int programmeIndex, std::string const& language);
  void clearProgrammeLanguage(int programmeIndex);
  void addItemsToSelectedProgramme(std::vector<communication::ConnectionId> const& id);
  void removeElementFromProgramme(int programmeIndex, communication::ConnectionId const& id);
  void moveElement(int programmeIndex, int oldIndex, int newIndex);
  void setElementOrder(int programmeIndex, std::vector<communication::ConnectionId> const& order);
  void updateElement(communication::ConnectionId const& id, proto::Object const& element);
  void sortProgrammeElements(int programmeIndex);

  // Queries
  int getSelectedProgrammeIndex();
  bool programmeHasElement(int programmeIndex, communication::ConnectionId const& id);
  bool getAutoMode();

  // Listeners
  void addUIListener(std::weak_ptr<MetadataListener> listener);
  void addBackendListener(std::weak_ptr<MetadataListener> listener);

 private:
  RouteMap routeMap() const;
  // ProgrammeStore callbacks
  void doAddItems(ProgrammeStatus status, std::vector<proto::Object> const& items);
  void doAddItemsToSelectedProgramme(std::vector<communication::ConnectionId> const& ids);
  void doSelectProgramme(int index, proto::Programme const& programme);

  // ItemStore callbacks
  void doChangeInputItem(const proto::InputItemMetadata& oldItem,
                         const proto::InputItemMetadata& newItem);

  void removeElementFromAllProgrammes(communication::ConnectionId const& id);
  void doRemoveElementFromProgramme(int programmeIndex, const communication::ConnectionId& id);

  void addProgrammeImpl(std::string const& name, std::string const& language);
  void ensureDefaultProgrammePresent();
  proto::Object* addObject(proto::Programme* programme, const communication::ConnectionId& id);
  void doSetElementOrder(int programmeIndex, std::vector<communication::ConnectionId> const& order);

  template<typename F, typename... Args>
  void fireEvent(F const & fn, Args const&... args) {
    fireEventWith(uiListeners_, *uiDispatcher_, fn, args...);
    fireEventWith(backendListeners_, *backendDispatcher_, fn, args...);
  }

  template<typename F, typename... Args>
  void fireEventWith(std::vector<std::weak_ptr<MetadataListener>>& listeners,
                     EventDispatcher& dispatcher,
                     F&& f, Args&&... args) {
    // clear out any dead weak_ptrs
    removeDeadListeners(listeners);

    // worth checking before copy as if UI is closed there's nothing to do
    if(!listeners.empty()) {
        auto &threadListeners = listeners;

        // dispatcher runs the passed lambda on some thread
        dispatcher.dispatchEvent(
                // copy all arguments and listener list once for thread safety, via
                // lambda capture list.
                // This happens on the thread that fires the event, all methods that
                // fire events hold the metadata lock so the copy is safe.
                [threadListeners, f, args...]() {
                    // The body of the lambda will be called asynchronously by the dispatcher
                    // after the lock is released
                    for (auto const &weak: threadListeners) {
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

  static void removeDeadListeners(std::vector<std::weak_ptr<MetadataListener>>& listeners) {
    listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
                                   [](auto const& listener) {
                                     return listener.expired();
                                   }), listeners.end());
  }

  mutable std::mutex mutex_;
  std::shared_ptr<spdlog::logger> logger_;
  std::unique_ptr<EventDispatcher> uiDispatcher_;
  std::unique_ptr<EventDispatcher> backendDispatcher_;
  proto::ProgrammeStore programmeStore_;
  ItemMap itemStore_;
  bool isDuplicateScene_{false};
  std::vector<std::weak_ptr<MetadataListener>> backendListeners_;
  std::vector<std::weak_ptr<MetadataListener>> uiListeners_;
};
}

#endif  // EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
