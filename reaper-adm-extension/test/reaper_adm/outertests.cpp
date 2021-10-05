#include <memory>
#include <gmock/gmock.h>
#include <catch2/catch_all.hpp>
#include "admmetadata.h"

#include <adm/document.hpp>
#include <bw64/bw64.hpp>

using namespace admplug;
TEST_CASE("ADM file can be constructed from valid file") {
    auto data = ADMMetaData("data/channels_mono_adm.wav");
    REQUIRE(true);
}

TEST_CASE("Invalid ADM filename throws exception") {
    std::unique_ptr<ADMMetaData> data;
    REQUIRE_THROWS(data = std::make_unique<ADMMetaData>("junk"));
}

TEST_CASE("Valid ADM file") {
    std::string fileName{ "data/channels_mono_adm.wav" };
    auto data = ADMMetaData(fileName);
    SECTION("returns non-null chna()") {
        auto chna = data.chna();
        REQUIRE(chna != nullptr);
    }
    SECTION("returns non-null adm()") {
        auto adm = data.adm();
        REQUIRE(adm != nullptr);
    }
    SECTION("returns fileName used for construction") {
        auto name = data.fileName();
        REQUIRE(name == fileName);
    }
}

TEST_CASE("ADM metadata fills in trackuid links") {
    auto data = ADMMetaData("data/channels_mono_adm.wav");
    auto trackUIDs = data.adm()->getElements<adm::AudioTrackUid>();
    REQUIRE(trackUIDs.size() > 0);
    auto firstUid = *trackUIDs.begin();
    SECTION("To audiotrackformat") {
        auto trackFormatRef = firstUid->getReference<adm::AudioTrackFormat>();
        REQUIRE(trackFormatRef);
    }
    SECTION("To audiopackformat") {
        auto packFormatRef = firstUid->getReference<adm::AudioPackFormat>();
        REQUIRE(packFormatRef);
    }
}
