#pragma once

#include <map>
#include <vector>
#include "metadata.hpp"
#include "helper/weak_ptr.hpp"

namespace ear {
namespace plugin {

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
  [[ nodiscard ]] proto::InputItemMetadata const& get(communication::ConnectionId const& id) const;
  [[ nodiscard ]] std::map<communication::ConnectionId, proto::InputItemMetadata> allItems() const;
  [[ nodiscard ]] std::multimap<int, communication::ConnectionId> routeMap() const;


  void setItem(communication::ConnectionId const& id,
               proto::InputItemMetadata const& item);
  void removeItem(communication::ConnectionId const& id);
  void clearChanged();
  void addListener(Listener* listener);

 private:
  std::vector<Listener*> listeners_;
  ItemMap store_;

  template <typename Fn>
  void fireEvent(Fn&& fn) {
    for(auto listener : listeners_) {
      fn(listener);
    }
  }

};

//using ItemStore =
//    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
