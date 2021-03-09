#ifndef IMPORTLISTENER_H
#define IMPORTLISTENER_H

#include <stdexcept>
#include <memory>
#include <vector>

#include "importstatus.h"

namespace admplug {

class ImportListener {
public:
    virtual ~ImportListener() = default;
    virtual void setStatus(ImportStatus status) = 0;
    virtual void elementAdded() = 0;
    virtual void elementCreated() = 0;
    virtual void totalFrames(uint64_t frames) = 0;
    virtual void framesWritten(uint64_t frames) = 0;
    virtual void error(std::exception const& e) = 0;
    virtual void warning(const std::string& textToShow) = 0;
};

class ImportBroadcaster : public ImportListener {

public:
    virtual ~ImportBroadcaster() override = default;
    virtual void addListener(std::shared_ptr<ImportListener> listener);
    virtual void setStatus(ImportStatus status) override;
    virtual void elementAdded() override;
    virtual void elementCreated() override;
    virtual void totalFrames(uint64_t frames) override;
    virtual void framesWritten(uint64_t frames) override;
    virtual void error(const std::exception &e) override;
    virtual void warning(const std::string& textToShow) override;
private:
    std::vector<std::shared_ptr<ImportListener>> listeners;
};

}

#endif // IMPORTLISTENER_H
