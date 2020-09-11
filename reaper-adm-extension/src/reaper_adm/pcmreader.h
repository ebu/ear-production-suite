#pragma once
#include <string>
#include <memory>
#include <vector>

namespace bw64 {
  class Bw64Reader;
}

namespace admplug {

class IPCMBlock;

class PCMReader {
public:
    virtual ~PCMReader() = default;
    virtual std::shared_ptr<IPCMBlock> read() = 0;
    virtual std::size_t totalFrames() = 0;
};

class Bw64PCMReader : public PCMReader
{
public:
    Bw64PCMReader(std::string fileName);
    ~Bw64PCMReader();
    std::shared_ptr<IPCMBlock> read() override;
    std::size_t totalFrames() override;
private:
    std::unique_ptr<bw64::Bw64Reader> reader;
    std::size_t blockSize;
};

class IPCMBlock {
public:
    virtual std::size_t frameCount() const = 0;
    virtual std::size_t channelCount() const = 0;
    virtual std::size_t sampleRate() const = 0;
    virtual std::vector<float> const& data() const = 0;
    virtual ~IPCMBlock() = default;
protected:
    IPCMBlock() = default;
    IPCMBlock(IPCMBlock const&) = delete;
    const IPCMBlock& operator=(IPCMBlock const&) = delete;
};

class PCMBlock : public IPCMBlock
{
public:
    std::size_t frameCount() const override;
    std::size_t channelCount() const override;
    std::size_t sampleRate() const override;
    std::vector<float> const& data() const override;
    friend class Bw64PCMReader;
protected:
    explicit PCMBlock(std::size_t blockSize,
                      std::size_t channelCount,
                      std::size_t sampleRate);
private:
    std::vector<float> blockData;
    std::size_t frames;
    std::size_t channels;
    std::size_t rate;
};

class PCMProcessBlock : public IPCMBlock
{
public:
    PCMProcessBlock(std::vector<float> data,
                    const IPCMBlock &inputBlock,
                    std::size_t channelCount);
    std::size_t frameCount() const override;
    std::size_t channelCount() const override;
    std::size_t sampleRate() const override;
    std::vector<float> const& data() const override;
    friend class Bw64PCMReader;
private:
    std::vector<float> blockData;
    std::size_t frames;
    std::size_t channels;
    std::size_t rate;
};

}
