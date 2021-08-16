#include "exportaction_pcmsink.h"

#include "adm/adm.hpp"
#include "ui_text.h"
#include <fmt/core.h>

#ifdef _WIN32
#include <Windows.h>
#else
#define Sleep(ms) sleep((double)ms / 1000.0)
#endif

#ifdef _MSC_VER
// Keep compiler happy using safe string func alternatives
#define strncpy(dst, src, dstlen) strncpy_s(dst, dstlen, src, dstlen-1)
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

using namespace admplug;

PCM_sink_adm::PCM_sink_adm(std::shared_ptr<ReaperAPI> api, const char *fn, void *cfgdata, int cfgdata_l, int nch, int srate, bool buildpeaks) : api{ api }, admFilename { fn }, sRate{ srate }
{
    admFilenameStr = std::string(fn);

    // Need to figure out if we're exporting just a time range
    double st = this->GetStartTime(); // TODO: is this used for bounds?

    // (re)Scan VSTs and states
    admExportHandler = std::make_shared<AdmExportHandler>();
    admExportHandler->repopulate(*api);

    auto admExportSources = admExportHandler->getAdmExportSources();

    if(!admExportSources) {
      api->ShowMessageBox(
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_SOURCES).c_str(),
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_SOURCES_TITLE).c_str(),
          0);
      abortRender();
        return;
    }

    // Check things that directly cause issues for the sink
    totalChannels = admExportSources->getTotalExportChannels();

    if (totalChannels == 0) {
      api->ShowMessageBox(
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_SOURCES).c_str(),
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_CHANNELS_TITLE).c_str(),
          0);
      abortRender();
        return;
    }

    if(sRate != admExportSources->getSampleRate())
    {
      auto msg =
          fmt::format(eps::uiText(eps::TextID::EXPORT_SINK_ERROR_SAMPLE_RATE),
                      sRate, admExportSources->getSampleRate());
      api->ShowMessageBox(
          msg.c_str(),
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_SAMPLE_RATE_TITLE).c_str(),
          0);
      abortRender();
      return;
    }

    // Check other errors (warnings/infos are fine... we can continue with those)
    auto errors = admExportHandler->generateExportErrorStrings();
    if(!errors.empty()) {
      auto op = eps::uiText(eps::TextID::EXPORT_HANDLER_ERROR_NO_VALID_SOURCES);
        for(auto& error : errors) {
            op.append("\r\n");
            op.append(error);
        }
        api->ShowMessageBox(
            op.c_str(),
            eps::uiText(eps::TextID::EXPORT_SINK_ISSUES_TITLE).c_str(), 0);
        abortRender();
        return;
    }

    // Check ADM
    auto doc = admExportSources->getAdmDocument();
    if(!doc) {
      api->ShowMessageBox(
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_ADM).c_str(),
          eps::uiText(eps::TextID::EXPORT_SINK_ERROR_NO_ADM_TITLE).c_str(), 0);
      abortRender();
      return;
    }

    // Configure ADM document
    if(admExportSources->documentRequiresProgrammeTitles()) {
        adm::AudioProgrammeName programmeName = adm::AudioProgrammeName("Programme"); // TODO: Pull from RenderDialogControl
        auto programmes = doc->getElements<adm::AudioProgramme>();
        for(auto& programme : programmes) {
            programme->set(programmeName);
        }
    }
    if(admExportSources->documentRequiresContentTitles()) {
        adm::AudioContentName contentName = adm::AudioContentName("Content"); // TODO: Pull from RenderDialogControl
        auto contents = doc->getElements<adm::AudioContent>();
        for(auto& content : contents) {
            content->set(contentName);
        }
    }

    // Start writing
    auto chna = admExportSources->getChnaChunk();
    auto axml = admExportSources->getAxmlChunk();
    writer = bw64::writeFile(admFilename, totalChannels, sRate, 24, chna, axml);

    // Start Renders
    admExportSources->setRenderInProgress(true);
}

