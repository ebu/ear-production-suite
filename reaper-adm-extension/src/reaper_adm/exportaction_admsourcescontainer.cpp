#include "exportaction_admsourcescontainer.h"
#include "pluginsuite_ear.h"

#include "exportaction_admsource-admvst.h"
#include "exportaction_admsource-earvst.h"

void AdmExportHandler::repopulate(ReaperAPI const & api)
{
    // Resets and reconstructs admExportVstSources and earSceneMasterVstSources
    admExportVstSources = std::make_shared<AdmVstExportSources>(api);
    earSceneMasterVstSources = std::make_shared<EarVstExportSources>(api);
}

IExportSources * AdmExportHandler::getAdmExportSources()
{
    // EAR plugins take priority.
    if(earSceneMasterVstSources && earSceneMasterVstSources->validForExport()) {
        return (IExportSources*)earSceneMasterVstSources.get();
    } else if(admExportVstSources && admExportVstSources->validForExport()) {
        return (IExportSources*)admExportVstSources.get();
    } else if(earSceneMasterVstSources && earSceneMasterVstSources->getAllFoundVsts()->size() > 0) {
        return (IExportSources*)earSceneMasterVstSources.get();
    } else if(admExportVstSources && admExportVstSources->getAllFoundVsts()->size() > 0) {
        return (IExportSources*)admExportVstSources.get();
    }
    return nullptr;
}

std::vector<std::string> AdmExportHandler::generateExportInfoStrings()
{
    // Gets info strings from chosen IExportSources
    auto admExportSources = getAdmExportSources();
    std::vector<std::string> msgs;

    if(admExportSources) {
        auto sourcesMsgs = admExportSources->generateExportInfoStrings();
        msgs.insert(msgs.end(), sourcesMsgs.begin(), sourcesMsgs.end());
    }

    return msgs;
}

std::vector<std::string> AdmExportHandler::generateExportErrorStrings()
{
    // Collates error strings from IExportSources and adds an additional one if there are no usable sources available
    auto admExportSources = getAdmExportSources();
    std::vector<std::string> msgs;

    if(!admExportSources) {
        msgs.push_back(std::string("No Valid Export Sources!"));
    }

    if(admExportSources) {
        if (admExportSources->getTotalExportChannels() == 0) {
            msgs.push_back("Current configuration of export sources provides 0 channels of audio.");
        }
        auto sourcesMsgs = admExportSources->generateExportErrorStrings();
        msgs.insert(msgs.end(), sourcesMsgs.begin(), sourcesMsgs.end());
    }

    return msgs;
}

std::vector<std::string> AdmExportHandler::generateExportWarningStrings()
{
    // Collates warning strings from IExportSources and adds an additional one if there are different types of IExportSources present
    auto admExportSources = getAdmExportSources();
    std::vector<std::string> msgs;

    if (admExportVstSources) {
        msgs.push_back(std::string("Support for FB360 and VISR plugin suites will be dropped in future versions of the EAR Production Suite."));
    }

    if(earSceneMasterVstSources && earSceneMasterVstSources->validForExport() && admExportVstSources && admExportVstSources->validForExport()) {
        msgs.push_back(std::string("Multiple Types of Export Source!"));
    }

    if(admExportSources) {
        auto sourcesMsgs = admExportSources->generateExportWarningStrings();
        msgs.insert(msgs.end(), sourcesMsgs.begin(), sourcesMsgs.end());
    }

    return msgs;
}
