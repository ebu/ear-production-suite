#include "admtypehelpers.h"
#include <adm/elements.hpp>

adm::TypeDescriptor admplug::getAdmType(const adm::AudioChannelFormat &obj) {
    if (obj.has<adm::TypeDescriptor>()) return obj.get<adm::TypeDescriptor>();
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioContent &obj) {
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioObject &obj) {
    return admplug::getAdmTypeFromPackRefs(obj.getReferences<adm::AudioPackFormat>());
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioPackFormat &obj) {
    if (obj.has<adm::TypeDescriptor>()) return obj.get<adm::TypeDescriptor>();
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioProgramme &obj) {
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioStreamFormat &obj) {
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioTrackFormat &obj) {
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmType(const adm::AudioTrackUid &obj) {
    return adm::TypeDefinition::UNDEFINED;
}

adm::TypeDescriptor admplug::getAdmTypeFromPackRefs(adm::ElementRange<const adm::AudioPackFormat> packs) {
    if (packs.size() > 0) {
        auto firstPack = *packs.begin();
        if (firstPack->has<adm::TypeDescriptor>()) {
            return firstPack->get<adm::TypeDescriptor>();
        }
        return adm::TypeDefinition::UNDEFINED;
    }
    return adm::TypeDefinition::UNDEFINED;
}

