#include <stdexcept>
#include "importelement.h"
#include "pluginsuite.h"
#include "reaperapi.h"

using namespace admplug;

ImportElement::ImportElement(MediaItem *item) : item{item}
{

}

void ImportElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
    if(item) {
      mediaItemStart = api.GetMediaItemInfo_Value(item, "D_POSITION");
    }
}

bool ImportElement::hasAdmElement(adm::ElementConstVariant) const
{
    return false;
}

bool admplug::ImportElement::hasAdmElements(std::vector<adm::ElementConstVariant> elements) const
{
    return false;
}

double ImportElement::startTime() const
{
    return mediaItemStart;

}

double ImportElement::startOffset() const
{
    return mediaItemOffset;
}

std::vector<adm::ElementConstVariant> admplug::ImportElement::getAdmElements() const
{
    throw std::logic_error("This function should never be called as hasAdmElements overridden");
}
