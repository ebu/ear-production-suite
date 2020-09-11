#pragma once
#include "importstatus.h"

namespace admplug {
class ImportReporter {
public:
    virtual ~ImportReporter() = default;
    virtual ImportStatus status() const = 0;
};
}
