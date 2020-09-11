#pragma once

#include <boost/filesystem.hpp>

namespace admplug {
namespace test {
class TempDir
{
public:
    TempDir();
    ~TempDir();
    boost::filesystem::path path() const;
private:
    boost::filesystem::path dirPath;
};
}
}



