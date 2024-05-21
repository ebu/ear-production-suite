#include "include_gmock.h"
#include <catch2/catch_all.hpp>
#include <reaperapiimpl.h>

using namespace admplug;

TEST_CASE("Clean Plugin Name") {

    reaper_plugin_info_t info{};
    admplug::ReaperAPIImpl api(info);

    SECTION("Containing only developer segment") {
        std::string name = "VST3: EAR Object (EBU)";
        api.CleanFXName(name);
        REQUIRE(name == "EAR Object");
    }

    SECTION("Containing developer segment and channel count") {
        std::string name = "VST3: EAR Monitoring 0+2+0 (EBU) (64ch)";
        api.CleanFXName(name);
        REQUIRE(name == "EAR Monitoring 0+2+0");
    }

    SECTION("Already Clean") {
        std::string name = "EAR Object";
        api.CleanFXName(name);
        REQUIRE(name == "EAR Object");
    }
}