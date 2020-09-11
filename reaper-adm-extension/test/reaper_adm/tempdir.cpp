#include <boost/filesystem.hpp>
#include <stdexcept>
#include "tempdir.h"

using namespace boost;
using namespace admplug::test;

TempDir::TempDir()
{
    auto basePath = filesystem::temp_directory_path();
    if(!filesystem::is_directory(basePath)) {
        throw std::runtime_error("could not determine temp directory");
    }
    dirPath = basePath / filesystem::unique_path();
    filesystem::create_directory(dirPath);
    if(!(filesystem::is_directory(dirPath) && filesystem::is_empty(dirPath))) {
        throw std::runtime_error("problem creating unique temp directory");
    }
}

TempDir::~TempDir() {
    // error code is to ensure remove_all cannot throw
    boost::system::error_code err;
    filesystem::remove_all(dirPath, err);
}

filesystem::path TempDir::path() const
{
    return dirPath;
}


