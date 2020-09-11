#include "channelindexer.h"
#include <adm/document.hpp>
#include <bw64/chunks.hpp>
#include "admmetadata.h"

using namespace admplug;
using adm::AudioTrackUid;
using adm::AudioChannelFormatId;
using adm::Document;
using bw64::ChnaChunk;

ChannelIndexer::ChannelIndexer(const IADMMetaData &metaData)
{
  if(metaData.chna()) {
    auto audioUIds = metaData.chna()->audioIds();
    for(auto& id : audioUIds) {
      auto trackUidId = adm::parseAudioTrackUidId(id.uid());
      auto uid = metaData.adm()->lookup(trackUidId);
//      auto uids = metaData.adm()->getElements<adm::AudioTrackUid>();
//      auto uidIt = std::find_if(uids.begin(), uids.end(), [&trackUidId](std::shared_ptr<adm::AudioTrackUid const> uid) {
//          return uid->get<adm::AudioTrackUidId>() == trackUidId;

      if(uid != nullptr) {
        uIdChannels.emplace(uid, id.trackIndex());
      }
    }
  }
}

int ChannelIndexer::indexOf(std::shared_ptr<AudioTrackUid const> uid) const
{
  int channelIndex = -1;
  if(uid) {
      auto pairIt = uIdChannels.find(uid);
      if(pairIt != uIdChannels.end()) {
          channelIndex = pairIt->second - 1;
      }
  }
  return channelIndex;
}
