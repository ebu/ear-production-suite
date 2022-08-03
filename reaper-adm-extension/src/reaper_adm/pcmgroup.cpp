#include "pcmgroup.h"
#include "channelindexer.h"
#include <adm/elements.hpp>

using namespace admplug;
using adm::AudioObject;
using adm::AudioTrackUid;

PCMGroup::PCMGroup(const IChannelIndexer &indexer, std::vector<std::shared_ptr<adm::AudioTrackUid const>> trackUids)
{
    assert(!trackUids.empty());

    fileName = "Unknown_UID";
    for(auto const& trackUid : trackUids) {
        fileName = adm::formatId(trackUid->get<adm::AudioTrackUidId>());
        break;
    }

    for(auto const& trackUid : trackUids) {
        int index = indexer.indexOf(trackUid); //returns -1 for missing from indexer... i.e, null or not found UID
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
