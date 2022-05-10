//
// Created by Richard Bailey on 08/04/2022.
//

#include "metadata.hpp"
#include "store_metadata.hpp"
#include "pending_store.hpp"
#include "programme_store_adm_populator.hpp"
#include <adm/parse.hpp>

namespace ear {
    namespace plugin {
        PendingStore::PendingStore(Metadata &metadata,
                                   std::string admStr,
                                   std::vector<uint32_t> mappings) : data_(metadata) {
            populateFromAdm(admStr, mappings);
        }

        void PendingStore::populateFromAdm(const std::string &admStr, const std::vector<uint32_t> &mappings) {
            auto iss = std::istringstream{std::move(admStr)};
            auto doc = adm::parseXml(iss, adm::xml::ParserOptions::recursive_node_search);
            pendingElements_ = populateStoreFromAdm(*doc, pendingStore_, mappings);
        }

        void PendingStore::inputAdded(InputItem const & item, bool autoModeState)
        {
          checkAgainstPendingElements(item);
        }

        void PendingStore::inputUpdated(InputItem const & item, proto::InputItemMetadata const & oldItem)
        {
          checkAgainstPendingElements(item);
        }

        void PendingStore::checkAgainstPendingElements(InputItem const & item)
        {
          if(!finished) {
            auto range = pendingElements_.equal_range(item.data.routing());
            if (range.first != range.second) {
              for (auto el = range.first; el != range.second; ++el) {
                el->second->mutable_object()->set_connection_id(item.data.connection_id());
              }
              pendingElements_.erase(range.first, range.second);

              if (pendingElements_.empty()) {
                data_.setStore(pendingStore_);
                finished = true;
              }
            }
          }
        }

    } // ear
} // plugin
