#pragma once
#include <memory>
#include <map>

namespace adm {
class Document;
class AudioChannelFormat;
class AudioTrackUid;
}
namespace bw64 {
class ChnaChunk;
}

namespace admplug {
class IADMMetaData;

class IChannelIndexer {
public:
   virtual ~IChannelIndexer() = default;
   virtual int indexOf(std::shared_ptr<adm::AudioTrackUid const> uid) const = 0;
};

class ChannelIndexer : public IChannelIndexer {
public:
    ChannelIndexer(IADMMetaData const& data);
    ~ChannelIndexer() override = default;
    /**
     * Return 0 based track index corresponding to TrackUid
     * N.B. Assumes 1-1 track->channel mapping.
     **/
    int indexOf(std::shared_ptr<adm::AudioTrackUid const> uid) const override;
private:
    std::map<std::shared_ptr<adm::AudioTrackUid const>, int> uIdChannels;
};

}