PCM_sink_adm::~PCM_sink_adm()
{
    if (writer) {
        // Flush out data remaining in socket queues

        int loopCounter = 0;
        int loopLimit = 40; // 40*50ms = 2s - if they've not got through the queue by then, they're probably not coming!
        while (actualFramesWritten < expectedFramesWritten && loopCounter < loopLimit) {

            while(int frameCount = processNextFrames(expectedFramesWritten) > 0) {
                actualFramesWritten += frameCount;
            }
            if(actualFramesWritten == expectedFramesWritten) break;

            Sleep(50); // Give NNG I/O thread chance to work it's queue
            loopCounter++;
        }

        if (actualFramesWritten < expectedFramesWritten) {
          // Warn user - didn't receive all the frames we expected
          auto msg = fmt::format(
              eps::uiText(eps::TextID::EXPORT_SINK_WARNING_FRAMES),
              actualFramesWritten,
              admExportHandler->getAdmExportSources()->getExportSourcesName(),
              expectedFramesWritten);
          api->ShowMessageBox(
              msg.c_str(),
              eps::uiText(eps::TextID::EXPORT_SINK_WARNING_FRAMES_TITLE)
                  .c_str(),
              0);
        }
        writer.reset();
    }

    // Stop Renders
    admExportHandler->getAdmExportSources()->setRenderInProgress(false);
}

void PCM_sink_adm::abortRender() {
    // TODO - abortRender()
    // This is called by validation routines in the PCM sink constructor if validation fails.
    // This should therefore abort the render at this stage and thus cancel and close the 'rendering' dialog.
    // Currently unaware of any way to do this. See https://forum.cockos.com/showpost.php?p=2071597&postcount=392
    // If we do nothing, the rendering will appear to complete, but no file will be generated.
    // The validation routines display a message box indicating the problem.
}

double PCM_sink_adm::GetLength() // length in seconds, so far
{
    return 0.0;
}

INT64 PCM_sink_adm::GetFileSize()
{
    return 0;
}

void PCM_sink_adm::GetOutputInfoString(char *buf, int buflen) {
  if (writer) {
    strncpy(buf, eps::uiText(eps::TextID::EXPORT_SINK_INFO_RENDERING).c_str(),
            buflen);
  } else {
    strncpy(buf,
            eps::uiText(eps::TextID::EXPORT_SINK_WARNING_BAD_STRUCTURE).c_str(),
            buflen);
  }
}

void PCM_sink_adm::WriteDoubles(double **samples, int len, int nch, int offset, int spacing)
{
    if (!writer) return;

    // First call sets up aggregatedBlockBuffer (because we need to know the default block size, which should be len)
    if (aggregatedBlockBufferStart == nullptr){
        aggregatedBlockBufferFrameCount = len;
        aggregatedBlockBufferSize = aggregatedBlockBufferFrameCount * totalChannels;
        assert(aggregatedBlockBufferSize > 0);
        aggregatedBlockBuffer = std::vector<float>(aggregatedBlockBufferSize, 0.0);
        aggregatedBlockBufferStart = &aggregatedBlockBuffer.front();
    }

    expectedFramesWritten += len;

    //while(processNextFrames(expectedFrames) == 0) continue; // Ensures we do at least one, but this is slower overall as we're always waiting on the message queue whenever this is called.
    while(processNextFrames(expectedFramesWritten) > 0) continue; // Whilst we have stuff to process, keep processing them.
}

int PCM_sink_adm::processNextFrames(uint64_t toMaxFrame){

    float *bufferWritePos = aggregatedBlockBufferStart;
    size_t sampleSize = sizeof(float);
    int framesWrittenToBlockBuffer = 0;
    int frameWriteLimit = (int)aggregatedBlockBufferSize;

    if (toMaxFrame > 0) {
        frameWriteLimit = (int)min(frameWriteLimit, toMaxFrame - actualFramesWritten); // Shouldn't ever be negative, but the loop below would catch that anyway.
    }

    while (framesWrittenToBlockBuffer < frameWriteLimit && nextFrameReady()) {
        auto writeSuccess = admExportHandler->getAdmExportSources()->writeNextFrameTo(bufferWritePos, true); // true = skip frame availability check - we've already done it
        assert(writeSuccess);
        bufferWritePos += totalChannels;
        framesWrittenToBlockBuffer++;
    }

    if (framesWrittenToBlockBuffer > 0) {
        writer->write(aggregatedBlockBufferStart, framesWrittenToBlockBuffer);
        actualFramesWritten += framesWrittenToBlockBuffer;
    }

    return framesWrittenToBlockBuffer;
}


bool PCM_sink_adm::nextFrameReady() {
    return admExportHandler->getAdmExportSources()->isFrameAvailable();
}
