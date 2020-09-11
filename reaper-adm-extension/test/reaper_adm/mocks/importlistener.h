#pragma once
#include <gmock/gmock.h>
#include <progress/importlistener.h>

class MockImportListener : public admplug::ImportListener {
public:
    MOCK_METHOD1(setStatus, void(admplug::ImportStatus status));
    MOCK_METHOD0(elementAdded,  void());
    MOCK_METHOD0(elementCreated,  void());
    MOCK_METHOD1(totalFrames, void(uint64_t frames));
    MOCK_METHOD1(framesWritten, void(uint64_t frames));
    MOCK_METHOD1(error, void(const std::exception& e));
};

