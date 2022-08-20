#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "routing_overlap.hpp"
#include "helper/common_definition_helper.h"

/*
namespace ear {
namespace plugin {

namespace {
// represet the routing of an input plugin
struct Routing {
  int startChannel;
  int size;
};

struct Overlap {
  std::size_t channel;
  std::unordered_set<std::string> ids;
};

// indices of returned vector are channel numbers
// for each channel, determine set of input IDs routed there
std::vector<std::unordered_set<std::string>> channelAllocation(
    const std::unordered_map<std::string, Routing>& routings) {
  std::vector<std::unordered_set<std::string>> allocation;
  for (auto const& [id, route] : routings) {
    // -1 routing used to represent unset state in plugin
    // ignore those as should never be rendered
    if (route.startChannel >= 0) {
      auto endIndex = route.startChannel + route.size;
      if (endIndex > allocation.size()) {
        allocation.resize(endIndex);
      }
      for (auto i = route.startChannel; i != endIndex; ++i) {
        allocation[i].insert(id);
      }
    }
  }
  return allocation;
}

// more than one ID per channel == overlap
bool hasOverlap(const std::unordered_set<std::string>& channelIds) {
  return channelIds.size() > 1;
}

// extract overlapping routes from vector of all alocations
std::vector<Overlap> getOverlaps(
    const std::unordered_map<std::string, Routing>& routings) {
  auto allocations = channelAllocation(routings);
  std::vector<Overlap> overlaps;
  for (std::size_t i = 0; i != allocations.size(); ++i) {
    if (hasOverlap(allocations[i])) {
      overlaps.push_back({i, allocations[i]});
    }
  }
  return overlaps;
}

// determine routings from scene store. 1 channel per ID for objects, n channels
// for directspeakers or hoa
std::unordered_map<std::string, Routing> getRoutings(
    proto::SceneStore const& store) {
  std::unordered_map<std::string, Routing> routings;
  auto const& items = store.monitoring_items();
  for (auto const& item : items) {
    int routeWidth = 1;
    if(item.has_ds_metadata()) {
      routeWidth = item.ds_metadata().speakers().size();
    } else if(item.has_hoa_metadata()) {
      auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
      auto hoaId = item.hoa_metadata().packformatidvalue();
      auto pfData = commonDefinitionHelper->getPackFormatData(4, hoaId);
      if (pfData) {
        routeWidth = pfData->relatedChannelFormats.size();
      }
    }
    routings.emplace(item.connection_id(), Routing{item.routing(), routeWidth});
  }
  return routings;
}

std::vector<Overlap> getOverlaps(const proto::SceneStore& store) {
  return getOverlaps(getRoutings(store));
}

// Possibly redundant now, added this to cache store id lookup when was
// doing this often in previous iteration, we still do it more than once
// so maybe worthwhile.
class ItemCache {
 public:
  explicit ItemCache(proto::SceneStore& store) : store{store} {}
  proto::MonitoringItemMetadata* get(std::string const& connectionId) {
    auto items = store.mutable_monitoring_items();
    if (auto it = cache.find(connectionId); it == cache.end()) {
      cache[connectionId] =
          &(*std::find_if(items->begin(), items->end(),
                          [connectionId](proto::MonitoringItemMetadata& item) {
                            return item.connection_id() == connectionId;
                          }));
    }
    return cache[connectionId];
  }

 private:
  proto::SceneStore& store;
  std::map<std::string, proto::MonitoringItemMetadata*> cache;
};

// if any of the items with ids in input set are flagged as changed, all should
// be updated in case that change was the routing. This could probably be
// improved
void setOverlapsAsChanged(std::set<std::string> const& overlappingIds,
                          proto::SceneStore& store) {
  ItemCache items{store};
  if (std::any_of(overlappingIds.begin(), overlappingIds.end(),
                  [&items](std::string const& id) {
                    return items.get(id)->changed();
                  })) {
    std::for_each(
        overlappingIds.begin(), overlappingIds.end(),
        [&items](std::string const& id) { items.get(id)->set_changed(true); });
  }
}
}  // namespace

// returns the set of connection IDs in the store that have overlapping routings
// may not all belong to same overlap set. (e.g. 4 inputs with 2 distinct
// overlaps)
std::set<std::string> getOverlapIds(const proto::SceneStore& store) {
  auto overlaps = getOverlaps(store);
  std::set<std::string> overlapIds;
  auto inserter = std::inserter(overlapIds, overlapIds.end());
  for (auto const& overlap : overlaps) {
    std::copy(overlap.ids.begin(), overlap.ids.end(), inserter);
  }
  return overlapIds;
}

// A blunt solution - a change in the set of overlapping ids flags all items
// with IDs in the previous and current overlap sets as changed
void flagChangedOverlaps(const std::set<std::string>& previousOverlaps,
                         const std::set<std::string>& currentOverlaps,
                         proto::SceneStore& store) {
  std::set<std::string> allOverlaps = currentOverlaps;
  // mark items that have had their routing resolved as changed
  // (potentially by a different item changing routing)
  // this is so the renderer will recalculate gain.
  for (auto const& overlap : previousOverlaps) {
    auto [it, success] = allOverlaps.insert(overlap);
    if (success) {
      auto items = store.mutable_monitoring_items();
      auto const& id = *it;
      auto itemIt =
          std::find_if(items->begin(), items->end(),
                       [&id](proto::MonitoringItemMetadata const& data) {
                         return data.connection_id() == id;
                       });
      if (itemIt != items->end()) {
        itemIt->set_changed(true);
      }
    }
  }
  setOverlapsAsChanged(allOverlaps, store);
}

}  // namespace plugin
}  // namespace ear
*/
