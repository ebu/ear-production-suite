#pragma once
#include <memory>
#include <string>
#include <adm/elements_fwd.hpp>
#include <adm/adm.hpp>
#include <boost/variant/static_visitor.hpp>

namespace admplug {

template<typename T>
struct ElementTraits {
};

template<>
struct ElementTraits<adm::AudioContent> {
    using name_type = adm::AudioContentName;
};

template<>
struct ElementTraits<adm::AudioObject> {
    using name_type = adm::AudioObjectName;
};

template<>
struct ElementTraits<adm::AudioPackFormat> {
    using name_type = adm::AudioPackFormatName;
    using id_type = adm::AudioPackFormatId;
    constexpr static std::size_t id_length{11};
    constexpr static std::size_t id_identifier_offset{7};
    constexpr static std::size_t id_identifier_length{4};
};

template<>
struct ElementTraits<adm::AudioTrackUid> {
    using id_type = adm::AudioTrackUidId;
    constexpr static std::size_t id_length{12};
    constexpr static std::size_t id_identifier_offset{8};
    constexpr static std::size_t id_identifier_length{4};
};

template<>
struct ElementTraits<adm::AudioProgramme> {
    using name_type = adm::AudioProgrammeName;
};

template<>
struct ElementTraits<adm::AudioStreamFormat> {
    using name_type = adm::AudioStreamFormatName;
};

template<>
struct ElementTraits<adm::AudioTrackFormat> {
    using name_type = adm::AudioTrackFormatName;
};

template<>
struct ElementTraits<adm::AudioChannelFormat> {
    using name_type = adm::AudioChannelFormatName;
};

template<>
struct ElementTraits<adm::AudioBlockFormatDirectSpeakers> {
    using id_type = adm::AudioBlockFormatId;
    constexpr static std::size_t id_length{20};
    constexpr static std::size_t id_identifier_offset{7};
    constexpr static std::size_t id_identifier_length{4};
};

template<>
struct ElementTraits<adm::AudioBlockFormatHoa> {
    using id_type = adm::AudioBlockFormatId;
    constexpr static std::size_t id_length{20};
    constexpr static std::size_t id_identifier_offset{7};
    constexpr static std::size_t id_identifier_length{4};
};

template<typename T>
std::string getNameAsString(T const& element) {
    return element.template get<typename ElementTraits<T>::name_type>().get();
}

template<typename T>
std::string getIdAsString(T const& element) {
     return adm::formatId(element.template get<typename T::id_type>());
}

class AdmNameReader : boost::static_visitor<std::string> {
public:
    template<typename T>
    std::string operator()(std::shared_ptr<T const> element) const {
        return getNameAsString(*element);
    }
};

template<typename ElementT>
bool isCommonDefinition(ElementT const& element) {
    using IdType = typename ElementTraits<ElementT>::id_type;
    auto id = element.template get<IdType>();
    auto idString = adm::formatId(id);
    bool isCommon{false};
    //TODO: Swap out with getting the value straight away from AudioBlockFormatId
    if(idString.size() == ElementTraits<ElementT>::id_length) {
        auto identityDigits = idString.substr(ElementTraits<ElementT>::id_identifier_offset, ElementTraits<ElementT>::id_identifier_length);
        try {
          auto identityValue = std::stoi(identityDigits, 0, 16);
          isCommon = (identityValue < 0x1000);
        }
        catch(std::exception e) {/* invalid ID, so not a common definition */ };
    }
    return isCommon;
}

template<typename ElementT>
uint32_t getIdValueAsInt(ElementT const& element) {
    using IdType = typename ElementTraits<ElementT>::id_type;
    auto id = element.template get<IdType>();
    auto idString = adm::formatId(id);
    auto identityDigits = idString.substr(ElementTraits<ElementT>::id_identifier_offset, ElementTraits<ElementT>::id_identifier_length);
    return std::stoi(identityDigits, 0, 16);
}

template<>
std::string AdmNameReader::operator()(std::shared_ptr<adm::AudioTrackUid const>) const;
}

