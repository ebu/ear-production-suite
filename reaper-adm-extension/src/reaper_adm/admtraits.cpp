#include "admtraits.h"

template<>
std::string admplug::AdmNameReader::operator()(std::shared_ptr<const adm::AudioTrackUid>) const {
    return "";
}



