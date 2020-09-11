#include "pcmgroup.h"
#include "channelindexer.h"
#include <adm/elements.hpp>

using namespace admplug;
using adm::AudioObject;
using adm::AudioTrackUid;

PCMGroup::PCMGroup(const IChannelIndexer &indexer, std::vector<ADMChannel> channels)
{
    assert(!channels.empty());

    fileName = "Unknown_UID";
    for(auto& channel : channels) {
        if(channel.trackUid()) {
            fileName = adm::formatId(channel.trackUid()->get<adm::AudioTrackUidId>());
            break;
        }
    }

    for(auto& channel : channels) {
        int index = indexer.indexOf(channel.trackUid()); //returns -1 for missing from indexer... i.e, null or not found UID
        indices.push_back(index);
    }
}

PCMGroup::PCMGroup(const IPCMGroup &other) : indices{other.trackIndices()}, fileName{other.name()}
{
}

PCMGroup& PCMGroup::operator=(const IPCMGroup& other) {
    indices = other.trackIndices();
    return *this;
}

const std::vector<int> &PCMGroup::trackIndices() const
{
    return indices;
}

std::string PCMGroup::name() const {
    return fileName;
}
