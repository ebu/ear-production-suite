#pragma once
#include <memory>
#include <string>
#include <adm/elements_fwd.hpp>
#include <adm/adm.hpp>
#include <boost/variant/static_visitor.hpp>

namespace admplug {

    adm::TypeDescriptor getAdmType(adm::AudioChannelFormat const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioContent const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioObject const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioPackFormat const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioProgramme const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioStreamFormat const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioTrackFormat const& obj);
    adm::TypeDescriptor getAdmType(adm::AudioTrackUid const& obj);

    adm::TypeDescriptor getAdmTypeFromPackRefs(adm::ElementRange<const adm::AudioPackFormat> packs);

    class AdmTypeReader : boost::static_visitor<adm::TypeDescriptor> {
    public:
        template <typename T>
        adm::TypeDescriptor operator()(T t) const {
            return admplug::getAdmType(*t);
        }
    };

}

