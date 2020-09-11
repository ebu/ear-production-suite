#include "pcmreader.h"
#include <bw64/bw64.hpp>

using namespace admplug;

namespace {
  // so we can use make_shared, friend wont work
  class PCMBlock_ : public PCMBlock {
  public:
      explicit PCMBlock_(std::size_t blockSize,
                         std::size_t channelCount,
                         std::size_t sampleRate) :
          PCMBlock{blockSize,
                   channelCount,
                   sampleRate} {}
  };

  constexpr std::size_t DEFAULT_BLOCK_SIZE{4096};
}

Bw64PCMReader::Bw64PCMReader(std::string fileName) : blockSize{DEFAULT_BLOCK_SIZE}
{
    reader = bw64::readFile(fileName);
}

Bw64PCMReader::~Bw64PCMReader() = default;

std::shared_ptr<IPCMBlock> Bw64PCMReader::read()
{
    auto block = std::make_shared<PCMBlock_>(blockSize, reader->channels(), reader->sampleRate());
    if(!reader->eof()) {
      block->frames = reader->read(&(block->blockData[0]), DEFAULT_BLOCK_SIZE);
    } else {
      block->frames = 0;
    }
    return block;
}

std::size_t admplug::Bw64PCMReader::totalFrames()
{
    return reader->numberOfFrames();
}

PCMBlock::PCMBlock(std::size_t blockSize,
                   std::size_t channelCount,
                   std::size_t sampleRate) : blockData(blockSize * channelCount, 0),
                                             channels{channelCount},
                                             rate{sampleRate}
{}

std::size_t PCMBlock::frameCount() const
{
    return frames;
}

std::size_t PCMBlock::channelCount() const
{
    return channels;
}

std::size_t PCMBlock::sampleRate() const
{
    return rate;
}

const std::vector<float> &PCMBlock::data() const
{
    return blockData;
}

PCMProcessBlock::PCMProcessBlock(std::vector<float> data,
                                 IPCMBlock const& inputBlock,
                                 std::size_t channelCount) :  blockData{data},
                                                              frames{inputBlock.frameCount()},
                                                              channels{channelCount},
                                                              rate{inputBlock.sampleRate()}
{  }

std::size_t PCMProcessBlock::frameCount() const
{
    return frames;
}

std::size_t PCMProcessBlock::channelCount() const
{
    return channels;
}

std::size_t PCMProcessBlock::sampleRate() const
{
    return rate;
}

const std::vector<float> &PCMProcessBlock::data() const
{
    return blockData;
}
