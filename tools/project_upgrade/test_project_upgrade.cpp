#include "upgrade.h"
#include <catch2/catch_all.hpp>
#include <sstream>

namespace {
// clang-format off
constexpr auto objInput =
    R"#(      <VST "VST3: EAR Object (EBU) (mono)" "EAR Object.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505320} ""
)#";
constexpr auto objOutput =
    R"#(      <VST "VST3: EAR Object (EBU) (mono)" "EAR Object.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505311} ""
)#";
constexpr auto dsInput =
    R"#(      <VST "VST3: EAR DirectSpeakers (EBU) (24ch)" "EAR DirectSpeakers.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505320} ""
)#";
constexpr auto dsOutput =
    R"#(      <VST "VST3: EAR DirectSpeakers (EBU) (24ch)" "EAR DirectSpeakers.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505310} ""
)#";
constexpr auto sceneInput =
    R"#(      <VST "VST3: EAR Scene (EBU) (64ch)" "EAR Scene.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505320} ""
)#";
constexpr auto sceneOutput =
    R"#(      <VST "VST3: EAR Scene (EBU) (64ch)" "EAR Scene.vst3" 0 "" 117098507{ABCDEF019182FAEB45425520455053FF} ""
)#";
constexpr auto monitorInput =
    R"#(      <VST "VST3: EAR Monitoring 0+2+0 (EBU)" "EAR Monitoring 0+2+0.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505320} ""
)#";
constexpr auto monitorOutput =
    R"#(      <VST "VST3: EAR Monitoring 0+2+0 (EBU)" "EAR Monitoring 0+2+0.vst3" 0 "" 117098507{ABCDEF019182FAEB45425520455053A0} ""
)#";

//clang-format on
}

TEST_CASE("Test Object plugin id replaced") {
  std::stringstream input{objInput};
  std::stringstream output;
  auto count = upgrade::upgrade(input, output);
  REQUIRE(count == 1);
  REQUIRE(output.str() == objOutput);
}

TEST_CASE("Test DirectSpeaker plugin id replaced") {
  std::stringstream input{dsInput};
  std::stringstream output;
  auto count = upgrade::upgrade(input, output);
  REQUIRE(count == 1);
  REQUIRE(output.str() == dsOutput);
}

TEST_CASE("Test Scene plugin id replaced") {
  std::stringstream input{sceneInput};
  std::stringstream output;
  auto count = upgrade::upgrade(input, output);
  REQUIRE(count == 1);
  REQUIRE(output.str() == sceneOutput);
}

TEST_CASE("Test monitor plugin id replaced") {
  std::stringstream input{monitorInput};
  std::stringstream output;
  auto count = upgrade::upgrade(input, output);
  REQUIRE(count == 1);
  REQUIRE(output.str() == monitorOutput);
}

TEST_CASE("Test random plugin not replaced") {
  auto const input{R"#(VST3: Some random plugin " "Rando.vst3" 0 "" 117098507{ABCDEF019182FAEB4542552045505320} ""
)#"};
  std::stringstream output;
  std::stringstream inStream{input};
  auto count = upgrade::upgrade(inStream, output);
  REQUIRE(count == 0);
  REQUIRE(output.str() == input);
}

TEST_CASE("Test already upgraded plugin not replaced") {
  std::stringstream output;
  std::stringstream inStream{objOutput};
  auto count = upgrade::upgrade(inStream, output);
  REQUIRE(count == 0);
  REQUIRE(output.str() == objOutput);
}