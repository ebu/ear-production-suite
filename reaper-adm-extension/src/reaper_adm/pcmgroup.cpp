#include "pcmgroup.h"
#include <adm/elements.hpp>

using namespace admplug;
using adm::AudioObject;
using adm::AudioTrackUid;

PCMGroup::PCMGroup(std::vector<int> channelsOfOriginal)
{
    assert(!channelsOfOriginal.empty());

    fileName = "ch";
    fileName += std::to_string(channelsOfOriginal.front());
    for(int cooIndex = 1; cooIndex < channelsOfOriginal.size(); ++cooIndex) {
        fileName += "-";
        fileName += std::to_string(channelsOfOriginal[cooIndex]);
    }

    for(auto const& channelOfOriginal : channelsOfOriginal) {
        indices.push_back(channelOfOriginal);
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
