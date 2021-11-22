#include "blockbuilders.h"
namespace {
   std::chrono::nanoseconds toNs(double seconds) {
     return std::chrono::nanoseconds{static_cast<uint64_t>(1000000000 * seconds)};
   }

   adm::CartesianPosition getDefaultPosition() {
       auto position = adm::CartesianPosition{adm::X{0.0}, adm::Y{1.0}};
       position.set(adm::Z{0.0});
       return position;
   }
}

admplug::testing::CartesianCoordBlock::CartesianCoordBlock() : block{getDefaultPosition()} {
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withX(double x) {
    return setCartesianElement<adm::X>(x);
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withY(double y) {
    return setCartesianElement<adm::Y>(y);
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withZ(double z) {
    return setCartesianElement<adm::Z>(z);
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withDuration(double seconds) {
    CartesianCoordBlock builder{*this};
    builder.block.set(adm::Duration{std::chrono::nanoseconds(toNs(seconds))});
    return builder;
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withRtime(double seconds) {
    CartesianCoordBlock builder{*this};
    builder.block.set(adm::Rtime{toNs(seconds)});
    return builder;
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withJumpPosition(bool flag){
    CartesianCoordBlock builder{*this};
    builder.block.set(adm::JumpPosition{adm::JumpPositionFlag{flag}});
    return builder;
}

admplug::testing::CartesianCoordBlock admplug::testing::CartesianCoordBlock::withJumpPosition(bool flag, std::chrono::nanoseconds length){
    CartesianCoordBlock builder{*this};
    builder.block.set(adm::JumpPosition{adm::JumpPositionFlag{flag}, adm::InterpolationLength{length}});
    return builder;
}

admplug::testing::CartesianCoordBlock::operator adm::AudioBlockFormatObjects() const {
    return adm::AudioBlockFormatObjects{block};
}

admplug::testing::SphericalCoordBlock::SphericalCoordBlock() : block {adm::Azimuth{0.0},
                                                                      adm::Rtime{std::chrono::seconds{0}},
                                                                      adm::Duration{std::chrono::seconds{0}}}
{}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withAzimuth(double azimuth) {
    return setSphericalElement<adm::Azimuth>(azimuth);
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withElevation(double elevation) {
    return setSphericalElement<adm::Elevation>(elevation);
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withDistance(double distance) {
    return setSphericalElement<adm::Distance>(distance);
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withDuration(double seconds) {
    SphericalCoordBlock builder{*this};
    builder.block.set(adm::Duration{std::chrono::nanoseconds(toNs(seconds))});
    return builder;
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withRtime(double seconds) {
    SphericalCoordBlock builder{*this};
    builder.block.set(adm::Rtime{toNs(seconds)});
    return builder;
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withGain(double linearGain)
{
    SphericalCoordBlock builder{*this};
    builder.block.set(adm::Gain{adm::Gain::fromLinear(linearGain)});
    return builder;
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withJumpPosition(bool flag){
    SphericalCoordBlock builder{*this};
    builder.block.set(adm::JumpPosition{adm::JumpPositionFlag{flag}});
    return builder;
}

admplug::testing::SphericalCoordBlock admplug::testing::SphericalCoordBlock::withJumpPosition(bool flag, std::chrono::nanoseconds length){
    SphericalCoordBlock builder{*this};
    builder.block.set(adm::JumpPosition{adm::JumpPositionFlag{flag}, adm::InterpolationLength{length}});
    return builder;
}

admplug::testing::SphericalCoordBlock::operator adm::AudioBlockFormatObjects() const {
    return adm::AudioBlockFormatObjects{block};
}

admplug::testing::SphericalCoordBlock admplug::testing::initialSphericalBlock() {
    return SphericalCoordBlock{}.withDuration(0).withRtime(0);
}

admplug::testing::ObjectTypeBlockRange::ObjectTypeBlockRange() {

}

admplug::testing::ObjectTypeBlockRange admplug::testing::ObjectTypeBlockRange::with(const admplug::testing::ObjectTypeBlock &block) {
    ObjectTypeBlockRange range = *this;
    range.blocks.push_back(block);
    return range;
}

admplug::testing::ObjectTypeBlockRange admplug::testing::ObjectTypeBlockRange::followedBy(const admplug::testing::ObjectTypeBlock &block) {
    ObjectTypeBlockRange builder{*this};
    if(blocks.size() < 1) {
        return with(block);
    } else {
        auto previousBlock = blocks.at(blocks.size() - 1);
        auto nextStart = getValOrDefaultTime<adm::Rtime>(previousBlock).asNanoseconds() + getValOrDefaultTime<adm::Duration>(previousBlock).asNanoseconds();
        adm::AudioBlockFormatObjects nextBlock{block};
        nextBlock.set(adm::Rtime{adm::Time(nextStart)});
        auto duration = getValOrDefaultTime<adm::Duration>(nextBlock);
        nextBlock.set(adm::Duration{duration});
        builder.blocks.push_back(nextBlock);
    }
    return builder;
}

adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> admplug::testing::ObjectTypeBlockRange::asConstRange()
{
    return static_cast<adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects>>(*this);
}

admplug::testing::ObjectTypeBlockRange::operator adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects>() {
    return adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects>{blocks};
}

admplug::testing::CartesianCoordBlock admplug::testing::initialCartesianBlock()
{
    return CartesianCoordBlock{}.withDuration(0).withRtime(0);
}
