#pragma once
#include <string>
#include <vector>
#include "reaper_plugin.h"
#include "reaperapi.h"

namespace admplug {

class ReaperAPIImpl : public ReaperAPI
{
public:
    ReaperAPIImpl(reaper_plugin_info_t& plugin_info);
    ~ReaperAPIImpl() override;
    const char* GetResourcePath() const override;
    const char* GetExePath() const override;
    void InsertTrackAtIndex(int index, bool wantDefaults) const override;
    double GetMediaTrackInfo_Value(MediaTrack* tr, const char* parmname) const override;
    bool GetTrackName(MediaTrack* track, char* buf, int buf_sz) const override;
    bool SetMediaTrackInfo_Value(MediaTrack *track, const char *parameterName, double newValue) const override;
    void GetSetMediaTrackInfo_String(MediaTrack *track, const char *parameterName, char *newValue, bool setNewValue) const override;
    void GetSetMediaItemInfo_String(MediaItem *item, const char *parameterName, char *newValue, bool setNewValue) const override;
    void GetSetMediaItemTakeInfo_String(MediaItem_Take *take, const char *parameterName, char *newValue, bool setNewValue) const override;
    MediaTrack* GetTrack(ReaProject *project, int trackIndex) const override;
    MediaTrack* GetMediaItem_Track(MediaItem* item) const override;
    virtual bool GetUserFileNameForRead(char* filenameNeed4096, const char* title, const char* defext) const override;
    int Register(std::string name, void *infostruct) const override;
    int CountSelectedTracks(ReaProject* proj) const override;
    MediaTrack* GetSelectedTrack(ReaProject* proj, int seltrackidx) const override;
    int CountSelectedMediaItems(ReaProject* proj) const override;
    int CountTracks(ReaProject* proj) const override;
    MediaItem_Take* GetActiveTake(MediaItem* item) const override;
    MediaItem* GetSelectedMediaItem(ReaProject* proj, int selitem) const override;
    PCM_source* GetMediaItemTake_Source(MediaItem_Take* take) const override;
    PCM_source* GetMediaSourceParent(PCM_source* src) const override;
    void GetMediaSourceFileName(PCM_source* source, char* filenamebuf, int filenamebuf_sz) const override;
    void GetProjectPath(char* buf, int buf_sz) const override;
    void ShowConsoleMsg(const char* msg) const override;
    MediaItem* AddMediaItemToTrack(MediaTrack* tr) const override;
    MediaItem_Take* AddTakeToMediaItem(MediaItem* item) const override;
    bool SetMediaItemTake_Source(MediaItem_Take* take, PCM_source* source) const override;
    bool SetMediaItemTakeInfo_Value(MediaItem_Take* take, const char* parmname, double newvalue) const override;
    bool SetMediaItemInfo_Value(MediaItem* item, const char* parmname, double newvalue) const override;
    double GetMediaItemInfo_Value(MediaItem* item, const char* parmname) const override;
    bool SetMediaItemLength(MediaItem* item, double length, bool refreshUI) const override;
    bool SetMediaItemPosition(MediaItem* item, double position, bool refreshUI) const override;
    void UpdateArrange() const override;
    void UpdateTimeline() const override;
    void Main_OnCommandEx(int command, int flag, ReaProject* proj) const override;
    bool AddExtensionsMainMenu() const override;
    void SelectAllMediaItems(ReaProject* proj, bool selected) const override;
    void SetMediaItemSelected(MediaItem* item, bool selected) const override;
    double GetMediaSourceLength(PCM_source* source, bool* lengthIsQNOut) const override;
    int GetMediaSourceNumChannels(PCM_source* source) const override;
    PCM_source* PCM_Source_CreateFromFile(const char* filename) const override;
    int TrackFX_AddByName(MediaTrack* track, const char* fxname, bool recFX, int instantiate) const override;
    bool TrackFX_Delete(MediaTrack* track, int fx ) const override;
    int TrackFX_GetCount(MediaTrack* track) const override;
    bool TrackFX_GetFXName(MediaTrack* track, int fx, char* buf, int buf_sz) const override;
    bool TrackFX_SetParam(MediaTrack* track, int fx, int param, double val) const override;
    bool TrackFX_SetParamNormalized(MediaTrack* track, int fx, int param, double val) const override;
    double TrackFX_GetParamNormalized(MediaTrack* track, int fx, int param) const override;
    double TrackFX_GetParam(MediaTrack* track, int fx, int param, double* minvalOut, double* maxvalOut) const override;
    bool TrackFX_GetEnabled(MediaTrack* track, int fx) const override;
    bool TrackFX_GetOffline(MediaTrack* track, int fx) const override;
    MediaTrack* GetLastTouchedTrack() const override;
    int CreateTrackSend(MediaTrack* tr, MediaTrack* desttrInOptional) const override;
    int GetTrackNumSends(MediaTrack* tr, int category) const override;
    bool RemoveTrackSend(MediaTrack* tr, int category, int sendidx) const override;
    bool SetTrackSendInfo_Value(MediaTrack* tr, int category, int sendidx, const char* parmname, double newvalue) const override;
    TrackEnvelope* GetFXEnvelope(MediaTrack* track, int fxindex, int parameterindex, bool create) const override;
    int TrackFX_GetByName(MediaTrack* track, const char* fxname, bool instantiate) const override;
    bool TrackFX_EndParamEdit(MediaTrack * track, int fx, int param) const override;
    void TrackFX_SetEnabled(MediaTrack * track, int fx, bool enabled) const override;
    bool InsertEnvelopePoint(TrackEnvelope* envelope, double time, double value, int shape, double tension, bool selected, bool* noSortInOptional) const override;
    bool DeleteEnvelopePointRange(TrackEnvelope* envelope, double time_start, double time_end) const override;
    bool Envelope_SortPoints(TrackEnvelope* envelope) const override;
    int ShowMessageBox(const char* msg, const char* title, int type) const override;
    MediaTrack* GetMasterTrack(ReaProject* proj) const override;
    int GetMasterTrackVisibility() const override;
    int SetMasterTrackVisibility(int flag) const override;
    TrackEnvelope* GetTrackEnvelopeByName(MediaTrack* track, const char* envname) const override;
    void SetOnlyTrackSelected(MediaTrack* track) const override;
    bool ReorderSelectedTracks(int beforeTrackIdx, int makePrevFolder) const override;
    unsigned int GetSetTrackGroupMembership(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue) const override;
    unsigned int GetSetTrackGroupMembershipHigh(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue) const override;
    void SetTrackColor(MediaTrack* track, int color) const override; //You've added this
    int ColorToNative(int r, int g, int b) const override;
    bool ValidatePtr(void* pointer, const char* ctypename) const override;
    GUID* GetTrackGUID(MediaTrack* track) const override;
    GUID* TrackFX_GetFXGUID(MediaTrack* track, int fxNum) const override;
    ReaProject* EnumProjects(int index, char* projectFileName, int projectFilenameSize) const override;
    void Undo_BeginBlock2(ReaProject* project) const override;
    void Undo_EndBlock2(ReaProject* project, char const* changeDescription, int extraFlags) const override;
    bool TrackFX_SetPreset(MediaTrack* track, int fx, const char* presetname) const override;
    int TrackFX_GetPresetIndex(MediaTrack* track, int fx, int* numberOfPresetsOut) const override;
    bool TrackFX_GetPreset(MediaTrack* track, int fx, char* presetname, int presetname_sz) const override;
    int CountEnvelopePoints(TrackEnvelope* envelope) const override;
    bool GetEnvelopePoint(TrackEnvelope* envelope, int ptidx, double* timeOutOptional, double* valueOutOptional, int* shapeOutOptional, double* tensionOutOptional, bool* selectedOutOptional) const override;
    bool GetTrackUIVolPan(MediaTrack* track, double* volumeOut, double* panOut) const override;
    int Envelope_Evaluate(TrackEnvelope* envelope, double time, double samplerate, int samplesRequested, double* valueOutOptional, double* dVdSOutOptional, double* ddVdSOutOptional, double* dddVdSOutOptional) const override;
    bool GetEnvelopeStateChunk(TrackEnvelope* env, char* strNeedBig, int strNeedBig_sz, bool isundoOptional) const override;
    bool SetEnvelopeStateChunk(TrackEnvelope* env, const char* str, bool isundoOptional ) const override;
    void* GetSetTrackSendInfo(MediaTrack* tr, int category, int sendidx, const char* parmname, void* setNewValue) const override;
    bool TrackFX_SetPinMappings(MediaTrack* tr, int fx, int isoutput, int pin, int low32bits, int hi32bits) const override;
    int GetEnvelopeScalingMode(TrackEnvelope* env ) const override;
    double ScaleFromEnvelopeMode(int scaling_mode, double val ) const override;
    double ScaleToEnvelopeMode(int scaling_mode, double val ) const override;
    int IsProjectDirty(ReaProject* proj) const override;
    void Main_openProject(const char* name) const override;
    double GetProjectLength(ReaProject* proj) const override;
    int GetTrackNumMediaItems(MediaTrack* tr) const override;
    MediaItem* GetTrackMediaItem(MediaTrack* tr, int itemidx) const override;
    double GetSetProjectInfo(ReaProject* project, const char* desc, double value, bool is_set) const override;
    const char* GetAppVersion() const override;
    const char* LocalizeString(const char* src_string, const char* section, int flagsOptional) const override;

