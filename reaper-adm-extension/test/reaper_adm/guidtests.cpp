#include <catch2/catch.hpp>
#include <gmock/gmock.h>
#include "reaperguid.h"

using namespace admplug;

TEST_CASE("ReaperGUID can be constructed from valid GUID string", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    REQUIRE_NOTHROW(ReaperGUID{uuid});
}

TEST_CASE("ReaperGUID throws when constructed from GUID string with missing separator", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7be25-13af17041db9}";
    REQUIRE_THROWS(ReaperGUID{uuid});
}

TEST_CASE("ReaperGUID throws when constructed from GUID string with too many characters", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-1d3af17041db9}";
    REQUIRE_THROWS(ReaperGUID{uuid});
}

TEST_CASE("ReaperGUID throws when constructed from GUID string with too few characters", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-3af17041db9}";
    REQUIRE_THROWS(ReaperGUID{uuid});
}

TEST_CASE("ReaperGUID throws when constructed from GUID string with invalid character", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13!f17041db9}";
    REQUIRE_THROWS(ReaperGUID{uuid});
}

TEST_CASE("ReaperGUIDs constructed from same string are equal", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    ReaperGUID guid{uuid};
    ReaperGUID otherGuid{uuid};
    REQUIRE(guid == otherGuid);
}

TEST_CASE("ReaperGUIDs constructed from different strings are not equal", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    std::string otherUuid = "{6d73241a-40b1-44c7-be25-13af17041db0}";
    ReaperGUID guid{uuid};
    ReaperGUID otherGuid{otherUuid};
    REQUIRE(guid != otherGuid);
}

TEST_CASE("ReaperGUIDs constructed from matching GUIDs are equal", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    ReaperGUID guid{uuid};
    auto guidPtr = guid.get();
    ReaperGUID lhs{guidPtr};
    ReaperGUID rhs{guidPtr};
    REQUIRE(lhs == rhs);
}

TEST_CASE("ReaperGUIDs constructed from  different GUIDs are not equal", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    std::string otherUuid = "{6d73241a-40b1-44c7-be25-13af17041db0}";
    ReaperGUID guid{uuid};
    ReaperGUID otherGuid{otherUuid};
    ReaperGUID lhs{guid.get()};
    ReaperGUID rhs{otherGuid.get()};
    REQUIRE(lhs != rhs);
}

TEST_CASE("ReaperGUID throws when constucted with nullptr", "[GUID]") {
    REQUIRE_THROWS(ReaperGUID{nullptr});
}

TEST_CASE("Copy constructed ReaperGUID is equal to original", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    ReaperGUID guid{uuid};
    ReaperGUID other{guid};
    REQUIRE(other == guid);
}

TEST_CASE("ReaperGUID can be assigned", "[GUID]") {
    std::string uuid = "{6d73241a-40b1-44c7-be25-13af17041db9}";
    std::string otherUuid = "{6d73241a-40b1-44c7-be25-13af17041dba}";
    ReaperGUID guid{uuid};
    ReaperGUID other{otherUuid};
    REQUIRE(other != guid);
    other = guid;
    REQUIRE(other == guid);
}
