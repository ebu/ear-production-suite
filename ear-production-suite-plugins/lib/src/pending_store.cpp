#include <chrono>

#include "metadata.hpp"
#include "store_metadata.hpp"
#include "pending_store.hpp"
#include "programme_store_adm_populator.hpp"
#include <adm/parse.hpp>

namespace ear {
    namespace plugin {
        PendingStore::PendingStore(Metadata &metadata,
                                   std::string admStr,
                                   std::vector<PluginToAdmMap> pluginToAdmMaps) : data_(metadata) {
            populateFromAdm(admStr, pluginToAdmMaps);
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
          while(msSinceLastConnection < msLimit) {
            if(killThread) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(msCheckInterval));
            // Not as accurate as comparing against a start time,
            // but this methodology used intentionally to perform same iterations even with debug/breaks
            msSinceLastConnection += msCheckInterval;
            if(killThread) return;
          }
          finishPendingElementsSearch();
        }

        void PendingStore::populateFromAdm(const std::string &admStr, const std::vector<PluginToAdmMap> &pluginToAdmMaps) {
            auto iss = std::istringstream{std::move(admStr)};
            auto doc = adm::parseXml(iss, adm::xml::ParserOptions::recursive_node_search);
            pluginToAdmMaps_ = pluginToAdmMaps;
            pendingElements_ = populateStoreFromAdm(*doc, pendingStore_);
            if (pendingElements_.empty()) {
              // not waiting for anything
              killThread = true;
              finishPendingElementsSearch();
            }
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

            auto instanceId = item.data.input_instance_id();
            auto it = std::find_if(pluginToAdmMaps_.begin(), pluginToAdmMaps_.end(),
                         [instanceId](PluginToAdmMap& pluginToAdmMap) {
              if(pluginToAdmMap.inputInstanceId == 0) return false;
              return pluginToAdmMap.inputInstanceId == instanceId;
            });

            if(it != pluginToAdmMaps_.end()) {
              auto ids = std::make_pair(
                adm::AudioObjectId(adm::AudioObjectIdValue(it->audioObjectIdVal)),
                adm::AudioTrackUidId(adm::AudioTrackUidIdValue(it->audioTrackUidVal))
              );
              auto range = pendingElements_.equal_range(ids);
              auto matchCount = std::distance(range.first, range.second);
              if(matchCount > 0) {
                msSinceLastConnection = 0;
                for(auto el = range.first; el != range.second; ++el) {
                  el->second->mutable_object()->set_connection_id(item.data.connection_id());
                }
                pendingElements_.erase(range.first, range.second);

                if(pendingElements_.empty()) {
                  killThread = true;
                  finishPendingElementsSearch();
                }
              }
            }
          }
        }

        void PendingStore::finishPendingElementsSearch()
        {
          std::lock_guard lock(finisher);
          if(!finished) {
            finished = true;
            // Don't set store if we didn't get any high-level metadata (programmes)
            // Leave it (will remain with the initial default programme and in auto mode)
            if (pendingStore_.programme_size() > 0) {
              data_.setStore(pendingStore_);
            }
          }
        }

    } // ear
} // plugin
