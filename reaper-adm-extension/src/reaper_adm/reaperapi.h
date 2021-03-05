#pragma once
#include <string>
#include <memory>
#include "reaperapivalues.h"
#include <optional>

#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <WDL/swell/swell-types.h>
#endif

class MediaTrack;
class MediaItem;
class MediaItem_Take;
class ReaProject;
class PCM_source;
class TrackEnvelope;

namespace admplug {

class Track;

class ReaperAPI {
   public:
       // From https://forum.cockos.com/showpost.php?p=2090533
       // The flag passed to Undo_EndBlock() controls how much of the project needs to be serialized
       // for the undo point. -1 means "everything changed," so everything gets serialized:
       // every track, media item, fx, the timeline, routing, and ARA state.
       // As you've seen, serializing the ARA state may take a while --
       // this is completely out of REAPER's control, all we can do is ask the plugin for its state.
       // If you know that your script can only have affected certain parts of the project,
       // you can pass that information as the flag argument to Undo_EndBlock which will make
       // it much more efficient.
       // So if your script may have affected tracks and media items only but not FX,
       // you could pass flag=5 (1+4) for example.
       // If your script may have affected everything but freeze state, automation items and ARA state,
       // you could pass flag=111 (1+2+4+8+32+64), etc.
       enum class UndoFlag : int {
          ALL              = -1,
          TRACK_CONFIG     = 1,   // track/master vol/pan/routing, routing/hwout envelopes too
          FX               = 2,   // track/master fx
          TRACK_ITEMS      = 4,   // track items
          MISC_CONFIG      = 8,   // loop selection, markers, regions, extensions
          FREEZE_STATE     = 16,  // freeze state
          TRACK_ENVELOPES  = 32,  // non-FX envelopes only
          FX_ENVELOPES     = 64,  // FX envelopes, implied by UNDO_STATE_FX too
          POOLED_ENVELOPES = 128, // contents of automation items -- not position, length, rate etc of automation items, which is part of envelope state
          FX_ARA_STATE     = 256  // ARA state
       };

