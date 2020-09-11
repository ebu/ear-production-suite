#pragma once
#include "projectelements.h"

namespace admplug {

class ImportElement : public RootElement {

public:
    explicit ImportElement(MediaItem *item = nullptr);
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api) override;
    virtual bool hasAdmElement(adm::ElementConstVariant element) const override;
    virtual bool hasAdmElements(std::vector<adm::ElementConstVariant> elements) const override;
    double startTime() const override;
    double startOffset() const override;
private:
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
    MediaItem* item;
    double mediaItemStart{0};
    double mediaItemOffset{0};

private:
};

}
