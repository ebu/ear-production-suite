#include "pcmwriterfactory.h"
#include "channelrouter.h"
#include "pcmwriter.h"
#include "filehelpers.h"
#include <sstream>
#include <fstream>

using namespace admplug;

namespace {
}

RoutingWriterFactory::RoutingWriterFactory()
{

}

std::unique_ptr<admplug::IPCMWriter> admplug::RoutingWriterFactory::createGroupWriter(const admplug::IPCMGroup &group, std::string path, std::string fnPrefix) const
{
    std::stringstream ss;

    ss << path << file::dirChar() << fnPrefix << "---"<< group.name() << ".wav";
    int incCount = 2;
    while (file::fileExists(ss.str())) {
        ss.str(std::string());
        ss.clear();
        ss << path << file::dirChar() << fnPrefix << "---" << group.name() << "(" << incCount++ << ").wav";
    }

    return std::make_unique<ChannelRouter>(std::make_unique<PCMWriter>(ss.str()), group.trackIndices());
}
