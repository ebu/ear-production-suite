#pragma once
#include <memory>
#include <string>

namespace admplug {

class IPCMWriter;
class IPCMGroup;

class PCMWriterFactory
{
public:
    virtual ~PCMWriterFactory() = default;
    virtual std::unique_ptr<IPCMWriter> createGroupWriter(IPCMGroup const& group, std::string path, std::string fnPrefix) const = 0;
};

class RoutingWriterFactory : public PCMWriterFactory {
public:
    RoutingWriterFactory();
    std::unique_ptr<IPCMWriter> createGroupWriter(IPCMGroup const& group, std::string path, std::string fnPrefix) const override;

};

}
