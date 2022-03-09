#pragma once

#include "communication/common_types.hpp"
#include "input_item_metadata.pb.h"
#include <map>
#include <vector>
#include "helper/weak_ptr.hpp"

namespace ear {
namespace plugin {

struct InputItem {
  communication::ConnectionId id;
  proto::InputItemMetadata data;
};

using ItemMap = std::map<communication::ConnectionId, proto::InputItemMetadata>;

/*
 * Item store fully describes items provided by all connected input plugins
 * along with their configuration in those plugins.
 * It is not saved with the scene but populated by metadata from the input
 * plugins.
 */
class ItemStore {
 public:
  class Listener {
   protected:
    virtual void dispatch(std::function<void()> event);
    virtual void addItem(proto::InputItemMetadata const& item) = 0;
    virtual void changeItem(proto::InputItemMetadata const& oldItem,
                            proto::InputItemMetadata const& newItem) = 0;

    virtual void removeItem(proto::InputItemMetadata const& oldItem) = 0;
    virtual void clearChanges() = 0;
    friend class ItemStore;
   private:
    void itemAdded(proto::InputItemMetadata const& item);
    void itemChanged(proto::InputItemMetadata const& oldItem,
                     proto::InputItemMetadata const& newItem);

    void itemRemoved(proto::InputItemMetadata const& oldItem);
    void changesCleared();
  };

  [[ nodiscard ]] ItemMap get() const;
  [[ nodiscard ]] proto::InputItemMetadata getItem(communication::ConnectionId const& id) const;
  proto::InputItemMetadata const& get(communication::ConnectionId const& id) const;
  [[ nodiscard ]] std::map<communication::ConnectionId, proto::InputItemMetadata> allItems() const;
  std::multimap<int, communication::ConnectionId> routeMap() const;


  void setItem(communication::ConnectionId const& id,
               proto::InputItemMetadata const& item);
  void removeItem(communication::ConnectionId const& id);
  void clearChanged();
  void addListener(std::shared_ptr<Listener> const& listener);

 private:
  std::vector<std::weak_ptr<Listener>> listeners_;
  ItemMap store_;

  template <typename Fn>
  void fireEvent(Fn&& fn) {
    removeDeadPointersAfter(listeners_,
                            std::forward<Fn>(fn));
  }

  // note deliberate capture by value in inner lambda, this is so
  // we can dispatch asynchronously without the listener expiring or
  // synchronisation issues with the item store data
  void notifyItemAdded(proto::InputItemMetadata const& item) {
    removeDeadPointersAfter(listeners_,
                            [&item, this](auto const& listener) {
                              listener->dispatch([item, listener]{
                                listener->itemAdded(item);
                              });
                            });
  }

  void notifyItemChanged(proto::InputItemMetadata const& newItem,
                   proto::InputItemMetadata const& oldItem) {
    removeDeadPointersAfter(listeners_,
                            [&oldItem, &newItem, this](auto const& listener) {
                              listener->dispatch([oldItem, newItem, listener]{
                                listener->itemChanged(oldItem, newItem);
                              });
                            });
  }

  void notifyItemRemoved(proto::InputItemMetadata const& oldItem) {
    removeDeadPointersAfter(listeners_,
                            [&oldItem, this](auto const& listener) {
                              listener->dispatch([oldItem, listener]{
                                listener->itemRemoved(oldItem);
                              });
                            });
  }

};

//using ItemStore =
//    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
