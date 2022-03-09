//
// Created by Richard Bailey on 01/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#define EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
#include "item_store.hpp"
#include "programme_store.hpp"
#include "input_item_metadata.pb.h"
#include "programme_store.hpp"
#include "helper/weak_ptr.hpp"
#include <algorithm>
#include <vector>
#include <memory>

namespace ear::plugin {

struct ProgrammeObject {
  proto::Object programmeObject;
  proto::InputItemMetadata inputMetadata;
};

class ProgrammeObjects {
 public:
  ProgrammeObjects(ProgrammeStatus status,
                   proto::Programme const& programme,
                   ItemMap const& inputItems)
      : status{status},
        name_{programme.has_name() ? programme.name() : ""},
        hasLanguage_{programme.has_language()},
        language_{programme.has_language() ? programme.language() : ""} {
    data.reserve(programme.element_size());
    for (auto const& element : programme.element()) {
      if (element.has_object()) {
        auto id = element.object().connection_id();
        if (auto inputItemIt = inputItems.find(id);
            inputItemIt != inputItems.end()) {
          data.push_back({element.object(), inputItemIt->second});
        }
      }
    }
  }

  [[nodiscard]]
  int index() const {
    return status.index;
  }

  [[nodiscard]]
  bool isSelected() const {
    return status.isSelected;
  }

  [[nodiscard]]
  bool hasLanguage() const {
     return hasLanguage_;
  };

  std::string const& language() {
    return language_;
  }

  [[nodiscard]]
  std::optional<ProgrammeObject> dataItem(communication::ConnectionId const& id) const {
    auto it = std::find_if(data.begin(), data.end(), [this, &id](auto const& item) {
      return communication::ConnectionId(item.inputMetadata.connection_id()) == id;
    });
    std::optional<ProgrammeObject> item;
    if(it != data.end()) {
      item = *it;
    }
    return item;
  }
  using const_iterator = std::vector<ProgrammeObject>::const_iterator;
  [[nodiscard]]
  const_iterator begin() const {
    return data.begin();
  }
  [[nodiscard]]
  const_iterator end() const {
    return data.end();
  }
  [[nodiscard]]
  const_iterator cbegin() const {
    return data.cbegin();
  }
  [[nodiscard]]
  const_iterator cend() const {
    return data.cend();
  }
 private:
  std::string name_;
  bool hasLanguage_;
  std::string language_;
  ProgrammeStatus status;
  std::vector<ProgrammeObject> data;
};

class MetadataListener {
 public:
  virtual void dataReset(proto::ProgrammeStore const& programmes, ItemMap const& items) = 0;
  virtual void programmeAdded(int programmeIndex, proto::Programme const& programme) = 0;
  virtual void programmeRemoved(int programmeIndex) = 0;
  virtual void programmeUpdated(int programmeIndex, proto::Programme const& programme) = 0;
  virtual void programmeSelected(ProgrammeObjects const& objects) = 0;
  virtual void programmeMoved(Movement motion, proto::Programme const& programme) = 0;
  virtual void autoModeChanged(bool enabled) = 0;
  virtual void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& objects) = 0;
  virtual void itemRemovedFromProgramme(ProgrammeStatus status, ProgrammeObject const& object) = 0;
  virtual void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& object) = 0;
  virtual void inputAdded(InputItem const& item) = 0;
  virtual void inputRemoved(communication::ConnectionId const& id) = 0;
  virtual void inputUpdated(InputItem const& item) = 0;
};

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
      uiDispatcher_{std::move(uiDispatcher)} {}

  void addUIListener(std::weak_ptr<MetadataListener> listener) {
    uiListeners_.push_back(std::move(listener));
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
    return{itemStore.get(), programmeStore.get()};
  }

  void refreshUI() {
    fireEvent(&MetadataListener::dataReset,
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

    auto& listeners = uiListeners_;

    // dispatcher handles actual post of event on JUCE message thread
    // as needs to be in the plugin not library code
    uiDispatcher_->dispatchEvent(
        // copy all arguments and listener list once for thread safety, via
        // lambda capture list.
        // This happens on the thread that fires the event, all methods that
        // fire events hold the metadata lock so the copy is safe.
        [listeners, f, args...] () {
          // The body of the lambda will be called on message thread
          for(auto const& weak : listeners) {
            if(auto listener = weak.lock()) {
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

  void addItem(const proto::InputItemMetadata& item) override;
  void changeItem(const proto::InputItemMetadata& oldItem,
                  const proto::InputItemMetadata& newItem) override;
  void removeItem(const proto::InputItemMetadata& oldItem) override;
  void clearChanges() override;

  std::mutex mutex_;
  std::unique_ptr<EventDispatcher> uiDispatcher_;
  ProgrammeStore programmeStore;
  ItemStore itemStore;
  std::vector<std::weak_ptr<MetadataListener>> listeners_;
  std::vector<std::weak_ptr<MetadataListener>> uiListeners_;
};
}

#endif  // EAR_PRODUCTION_SUITE_STORE_METADATA_HPP
