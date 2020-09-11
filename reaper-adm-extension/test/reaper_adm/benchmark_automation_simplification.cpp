#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <automationpoint.h>
using namespace admplug;

template<typename Fn>
auto bench(Fn f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end - start;
    return elapsed;
}

template<typename Fn, typename GeneratorFn>
auto runBenchOnce(Fn simplifyFn, GeneratorFn pointGen, std::size_t numPoints) {
    auto points = pointGen(numPoints);
    return bench(std::bind(simplifyFn, points));
}

float randBetween0_1and0_9() {
    static std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_real_distribution<float> distribution(0.1f, 0.9f);
    return distribution(generator);
}

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

std::vector<AutomationPoint> generateRandomPoints(std::size_t numberOfPoints) {
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    auto makeRand = std::bind(distribution, generator);
    std::vector<AutomationPoint> randomPoints;
    randomPoints.reserve(numberOfPoints);
    TimeInc timeInc;
    std::generate_n(std::back_inserter(randomPoints), numberOfPoints, [&makeRand, &timeInc](){
        auto [start, duration] = timeInc();
        return AutomationPoint{start, duration, makeRand()};
    });
    return randomPoints;
}

std::vector<AutomationPoint> generateDuplicatePoints(float value, std::size_t numberOfPoints) {
    return std::vector<AutomationPoint>(numberOfPoints, AutomationPoint{value});
}

std::vector<AutomationPoint> generatePointsInGroupsOfThree(std::size_t numberOfPoints) {
    std::vector<AutomationPoint> points;
    TimeInc timeInc;
    float val{0.f};
    for(auto i = 0u; i != numberOfPoints; ++i) {
        if(i % 3 == 0) {
            val = randBetween0_1and0_9();
        }
        auto [start, duration] = timeInc();
        points.emplace_back(start, duration, val);
    }
    return points;
}

bool approxEqual(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return fabs(rhs.value() - lhs.value()) < 0.00001f;
}

bool notApproxEqual(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return !approxEqual(lhs, rhs);
}

namespace admplug {
bool operator==(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return lhs.time() == rhs.time() && lhs.duration() == rhs.duration() && approxEqual(lhs, rhs);
}
}

bool before(AutomationPoint const& lhs, AutomationPoint const& rhs) {
    return lhs.time() < rhs.time();
}

void simplifyPointsOriginal(std::vector<AutomationPoint>& points) {
    int pointIndex = 1;
    while(pointIndex < points.size() && points.size() > 1) {
        auto& thisPoint = points[pointIndex];
        if(pointIndex == (points.size() - 1)) {
            // Only check against point before
            auto& beforePoint = points[pointIndex - 1];
            if(beforePoint.value() == thisPoint.value()) {
                // This point is redundant
                points.erase(points.begin() + pointIndex);
            } else {
                pointIndex++;
            }

        } else {
            // Check against neighbouring points.
            // TODO: Check if it sits on a linear path between neighbouring points... if it does, it's still redundant! Probably needs some tolerance logic due to FP error
            auto& beforePoint = points[pointIndex - 1];
            auto& afterPoint = points[pointIndex + 1];
            if(beforePoint.value() == thisPoint.value() && afterPoint.value() == thisPoint.value()) {
                // This point is redundant
                points.erase(points.begin() + pointIndex);
            } else {
                pointIndex++;
            }
        }
    }
}
void simplifyPointsViaUnique(std::vector<AutomationPoint>& points) {
    std::vector<AutomationPoint> firstVals;
    std::unique_copy(points.begin(), points.end(), std::back_inserter(firstVals), approxEqual);
    std::vector<AutomationPoint> lastVals;
    std::unique_copy(points.rbegin(), points.rend(), std::back_inserter(lastVals), approxEqual);
    std::vector<AutomationPoint> deduplicated;
    std::merge(firstVals.begin(), firstVals.end(), lastVals.rbegin(), lastVals.rend(), std::back_inserter(deduplicated), before);
    deduplicated.erase(std::unique(deduplicated.begin(), deduplicated.end()), deduplicated.end());
    if(deduplicated.size() > 1 && approxEqual(*(deduplicated.end() - 2), deduplicated.back())) {
        deduplicated.pop_back();
    }
    points = std::move(deduplicated);
}

void simplifyPointsViaFindIf(std::vector<AutomationPoint>& points) {
    using namespace std::placeholders;
    auto start = points.begin();
    while(start != points.end()) {
        auto firstNotEqual = std::find_if(start, points.end(), std::bind(notApproxEqual, *start, _1));
        if(std::distance(start, firstNotEqual) > 2) {
            start = points.erase(start + 1, firstNotEqual - 1) + 1;
        } else {
            start = firstNotEqual;
        }
    }
    if(points.size() > 1 && approxEqual(*(points.end() - 2), points.back())) {
        points.pop_back();
    }
}

void simplifyPointsCopy(std::vector<AutomationPoint>& points) {
    std::vector<AutomationPoint> filtered;
    if(!points.empty()) {
        filtered.push_back(points.front());
        bool previousCopied{true};
        for(auto currentPos = points.cbegin() + 1, end = points.cend(); currentPos != end; ++currentPos) {
            auto const& current = *currentPos;
            auto const& previous = *(currentPos - 1);
            if(current.value() != previous.value()) {
                if(!previousCopied) {
                    filtered.push_back(previous);
                }
                filtered.push_back(current);
                previousCopied = true;
            } else {
                previousCopied = false;
            }
        }
    }
    points = std::move(filtered);
}

