#include <chrono>

#include "metadata.hpp"
#include "store_metadata.hpp"
#include "pending_store.hpp"
#include "programme_store_adm_populator.hpp"
#include <adm/parse.hpp>

namespace ear {
    namespace plugin {
        PendingStore::PendingStore(Metadata &metadata,
                                   std::string admStr) : data_(metadata) {
            populateFromAdm(admStr);
            timeoutThread = std::thread(&PendingStore::timeout, this, 3000);

        }

        PendingStore::~PendingStore()
        {
          killThread = true;
          timeoutThread.join(); // Wait for it to die
        }

        void PendingStore::timeout(int msLimit)
        {
          const int msCheckInterval = 10;
          int msWaited = 0;
          while(msWaited < msLimit) {
            if(killThread) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(msCheckInterval));
            // Not as accurate as comparing against a start time,
            // but this methodology used intentionally to perform same iterations even with debug/breaks
            msWaited += msCheckInterval;
            if(killThread) return;
          }
          finishPendingElementsSearch();
        }

        void PendingStore::populateFromAdm(const std::string &admStr) {
            auto iss = std::istringstream{std::move(admStr)};
            auto doc = adm::parseXml(iss, adm::xml::ParserOptions::recursive_node_search);
            pendingElements_ = populateStoreFromAdm(*doc, pendingStore_);
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

            auto ids = std::make_pair(
              adm::AudioObjectId(adm::AudioObjectIdValue(item.data.imported_ao_id())),
              adm::AudioTrackUidId(adm::AudioTrackUidIdValue(item.data.imported_atu_id())));
            auto range = pendingElements_.equal_range(ids);
            auto matchCount = std::distance(range.first, range.second);
            if (matchCount > 0) {
              for (auto el = range.first; el != range.second; ++el) {
                el->second->mutable_object()->set_connection_id(item.data.connection_id());
              }
              pendingElements_.erase(range.first, range.second);

              if (pendingElements_.empty()) {
                killThread = true;
                finishPendingElementsSearch();
              }
            }
          }
        }

        void PendingStore::finishPendingElementsSearch()
        {
          std::lock_guard lock(finisher);
          if(!finished) {
            finished = true;
            data_.setStore(pendingStore_);
          }
        }

    } // ear
} // plugin