    //Custom Funcs
    void UpdateArrangeForAutomation() const override;
    int RouteTrackToTrack(MediaTrack *srcTrack, int srcChOffset, MediaTrack *dstTrack, int dstChOffset, int busWidth, I_SENDMODE sendMode, bool evenIfAlreadyRouted = false) const override;
    void setTrackChannelCount(MediaTrack* track, int channelCount) const override;
    void disableTrackMasterSend(MediaTrack* track) const override;
    void setTrackName(MediaTrack* track, std::string name) const override;
    void setItemName(MediaItem* item, std::string name) const override;
    void setTakeName(MediaItem_Take* take, std::string name) const override;
    void setTrackSendBusWidth(MediaTrack* track, int sendIndex, int busWidth) const override;
    int addPluginToTrack(MediaTrack* track, const char* pluginName) const override;
    MediaTrack* createTrackAtIndex(int index, bool fromEnd = false) const override;
    TrackEnvelope* getPluginEnvelope(MediaTrack* track, GUID* pluginGuid, int parameterIndex) const override;
    void activateAndShowTrackVolumeEnvelope(MediaTrack* track) const override;
    int getTrackIndexOfSelectedMediaItem(int selIndex = 0) const override;
    void moveTrackToBeforeIndex(MediaTrack* trk, int index, TrackMoveMode moveMode = TrackMoveMode::Normal) const override;
    void setAsVCAGroupMaster(MediaTrack* trk, int groupId) const override;
    void setAsVCAGroupSlave(MediaTrack* trk, int groupId) const override;
    std::unique_ptr<Track> firstTrackWithPluginNamed(std::string pluginName) const override;
    std::unique_ptr<Track> createTrack() const override;
    std::unique_ptr<Track> createTrack(MediaTrack*) const override;
    std::unique_ptr<Track> masterTrack() const override;
    ReaProject* getCurrentProject() const override;
    void resetFxPinMap(MediaTrack* trk, int fxNum) const override;
    void mapFxPin(MediaTrack* trk, int fxNum, int trackChannel, int fxChannel) const override;
    bool forceAmplitudeScaling(TrackEnvelope * trackEnvelope) const override;
    std::optional<std::pair<double, double>> getTrackAudioBounds(MediaTrack* trk, bool ignoreBeforeZero) const override;

private:
    reaper_plugin_info_t& plugin_info;

    int reaperChannelOffsetForBusWidth(int busWidth) const;
    int toReaperChannelValue(int busWidth, int startChNum) const;
    int getSrcChannelValue(int busWidth, int startCh) const;
    int getDstChannelValue(int busWidth, int startCh) const;

};
}
