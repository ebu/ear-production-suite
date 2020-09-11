#include "channelrouter.h"
#include "pcmreader.h"
#include "pcmwriter.h"
#include <cassert>
using namespace admplug;


ChannelRouter::ChannelRouter(std::unique_ptr<IPCMWriter> pcmWriter, std::vector<int> channelIndices) : writer{std::move(pcmWriter)}, channelIndices{channelIndices}
{ }

void ChannelRouter::write(const IPCMBlock &block)
{
    std::vector<float> buffer(channelIndices.size() * block.frameCount(), 0);
    std::size_t outputChannelIndex = 0;
    for(auto channelIndex : channelIndices) {
        for(std::size_t frameNumber = 0;  frameNumber != block.frameCount(); ++frameNumber) {
            auto outputPosition = frameNumber * channelIndices.size() + outputChannelIndex;
            if(channelIndex < 0) {
                // -1 is used for undefined tracks - they need to produce a zerod take channel
                buffer[outputPosition] = 0.0;
            } else {
                auto inputPosition = frameNumber * block.channelCount() + static_cast<std::size_t>(channelIndex);
                buffer[outputPosition] = block.data()[inputPosition];
            }
        }
        outputChannelIndex += 1;
    }
    PCMProcessBlock outputBlock{buffer, block, channelIndices.size()};
    writer->write(outputBlock);
}

std::string ChannelRouter::fileName()
{
    return writer->fileName();
}