    ReaperAPI();
    virtual ~ReaperAPI() = default;
    virtual const char* GetResourcePath() const = 0;
    virtual const char* GetExePath() const = 0;
    virtual void InsertTrackAtIndex(int index, bool wantDefaults) const = 0;
    virtual double GetMediaTrackInfo_Value(MediaTrack* tr, const char* parmname) const = 0;
    virtual bool GetTrackName(MediaTrack* track, char* buf, int buf_sz) const = 0;
    virtual bool SetMediaTrackInfo_Value(MediaTrack* tr, const char* parmname, double newvalue) const = 0;
    virtual void GetSetMediaTrackInfo_String(MediaTrack* track, const char* parameterName, char* newValue, bool setNewValue) const = 0;
    virtual void GetSetMediaItemInfo_String(MediaItem *item, const char *parameterName, char *newValue, bool setNewValue) const = 0;
    virtual void GetSetMediaItemTakeInfo_String(MediaItem_Take *take, const char *parameterName, char *newValue, bool setNewValue) const = 0;
    virtual MediaTrack* GetTrack(ReaProject* project, int trackIndex) const = 0;
    virtual MediaTrack* GetMediaItem_Track(MediaItem* item) const = 0;
    virtual bool GetUserFileNameForRead(char* filenameNeed4096, const char* title, const char* defext) const = 0;
    virtual int Register(std::string name, void *infostruct) const = 0;
    virtual int CountSelectedTracks(ReaProject* proj) const = 0;
    virtual MediaTrack* GetSelectedTrack(ReaProject* proj, int seltrackidx) const = 0;
    virtual int CountSelectedMediaItems(ReaProject* proj) const = 0;
    virtual int CountTracks(ReaProject* proj) const = 0;
    virtual MediaItem_Take* GetActiveTake(MediaItem* item) const = 0;
    virtual MediaItem* GetSelectedMediaItem(ReaProject* proj, int selitem) const = 0;
    virtual PCM_source* GetMediaItemTake_Source(MediaItem_Take* take) const = 0;
    virtual PCM_source* GetMediaSourceParent(PCM_source* src) const = 0;
    virtual void GetMediaSourceFileName(PCM_source* source, char* filenamebuf, int filenamebuf_sz) const = 0;
    virtual void GetProjectPath(char* buf, int buf_sz) const = 0;
    virtual void ShowConsoleMsg(const char* msg) const = 0;
    virtual MediaItem* AddMediaItemToTrack(MediaTrack* tr) const = 0;
    virtual MediaItem_Take* AddTakeToMediaItem(MediaItem* item) const = 0;
    virtual bool SetMediaItemTake_Source(MediaItem_Take* take, PCM_source* source) const = 0;
    virtual bool SetMediaItemTakeInfo_Value(MediaItem_Take* take, const char* parmname, double newvalue) const = 0;
    virtual bool SetMediaItemInfo_Value(MediaItem* item, const char* parmname, double newvalue) const = 0;
    virtual double GetMediaItemInfo_Value(MediaItem* item, const char* parmname) const = 0;
    virtual bool SetMediaItemLength(MediaItem* item, double length, bool refreshUI) const = 0;
    virtual bool SetMediaItemPosition(MediaItem* item, double position, bool refreshUI) const = 0;
    virtual void UpdateArrange() const = 0;
    virtual void UpdateTimeline() const = 0;
    virtual void Main_OnCommandEx(int command, int flag, ReaProject* proj) const = 0;
    virtual void SelectAllMediaItems(ReaProject* proj, bool selected) const = 0;
    virtual void SetMediaItemSelected(MediaItem* item, bool selected) const = 0;
    virtual double GetMediaSourceLength(PCM_source* source, bool* lengthIsQNOut) const = 0;
    virtual int GetMediaSourceNumChannels(PCM_source* source) const = 0;
    virtual PCM_source* PCM_Source_CreateFromFile(const char* filename) const = 0;
    virtual int TrackFX_AddByName(MediaTrack* track, const char* fxname, bool recFX, int instantiate) const = 0;
    virtual bool TrackFX_Delete(MediaTrack* track, int fx ) const = 0;
    virtual int TrackFX_GetCount(MediaTrack* track) const = 0;
    virtual bool TrackFX_GetFXName(MediaTrack* track, int fx, char* buf, int buf_sz) const = 0;
    virtual bool TrackFX_SetParam(MediaTrack* track, int fx, int param, double val) const = 0;
    virtual bool TrackFX_SetParamNormalized(MediaTrack* track, int fx, int param, double val) const = 0;
    virtual double TrackFX_GetParamNormalized(MediaTrack* track, int fx, int param) const = 0;
    virtual double TrackFX_GetParam(MediaTrack* track, int fx, int param, double* minvalOut, double* maxvalOut) const = 0;
    virtual bool TrackFX_GetEnabled(MediaTrack* track, int fx) const = 0;
    virtual bool TrackFX_GetOffline(MediaTrack* track, int fx) const = 0;
    virtual bool TrackFX_EndParamEdit(MediaTrack * track, int fx, int param) const = 0;
    virtual void TrackFX_SetEnabled(MediaTrack * track, int fx, bool enabled) const = 0;
    virtual MediaTrack* GetLastTouchedTrack() const = 0;
    virtual int CreateTrackSend(MediaTrack* tr, MediaTrack* desttrInOptional) const = 0;
    virtual int GetTrackNumSends(MediaTrack* tr, int category) const  = 0;
    virtual bool RemoveTrackSend(MediaTrack* tr, int category, int sendidx) const  = 0;
    virtual bool SetTrackSendInfo_Value(MediaTrack* tr, int category, int sendidx, const char* parmname, double newvalue) const = 0;
    virtual TrackEnvelope* GetFXEnvelope(MediaTrack* track, int fxindex, int parameterindex, bool create) const = 0;
    virtual int TrackFX_GetByName(MediaTrack* track, const char* fxname, bool instantiate) const = 0;
    virtual bool InsertEnvelopePoint(TrackEnvelope* envelope, double time, double value, int shape, double tension, bool selected, bool* noSortInOptional) const = 0;
    virtual bool DeleteEnvelopePointRange(TrackEnvelope* envelope, double time_start, double time_end) const = 0;
    virtual bool Envelope_SortPoints(TrackEnvelope* envelope) const = 0;
    virtual int ShowMessageBox(const char* msg, const char* title, int type) const = 0;
    virtual MediaTrack* GetMasterTrack(ReaProject* proj) const = 0;
    virtual int GetMasterTrackVisibility() const = 0;
    virtual int SetMasterTrackVisibility(int flag) const = 0;
    virtual TrackEnvelope* GetTrackEnvelopeByName(MediaTrack* track, const char* envname) const = 0;
    virtual void SetOnlyTrackSelected(MediaTrack* track) const = 0;
    virtual bool ReorderSelectedTracks(int beforeTrackIdx, int makePrevFolder) const = 0;
    virtual unsigned int GetSetTrackGroupMembership(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue) const = 0;
    virtual unsigned int GetSetTrackGroupMembershipHigh(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue) const = 0;
    virtual bool ValidatePtr(void* pointer, const char* ctypename) const = 0;
    virtual void SetTrackColor(MediaTrack* track, int color) const = 0;
    virtual int ColorToNative(int r, int g, int b) const = 0;
    virtual GUID* GetTrackGUID(MediaTrack* track) const = 0;
    virtual GUID* TrackFX_GetFXGUID(MediaTrack* track, int fxNum) const = 0;
    virtual ReaProject* EnumProjects(int index, char* projectFileName, int projectFilenameSize) const = 0;
    virtual void Undo_BeginBlock2(ReaProject* project) const = 0;
    virtual void Undo_EndBlock2(ReaProject* project, char const* changeDescription, int extraFlags) const = 0;
    virtual bool TrackFX_SetPreset(MediaTrack* track, int fx, const char* presetname) const = 0;
    virtual int TrackFX_GetPresetIndex(MediaTrack* track, int fx, int* numberOfPresetsOut) const = 0;
    virtual bool TrackFX_GetPreset(MediaTrack* track, int fx, char* presetname, int presetname_sz) const = 0;
    virtual int CountEnvelopePoints(TrackEnvelope* envelope) const = 0;
    virtual bool GetEnvelopePoint(TrackEnvelope* envelope, int ptidx, double* timeOutOptional, double* valueOutOptional, int* shapeOutOptional, double* tensionOutOptional, bool* selectedOutOptional) const = 0;
    virtual bool GetTrackUIVolPan(MediaTrack* track, double* volumeOut, double* panOut) const = 0;
    virtual int Envelope_Evaluate(TrackEnvelope* envelope, double time, double samplerate, int samplesRequested, double* valueOutOptional, double* dVdSOutOptional, double* ddVdSOutOptional, double* dddVdSOutOptional) const = 0;
    virtual bool GetEnvelopeStateChunk(TrackEnvelope* env, char* strNeedBig, int strNeedBig_sz, bool isundoOptional) const = 0;
    virtual bool SetEnvelopeStateChunk(TrackEnvelope* env, const char* str, bool isundoOptional ) const = 0;
    virtual void* GetSetTrackSendInfo(MediaTrack* tr, int category, int sendidx, const char* parmname, void* setNewValue) const = 0;
    virtual bool TrackFX_SetPinMappings(MediaTrack* tr, int fx, int isoutput, int pin, int low32bits, int hi32bits) const = 0;
    virtual int GetEnvelopeScalingMode(TrackEnvelope* env ) const = 0;
    virtual double ScaleFromEnvelopeMode(int scaling_mode, double val ) const = 0;
    virtual double ScaleToEnvelopeMode(int scaling_mode, double val ) const = 0;
    virtual int IsProjectDirty(ReaProject* proj) const = 0;
    virtual void Main_openProject(const char* name) const = 0;
    virtual double GetProjectLength(ReaProject* proj) const = 0;
    virtual int GetTrackNumMediaItems(MediaTrack* tr) const = 0;
    virtual MediaItem* GetTrackMediaItem(MediaTrack* tr, int itemidx) const = 0;
    virtual double GetSetProjectInfo(ReaProject * project, const char* desc, double value, bool is_set) const = 0;

