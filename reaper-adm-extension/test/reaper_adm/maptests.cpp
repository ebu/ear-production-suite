#include <chrono>
#include "include_gmock.h"
#include <catch2/catch_all.hpp>

#include <parameter.h>
#include <automationpoint.h>

using namespace admplug;
using Catch::Approx;

namespace {
AutomationPoint createPoint(double value) {
    auto zero = std::chrono::nanoseconds::zero();
    return {zero, zero, value};
}
}

TEST_CASE("Normalised value at start of 0-centred range is 0", "[Parameter]") {
    ParameterRange range{-30, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(-30));
    REQUIRE(normed.value() == Approx(0.0));
}

TEST_CASE("Normalised value in middle of 0-centred range is 0.5", "[Parameter]") {
    ParameterRange range{-30, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(0));
    REQUIRE(normed.value() == Approx(0.5));
}

TEST_CASE("Normalised value at end of 0-centred range is 1", "[Parameter]") {
    ParameterRange range{-30, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(30));
    REQUIRE(normed.value() == Approx(1.0));
}

TEST_CASE("Normalised value at start of positive range is 0", "[Parameter]") {
    ParameterRange range{0, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(0));
    REQUIRE(normed.value() == Approx(0.0));
}

TEST_CASE("Normalised value in middle of positive range is 0.5", "[Parameter]") {
    ParameterRange range{0, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(15));
    REQUIRE(normed.value() == Approx(0.5));
}

TEST_CASE("Normalised value at end of positive range is 1", "[Parameter]") {
    ParameterRange range{0, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(30));
    REQUIRE(normed.value() == Approx(1.0));
}

TEST_CASE("Normalised value at start of positive, offset range is 0.0", "[Parameter]") {
    ParameterRange range{10, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(10));
    REQUIRE(normed.value() == Approx(0.0));
}

TEST_CASE("Normalised value in middle of positive, offset range is 0.5", "[Parameter]") {
    ParameterRange range{10, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(20));
    REQUIRE(normed.value() == Approx(0.5));
}

TEST_CASE("Normalised value at end of positive, offset range is 1.0", "[Parameter]") {
    ParameterRange range{10, 30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(30));
    REQUIRE(normed.value() == Approx(1.0));
}

TEST_CASE("Normalised value at start of negative range is 0.0", "[Parameter]") {
    ParameterRange range{-30, -10};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(-30));
    REQUIRE(normed.value() == Approx(0.0));
}

TEST_CASE("Normalised value in middle of negative range is 0.5", "[Parameter]") {
    ParameterRange range{-30, -10};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(-20));
    REQUIRE(normed.value() == Approx(0.5));
}

TEST_CASE("Normalised value at end of negative range is 1.0", "[Parameter]") {
    ParameterRange range{-30, -10};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(-10));
    REQUIRE(normed.value() == Approx(1.0));
}

TEST_CASE("Normalised value at start of reversed range is 0.0", "[Parameter]") {
    ParameterRange range{30, -30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(30));
    REQUIRE(normed.value() == Approx(0.0));
}

TEST_CASE("Normalised value in middle of reversed range is 0.5", "[Parameter]") {
    ParameterRange range{30, -30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(0));
    REQUIRE(normed.value() == Approx(0.5));
}

TEST_CASE("Normalised value at end of reversed range is 1", "[Parameter]") {
    ParameterRange range{30, -30};
    auto normaliser = range.normaliser();
    auto normed = (*normaliser)(createPoint(-30));
    REQUIRE(normed.value() == Approx(1.0));
}
