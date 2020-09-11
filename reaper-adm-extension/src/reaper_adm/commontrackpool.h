#ifndef COMMONTRACKPOOL_H
#define COMMONTRACKPOOL_H

#include <memory>
#include <map>
#include <functional>

namespace admplug {

class Track;
class ADMChannel;
class ReaperAPI;


class CommonTrackPool
{
public:
    CommonTrackPool(ReaperAPI const& api);
    Track& trackFor(ADMChannel const& channel,
                    std::function<void(Track&)> onCreate);
    void removeDeletedTracks();
private:
    std::map<std::string, std::unique_ptr<Track>> tracks;
    ReaperAPI const& api;
};
}

#endif // COMMONTRACKPOOL_H
