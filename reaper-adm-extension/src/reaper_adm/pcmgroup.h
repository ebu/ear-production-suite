#ifndef PCMGROUP_H
#define PCMGROUP_H
#include <vector>
#include <string>
#include <memory>

#include "admchannel.h"

namespace adm {
class AudioObject;
class AudioTrackUid;
}

namespace admplug {
class IChannelIndexer;

class IPCMGroup {
public:
    virtual ~IPCMGroup() = default;
    virtual std::vector<int> const& trackIndices() const = 0;
    virtual std::string name() const = 0;
};

inline bool operator== (IPCMGroup const& lhs, IPCMGroup const& rhs) {
    return lhs.trackIndices() == rhs.trackIndices();
}

inline bool operator!= (IPCMGroup const& lhs, IPCMGroup const& rhs) {
    return !(lhs == rhs);
}

inline bool operator< (IPCMGroup const& lhs, IPCMGroup const& rhs) {
    return lhs.trackIndices() < rhs.trackIndices();
}

inline bool operator> (IPCMGroup const& lhs, IPCMGroup const& rhs) {
    return lhs.trackIndices() > rhs.trackIndices();
}

class PCMGroup : public IPCMGroup
{
public:
    PCMGroup(const IChannelIndexer &indexer, std::vector<ADMChannel> channels);
    PCMGroup(const IPCMGroup& other);
    PCMGroup& operator=(IPCMGroup const& other);
    std::vector<int> const& trackIndices() const override;
    std::string name() const override;
private:
    std::vector<int> indices;
    std::string fileName;
};

}

#endif // PCMGROUP_H
