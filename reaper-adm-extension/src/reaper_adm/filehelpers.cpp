#include "filehelpers.h"
#include <fstream>

bool admplug::file::fileExists(std::string path) {
    std::ifstream file(path.c_str());
    return file.good();
}
