#pragma once
#include <memory>
#include "pcmgroup.h"
#include "pcmwriter.h"

class PCM_source;

namespace admplug {

class ChannelRouter : public IPCMWriter {
public:
    ChannelRouter(std::unique_ptr<IPCMWriter>, std::vector<int> channelIndices);
    void write(IPCMBlock const& block) override;
    std::string fileName() override;
private:
    std::unique_ptr<IPCMWriter> writer;
    std::vector<int> channelIndices;
};

}
