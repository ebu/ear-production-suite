#pragma once
#include <string>
#include <memory>

namespace bw64 {
    class Bw64Writer;
}

namespace admplug {

class IPCMBlock;

class IPCMWriter {
public:
    virtual ~IPCMWriter() = default;
    virtual void write(IPCMBlock const& block) = 0;
    virtual std::string fileName() = 0;
};

class PCMWriter : public IPCMWriter
{
public:
    PCMWriter(std::string fileName);
    ~PCMWriter() override;
    void write(IPCMBlock const& block) override;
    std::string fileName() override;
private:
    std::string name;
    std::unique_ptr<bw64::Bw64Writer> writer;
};

}
