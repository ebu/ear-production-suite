#pragma once

#include <map>
#include <vector>
#include "metadata.hpp"
#include "helper/weak_ptr.hpp"

namespace ear {
namespace plugin {
class Metadata;
/*
 * Item store fully describes items provided by all connected input plugins
 * along with their configuration in those plugins.
 * It is not saved with the scene but populated by metadata from the input
 * plugins.
 */
class ItemStore {
 public:
explicit ItemStore(Metadata& metadata) : metadata_{metadata} {}
  [[ nodiscard ]] ItemMap get() const;
  [[ nodiscard ]] proto::InputItemMetadata getItem(communication::ConnectionId const& id) const;
  [[ nodiscard ]] proto::InputItemMetadata const& get(communication::ConnectionId const& id) const;
  [[ nodiscard ]] std::map<communication::ConnectionId, proto::InputItemMetadata> allItems() const;
  [[ nodiscard ]] std::multimap<int, communication::ConnectionId> routeMap() const;


  void setItem(communication::ConnectionId const& id,
               proto::InputItemMetadata const& item);
  void removeItem(communication::ConnectionId const& id);
  void clearChanged();

 private:
  Metadata& metadata_;
  ItemMap store_;
};

//using ItemStore =
//    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