    static constexpr int RENDER_ITEMS_AS_NEW_TAKE_ID = 41999;
    static constexpr int CROP_TO_ACTIVE_TAKE_IN_ITEMS = 40131;
    static constexpr int BUILD_MISSING_PEAKS = 40047;
    static constexpr int TOGGLE_VOLUME_ENVELOPE_ACTIVE = 40052;
    static constexpr int TOGGLE_VOLUME_ENVELOPE_VISIBLE = 40406; // also activates envelope

    //Custom Funcs
    virtual void UpdateArrangeForAutomation() const = 0;
    virtual int RouteTrackToTrack(MediaTrack *srcTrack, int srcChOffset, MediaTrack *dstTrack, int dstChOffset, int busWidth, I_SENDMODE sendMode, bool evenIfAlreadyRouted = false) const = 0;
    virtual void setTrackChannelCount(MediaTrack* track, int channelCount) const = 0;
    virtual void disableTrackMasterSend(MediaTrack* track) const = 0;
    virtual void setTrackName(MediaTrack* track, std::string name) const = 0;
    virtual void setItemName(MediaItem* item, std::string name) const = 0;
    virtual void setTakeName(MediaItem_Take* take, std::string name) const = 0;
    virtual void setTrackSendBusWidth(MediaTrack* track, int sendIndex, int busWidth) const = 0;
    virtual int addPluginToTrack(MediaTrack* track, const char* pluginName) const = 0;
    virtual MediaTrack* createTrackAtIndex(int index, bool fromEnd = false) const = 0;
    virtual TrackEnvelope* getPluginEnvelope(MediaTrack* track, const char* pluginName, int parameterIndex) const = 0;
    virtual void activateAndShowTrackVolumeEnvelope(MediaTrack* track) const = 0;
    virtual int getTrackIndexOfSelectedMediaItem(int selIndex = 0) const = 0;
    virtual void moveTrackToBeforeIndex(MediaTrack* trk, int index, TrackMoveMode moveMode = TrackMoveMode::Normal) const = 0;
    virtual void setAsVCAGroupMaster(MediaTrack* trk, int groupId) const = 0;
    virtual void setAsVCAGroupSlave(MediaTrack* trk, int groupId) const = 0;
    virtual std::unique_ptr<Track> firstTrackWithPluginNamed(std::string pluginName) const = 0;
    virtual std::unique_ptr<Track> createTrack() const = 0;
    virtual std::unique_ptr<Track> createTrack(MediaTrack* track) const = 0;
    virtual std::unique_ptr<Track> masterTrack() const = 0;
    virtual ReaProject* getCurrentProject() const = 0;
    virtual void resetFxPinMap(MediaTrack* trk, int fxNum) const = 0;
    virtual void mapFxPin(MediaTrack* trk, int fxNum, int trackChannel, int fxChannel) const = 0;
    virtual bool forceAmplitudeScaling(TrackEnvelope * trackEnvelope) const = 0;
    virtual std::optional<std::pair<double, double>> getTrackAudioBounds(MediaTrack* trk, bool ignoreBeforeZero) const = 0;
};
}
