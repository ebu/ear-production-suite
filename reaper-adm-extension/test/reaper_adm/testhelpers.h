#pragma once

#include <gmock/gmock.h>
#include <catch2/catch.hpp>
#include <memory>
#include <bw64/bw64.hpp>
#include "channelindexer.h"
#include "pcmreader.h"
#include "mocks/channelindexer.h"
#include "admmetadata.h"
#include "mocks/admmetadata.h"
#include <adm/document.hpp>

namespace testhelper {

inline std::shared_ptr<adm::AudioObject> createAudioObject(std::size_t numUids = 0) {
    auto object = adm::AudioObject::create(adm::AudioObjectName{"Test Object"});
    for(std::size_t i = 0; i != numUids; ++i) {
        auto trackUid = adm::AudioTrackUid::create();
        object->addReference(trackUid);
    }
    return object;
}

inline std::unique_ptr<::testing::NiceMock<admplug::MockIChannelIndexer const>> createMockIndexer(std::size_t returnValue) {
    auto mockIndexer = std::make_unique<::testing::NiceMock<admplug::MockIChannelIndexer const>>();
    ON_CALL(*mockIndexer, indexOf(::testing::_)).WillByDefault(::testing::Return(returnValue));
    return mockIndexer;
}

inline auto getIPMCBlockMatcherFn(std::size_t expectedSize, std::size_t expectedValue) {
    return [expectedSize, expectedValue](admplug::IPCMBlock const& block){
        REQUIRE(block.data().size() == expectedSize);
        for(auto& sample : block.data()) {
            REQUIRE(sample == Approx(expectedValue));
        }
    };
}
}
