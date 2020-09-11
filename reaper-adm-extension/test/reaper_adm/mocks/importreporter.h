#pragma once
#include <gmock/gmock.h>
#include <progress/importreporter.h>

class MockImportReporter : public admplug::ImportReporter {

public:
    MOCK_CONST_METHOD0(status, admplug::ImportStatus());
};