namespace {
struct Accumulator {
    Accumulator(AutomationPoint first) :
        points{first}, previous{first} {
    }
    // this is naughty as operator+ doesn't
    // normally modify the lhs and return a ref,
    // but it avoids copies with std::accumulate.
    // in c++20 std::accumulate is move aware so could
    // do a normal operator+ instead.
    Accumulator& operator+(AutomationPoint const& point) {
        if(previous.value() != point.value()) {
            if(!previousCopied) {
                points.push_back(previous);
            }
            points.push_back(point);
            previousCopied = true;
        } else {
            previousCopied = false;
        }
        previous = point;
        return *this;
    }
    std::vector<AutomationPoint> points;
    AutomationPoint previous;
    bool previousCopied{true};
};
}

void simplifyPointsViaAccumulate(std::vector<AutomationPoint>& points) {
    if(!points.empty()) {
      auto acc = std::accumulate(points.cbegin()+1, points.cend(), Accumulator{points.front()});
      points = std::move(acc.points);
    }
}


void printResult(std::string benchName, std::chrono::nanoseconds elapsed, std::size_t numIterations) {
    std::cout << elapsed.count() / (numIterations * 1000.0 * 1000.0) << "ms: \t" << "average time taken over " << numIterations << " iterations of " << benchName << std::endl;
}

std::vector<AutomationPoint> generateSequence(std::vector<float> values) {
    TimeInc incrementor{};
    std::vector<AutomationPoint> points;
    points.reserve(values.size());
    std::transform(values.begin(), values.end(), std::back_inserter(points), [&incrementor](float value){
        auto[start, duration] = incrementor();
        return AutomationPoint(start, duration, value);
    });
    return points;
}

struct TestData {
    std::vector<float> inputValues;
    std::vector<int> expectedIndices;
    std::string description;
};

void print(std::vector<AutomationPoint> const& v) {
    std::cout << '[';
    for(auto it = v.begin(); it != v.end(); ++it) {
        std::cout << '{' << it->time() << ',' << it->duration() << ',' << it->value() << '}';
        if(it+1 != v.end()) {
            std::cout << ',';
        }
    }
    std::cout << ']';
}

template<typename Fn>
bool testMethod(TestData testData, Fn method, std::string name) {
    auto testInput = generateSequence(testData.inputValues);
    std::vector<AutomationPoint> expectedOutput;
    for(auto i : testData.expectedIndices) {
        expectedOutput.push_back(testInput.at(i));
    }
    method(testInput);
    auto success = testInput == expectedOutput;
    if(!success) {
        std::cout << "Test " << testData.description << " failed on implementation " << name << "\n";
        std::cout << "Input, Expected, Actual" << '\n';
        print(generateSequence(testData.inputValues));
        std::cout << "\n";
        print(expectedOutput);
        std::cout << "\n";
        print(testInput);
        std::cout << "\n" << "\n";
    }
    return success;
}

void testMethods() {
    std::vector<TestData> testData {
        {{},                             {},           "empty input"},
        {{0.f, 0.f},                     {0},          "redundant end point"},
        {{0.f, 0.f, 0.f},                {0},          ">1 redundant points"},
        {{0.f, 1.f, 0.f},                {0,1,2},      "all essential points"},
        {{0.f, 0.f, 1.f, 1.f, 0.f, 0.f}, {0,1,2,3,4},  "keep corner points but not redundant end point"},
        {{0.f, 1.f, 1.f, 1.f, 0.f},      {0,1,3,4},    "remove redundant mid point"}
    };

    for(auto& test : testData) {
        testMethod(test, simplifyPointsOriginal, "Original");
        testMethod(test, simplifyPointsViaUnique, "Unique");
        testMethod(test, simplifyPointsViaFindIf, "FindIf");
        testMethod(test, simplifyPointsCopy, "Copy");
        testMethod(test, simplifyPointsViaAccumulate, "Accumulate");
    }
}


template<typename Fn>
void runBenchmarks(std::string name, Fn fn) {
    using namespace std::placeholders;
    using namespace std::chrono_literals;
    auto const ITERATIONS = 100u;
    auto const POINT_COUNT = 10000u;

    auto randomTime = 0ns;
    for(auto i = 0u; i != ITERATIONS; ++i) {
        randomTime += runBenchOnce(fn, generateRandomPoints, POINT_COUNT);
    }

    auto duplicatesTime = 0ns;
    for(auto i = 0u; i != ITERATIONS; ++i) {
        duplicatesTime += runBenchOnce(fn, std::bind(generateDuplicatePoints, randBetween0_1and0_9(), _1), POINT_COUNT);
    }

    auto groupsOfThreeTime = 0ns;
    for(auto i = 0u; i != ITERATIONS; ++i) {
        groupsOfThreeTime += runBenchOnce(fn, generatePointsInGroupsOfThree, POINT_COUNT);
    }

    printResult(name + " implementation with random points", randomTime, ITERATIONS);
    printResult(name + " implementation with indentical points", duplicatesTime, ITERATIONS);
    printResult(name + " implementation with points in groups of three", groupsOfThreeTime, ITERATIONS);
}

int main() {
    testMethods();
    auto const POINT_COUNT = 10000u;
    std::cout << "With " << POINT_COUNT << " points:" << std::endl;
    runBenchmarks("original", simplifyPointsOriginal);
    runBenchmarks("copy_unique", simplifyPointsViaUnique);
    runBenchmarks("find_if", simplifyPointsViaFindIf);
    runBenchmarks("copy", simplifyPointsCopy);
    runBenchmarks("accumulate", simplifyPointsViaAccumulate);
}

