#include <automationpoint.h>
#include <admextraction.h>
#include <chrono>
#include <ostream>
#include <cmath>

namespace admplug {
std::ostream& operator<<(std::ostream& os, AutomationPoint const& point) {
    os << "{value: " << point.value() << ", start: " << point.time() << ", duration: " << point.duration() << "}";
    return os;
}

bool approxEqual(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return fabs(rhs.value() - lhs.value()) < 0.00001f;
}

bool operator==(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return lhs.time() == rhs.time() && lhs.duration() == rhs.duration() && approxEqual(lhs, rhs);
}
}

#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
using namespace admplug;

namespace {
struct TimeInc {
    using Ns = std::chrono::nanoseconds;
    using Ms = std::chrono::milliseconds;
    TimeInc() = default;
    explicit TimeInc(Ns interval) : duration{interval} {}
    Ns start{Ns::zero()};
    Ns duration{Ms{20}};
    std::pair<Ns, Ns> operator()() {
        auto time = std::make_pair(start, duration);
        start += duration;
        return time;
    }
};

class TestData {
public:
    std::vector<float> inputValues;
    std::vector<int> expectedIndices;
    std::string description;

    std::pair<std::vector<AutomationPoint>, std::vector<AutomationPoint>> getTestPoints() const {
        auto testInput = generateSequence(inputValues);
        std::vector<AutomationPoint> expectedOutput;
        for(auto i : expectedIndices) {
            expectedOutput.push_back(testInput.at(i));
        }
        return {testInput, expectedOutput};
    }

private:
    std::vector<AutomationPoint> generateSequence(std::vector<float> values) const {
        TimeInc incrementor{};
        std::vector<AutomationPoint> points;
        points.reserve(values.size());
        std::transform(values.begin(), values.end(), std::back_inserter(points), [&incrementor](float value){
            auto[start, duration] = incrementor();
            return AutomationPoint(start, duration, value);
        });
        return points;
    }
};

static std::vector<TestData> const testData {
        {{},                             {},           "empty input"},
        {{0.f, 0.f},                     {0},          "redundant end point"},
        {{0.f, 0.f, 0.f},                {0},          ">1 redundant points"},
        {{0.f, 1.f, 0.f},                {0,1,2},      "all essential points"},
        {{0.f, 0.f, 1.f, 1.f, 0.f, 0.f}, {0,1,2,3,4},  "keep corner points but not redundant end point"},
        {{0.f, 1.f, 1.f, 1.f, 0.f},      {0,1,3,4},    "remove redundant mid point"}
};

}

TEST_CASE("Test automation simplification") {
    for(auto const& test : testData) {
        auto [input, expected] = test.getTestPoints();
        REQUIRE(detail::simplify(input) == expected);
    }
}

