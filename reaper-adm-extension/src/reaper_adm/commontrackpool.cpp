#include <algorithm>
#include <cassert>

#include "admchannel.h"
#include "commontrackpool.h"
#include "reaperapi.h"
#include "track.h"

using namespace admplug;

CommonTrackPool::CommonTrackPool(ReaperAPI const& api) : api{api} {
}

Track &CommonTrackPool::trackFor(const ADMChannel &channel,
                                 std::function<void(Track&)> onCreate) {
    auto formatId = channel.formatId();
    if(auto trackIt = tracks.find(formatId); trackIt == std::end(tracks)) {
        auto track = api.createTrack();
        track->setName(channel.name());
        onCreate(*track);
        auto formatIdTrack = std::make_pair(channel.formatId(), std::move(track));
        auto [it, success] = tracks.insert(std::move(formatIdTrack));
        assert(success);
    }
    return *tracks.at(channel.formatId());
}


void CommonTrackPool::removeDeletedTracks()
{
    for(auto it = tracks.begin(); it != tracks.end(); ) {
        if(!it->second->stillExists()) {
            it = tracks.erase(it);
        } else {
            ++it;
        }
    }
}
