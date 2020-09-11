#pragma once

#include <bw64/bw64.hpp>

#include "reaperapi.h"
#include "reaper_plugin.h"
#include "exportaction_admsourcescontainer.h"

using namespace admplug;

class PCM_sink_adm : public PCM_sink
{
public:
    PCM_sink_adm(std::shared_ptr<ReaperAPI> api, const char *fn, void *cfgdata, int cfgdata_l, int nch, int srate, bool buildpeaks);
    ~PCM_sink_adm();
    void GetOutputInfoString(char *buf, int buflen);
    const char* GetFileName() { return admFilename; }
    int GetNumChannels() { return totalChannels; } // return number of channels
    double GetLength();  // length in seconds, so far
    INT64 GetFileSize();
    void WriteMIDI(MIDI_eventlist *events, int len, double samplerate) {} // N/A
    void WriteDoubles(double **samples, int len, int nch, int offset, int spacing);

    // MAYBE USEFUL OVERRIDES AT A LATER DATE
    //int GetLastSecondPeaks(int sz, ReaSample *buf);
    //void GetPeakInfo(PCM_source_peaktransfer_t *block);
    //virtual int Extended(int call, void *parm1, void *parm2, void *parm3) override;

private:
    std::shared_ptr<ReaperAPI> api;

    const char* admFilename;
    std::string admFilenameStr;
    std::unique_ptr<bw64::Bw64Writer> writer;
    int sRate;
    int totalChannels;

    std::shared_ptr<AdmExportHandler> admExportHandler;

    uint64_t expectedFramesWritten{ 0 };
    uint64_t actualFramesWritten{ 0 };

    void abortRender();
    int processNextFrames(uint64_t toMaxFrame);
    bool nextFrameReady();

    std::vector<float> aggregatedBlockBuffer; // predefined to prevent reserving memory on every call to process a block
    float *aggregatedBlockBufferStart{ nullptr }; // Faster than calling begin() everytime.
    float *aggregatedBlockBufferEnd{ nullptr }; // Faster than calling end() everytime.
    float *aggregatedBlockBufferPos{ nullptr }; // Current write position
    size_t aggregatedBlockBufferSize{ 0 };
    size_t aggregatedBlockBufferFrameCount{ 0 };
};

