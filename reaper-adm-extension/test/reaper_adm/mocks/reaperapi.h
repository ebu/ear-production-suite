#include <gmock/gmock.h>
#include <reaperapi.h>
#include "reaperapivalues.h"
#include "track.h"

namespace admplug {

class MockReaperAPI : public ReaperAPI {
 public:
  MOCK_CONST_METHOD0(GetResourcePath,
      const char*());
  MOCK_CONST_METHOD0(GetExePath,
      const char*());
  MOCK_CONST_METHOD2(InsertTrackAtIndex,
      void(int index, bool wantDefaults));
  MOCK_CONST_METHOD2(GetMediaTrackInfo_Value,
      double(MediaTrack* tr, const char* parmname));
  MOCK_CONST_METHOD3(GetTrackName,
      bool(MediaTrack* track, char* buf, int buf_sz));
  MOCK_CONST_METHOD3(SetMediaTrackInfo_Value,
      bool(MediaTrack* track, const char* parameterName, double newValue));
  MOCK_CONST_METHOD4(GetSetMediaTrackInfo_String,
      void(MediaTrack* track, const char* parameterName, char* newValue, bool setNewValue));
  MOCK_CONST_METHOD4(GetSetMediaItemInfo_String,
      void(MediaItem *item, const char* parameterName, char* newValue, bool setNewValue));
  MOCK_CONST_METHOD4(GetSetMediaItemTakeInfo_String,
      void(MediaItem_Take *take, const char* parameterName, char* newValue, bool setNewValue));
  MOCK_CONST_METHOD2(GetTrack,
      MediaTrack*(ReaProject* project, int trackIndex));
  MOCK_CONST_METHOD1(GetMediaItem_Track,
      MediaTrack*(MediaItem* item));
  MOCK_CONST_METHOD3(GetUserFileNameForRead,
      bool(char* filenameNeed4096, const char* title, const char* defext));
  MOCK_CONST_METHOD2(Register,
      int(std::string name, void *infostruct));
  MOCK_CONST_METHOD1(CountSelectedTracks,
      int(ReaProject* proj));
  MOCK_CONST_METHOD2(GetSelectedTrack,
      MediaTrack*(ReaProject* proj, int seltrackidx));
  MOCK_CONST_METHOD1(CountSelectedMediaItems,
      int(ReaProject* proj));
  MOCK_CONST_METHOD1(CountTracks,
      int(ReaProject* proj));
  MOCK_CONST_METHOD1(GetActiveTake,
      MediaItem_Take*(MediaItem* item));
  MOCK_CONST_METHOD2(GetSelectedMediaItem,
      MediaItem*(ReaProject* proj, int selitem));
  MOCK_CONST_METHOD1(GetMediaItemTake_Source,
      PCM_source*(MediaItem_Take* take));
  MOCK_CONST_METHOD1(GetMediaSourceParent,
      PCM_source*(PCM_source* src));
  MOCK_CONST_METHOD3(GetMediaSourceFileName,
      void(PCM_source* source, char* filenamebuf, int filenamebuf_sz));
  MOCK_CONST_METHOD2(GetProjectPath,
      void(char* buf, int buf_sz));
  MOCK_CONST_METHOD1(ShowConsoleMsg,
      void(const char* msg));
  MOCK_CONST_METHOD1(AddMediaItemToTrack,
      MediaItem*(MediaTrack* tr));
  MOCK_CONST_METHOD1(AddTakeToMediaItem,
      MediaItem_Take*(MediaItem* item));
  MOCK_CONST_METHOD2(SetMediaItemTake_Source,
      bool(MediaItem_Take* take, PCM_source* source));
  MOCK_CONST_METHOD3(SetMediaItemTakeInfo_Value,
      bool(MediaItem_Take* take, const char* parmname, double newvalue));
  MOCK_CONST_METHOD3(SetMediaItemInfo_Value,
    bool(MediaItem* item, const char* parmname, double newvalue));
  MOCK_CONST_METHOD2(GetMediaItemInfo_Value,
    double(MediaItem* item, const char* parmname));
  MOCK_CONST_METHOD3(SetMediaItemLength,
      bool(MediaItem* item, double length, bool refreshUI));
  MOCK_CONST_METHOD3(SetMediaItemPosition,
      bool(MediaItem* item, double position, bool refreshUI));
  MOCK_CONST_METHOD0(UpdateArrange,
      void());
  MOCK_CONST_METHOD0(UpdateTimeline,
    void());
  MOCK_CONST_METHOD3(Main_OnCommandEx,
      void(int command, int flag, ReaProject* proj));
  MOCK_CONST_METHOD0(AddExtensionsMainMenu,
      bool());
  MOCK_CONST_METHOD2(SelectAllMediaItems,
      void(ReaProject* proj, bool selected));
  MOCK_CONST_METHOD2(SetMediaItemSelected,
      void(MediaItem* item, bool selected));
  MOCK_CONST_METHOD2(GetMediaSourceLength,
      double(PCM_source* source, bool* lengthIsQNOut));
  MOCK_CONST_METHOD1(GetMediaSourceNumChannels,
      int(PCM_source* source));
  MOCK_CONST_METHOD1(PCM_Source_CreateFromFile,
      PCM_source*(const char* filename));
  MOCK_CONST_METHOD4(TrackFX_AddByName,
      int(MediaTrack* track, const char* fxname, bool recFX, int instantiate));
  MOCK_CONST_METHOD2(TrackFX_Delete,
      bool(MediaTrack* track, int fx ));
  MOCK_CONST_METHOD1(TrackFX_GetCount,
      int(MediaTrack* track));
  MOCK_CONST_METHOD4(TrackFX_GetFXName,
      bool(MediaTrack* track, int fx, char* buf, int buf_sz));
  MOCK_CONST_METHOD4(TrackFX_SetParam,
      bool(MediaTrack* track, int fx, int param, double val));
  MOCK_CONST_METHOD4(TrackFX_SetParamNormalized,
      bool(MediaTrack* track, int fx, int param, double val));
  MOCK_CONST_METHOD3(TrackFX_GetParamNormalized,
      double(MediaTrack* track, int fx, int param));
  MOCK_CONST_METHOD5(TrackFX_GetParam,
      double(MediaTrack* track, int fx, int param, double* minvalOut, double* maxvalOut));
  MOCK_CONST_METHOD2(TrackFX_GetEnabled,
      bool(MediaTrack* track, int fx));
  MOCK_CONST_METHOD2(TrackFX_GetOffline,
      bool(MediaTrack* track, int fx));
  MOCK_CONST_METHOD3(TrackFX_EndParamEdit,
      bool(MediaTrack * track, int fx, int param));
  MOCK_CONST_METHOD3(TrackFX_SetEnabled,
      void(MediaTrack * track, int fx, bool enabled));
  MOCK_CONST_METHOD0(GetLastTouchedTrack,
      MediaTrack*());
  MOCK_CONST_METHOD2(CreateTrackSend,
      int(MediaTrack* tr, MediaTrack* desttrInOptional));
  MOCK_CONST_METHOD2(GetTrackNumSends,
      int(MediaTrack* tr, int category));
  MOCK_CONST_METHOD3(RemoveTrackSend,
      bool(MediaTrack* tr, int category, int sendidx));
  MOCK_CONST_METHOD5(SetTrackSendInfo_Value,
      bool(MediaTrack* tr, int category, int sendidx, const char* parmname, double newvalue));
  MOCK_CONST_METHOD4(GetFXEnvelope,
      TrackEnvelope*(MediaTrack* track, int fxindex, int parameterindex, bool create));
  MOCK_CONST_METHOD3(TrackFX_GetByName,
      int(MediaTrack* track, const char* fxname, bool instantiate));
  MOCK_CONST_METHOD7(InsertEnvelopePoint,
      bool(TrackEnvelope* envelope, double time, double value, int shape, double tension, bool selected, bool* noSortInOptional));
  MOCK_CONST_METHOD3(DeleteEnvelopePointRange,
      bool(TrackEnvelope* envelope, double time_start, double time_end));
  MOCK_CONST_METHOD1(Envelope_SortPoints,
      bool(TrackEnvelope* envelope));
  MOCK_CONST_METHOD3(ShowMessageBox, int(const char* msg, const char* title, int type));
  MOCK_CONST_METHOD1(GetMasterTrack,
      MediaTrack*(ReaProject* proj));
  MOCK_CONST_METHOD0(GetMasterTrackVisibility,
      int());
  MOCK_CONST_METHOD1(SetMasterTrackVisibility,
      int(int flag));
  MOCK_CONST_METHOD2(GetTrackEnvelopeByName,
      TrackEnvelope*(MediaTrack* track, const char* envname));
  MOCK_CONST_METHOD1(SetOnlyTrackSelected,
      void(MediaTrack* track));
  MOCK_CONST_METHOD2(ReorderSelectedTracks,
      bool(int beforeTrackIdx, int makePrevFolder));
  MOCK_CONST_METHOD4(GetSetTrackGroupMembership,
      unsigned int(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue));
  MOCK_CONST_METHOD4(GetSetTrackGroupMembershipHigh,
      unsigned int(MediaTrack* tr, const char* groupname, unsigned int setmask, unsigned int setvalue));
  MOCK_CONST_METHOD2(ValidatePtr,
      bool(void* pointer, const char* ctypename));
  MOCK_CONST_METHOD2(SetTrackColor,
      void(MediaTrack* track, int color));
  MOCK_CONST_METHOD3(ColorToNative,
      int(int r, int g, int b));
  MOCK_CONST_METHOD1(GetTrackGUID,
                     GUID*(MediaTrack*));
  MOCK_CONST_METHOD2(TrackFX_GetFXGUID,
                     GUID*(MediaTrack*, int));
  MOCK_CONST_METHOD3(EnumProjects, ReaProject* (int index, char* projectFileName, int projectFilenameSize));
  MOCK_CONST_METHOD1(Undo_BeginBlock2, void(ReaProject* project));
  MOCK_CONST_METHOD3(Undo_EndBlock2, void(ReaProject* project, char const* changeDescription, int extraFlags));
  MOCK_CONST_METHOD3(TrackFX_SetPreset,
      bool(MediaTrack* track, int fx, const char* presetname));
  MOCK_CONST_METHOD3(TrackFX_GetPresetIndex,
      int(MediaTrack* track, int fx, int* numberOfPresetsOut));
  MOCK_CONST_METHOD4(TrackFX_GetPreset,
      bool(MediaTrack* track, int fx, char* presetname, int presetname_sz));
  MOCK_CONST_METHOD1(CountEnvelopePoints, int(TrackEnvelope* envelope));
  MOCK_CONST_METHOD7(GetEnvelopePoint, bool(TrackEnvelope* envelope, int ptidx, double* timeOutOptional, double* valueOutOptional, int* shapeOutOptional, double* tensionOutOptional, bool* selectedOutOptional));
  MOCK_CONST_METHOD3(GetTrackUIVolPan, bool(MediaTrack* track, double* volumeOut, double* panOut));
  MOCK_CONST_METHOD8(Envelope_Evaluate, int(TrackEnvelope* envelope, double time, double samplerate, int samplesRequested, double* valueOutOptional, double* dVdSOutOptional, double* ddVdSOutOptional, double* dddVdSOutOptional));
  MOCK_CONST_METHOD4(GetEnvelopeStateChunk, bool(TrackEnvelope* env, char* strNeedBig, int strNeedBig_sz, bool isundoOptional));
  MOCK_CONST_METHOD3(SetEnvelopeStateChunk, bool(TrackEnvelope* env, const char* str, bool isundoOptional));
  MOCK_CONST_METHOD5(GetSetTrackSendInfo, void*(MediaTrack* tr, int category, int sendidx, const char* parmname, void* setNewValue));
  MOCK_CONST_METHOD6(TrackFX_SetPinMappings, bool(MediaTrack* tr, int fx, int isoutput, int pin, int low32bits, int hi32bits));
  MOCK_CONST_METHOD1(GetEnvelopeScalingMode, int(TrackEnvelope* env));
  MOCK_CONST_METHOD2(ScaleFromEnvelopeMode, double(int scaling_mode, double val ));
  MOCK_CONST_METHOD2(ScaleToEnvelopeMode, double(int scaling_mode, double val ));
  MOCK_CONST_METHOD1(IsProjectDirty, int(ReaProject* proj));
  MOCK_CONST_METHOD1(Main_openProject, void(const char* name));
  MOCK_CONST_METHOD1(GetProjectLength, double(ReaProject* proj));
  MOCK_CONST_METHOD1(GetTrackNumMediaItems, int(MediaTrack* tr));
  MOCK_CONST_METHOD2(GetTrackMediaItem, MediaItem*(MediaTrack* tr, int itemidx));
  MOCK_CONST_METHOD4(GetSetProjectInfo, double(ReaProject* project, const char* desc, double value, bool is_set));
  MOCK_CONST_METHOD0(GetAppVersion, const char*());
  MOCK_CONST_METHOD3(LocalizeString, const char*(const char* src_string, const char* section, int flagsOptional));
  MOCK_CONST_METHOD4(GetTrackStateChunk, bool(MediaTrack* track, char* strNeedBig, int strNeedBig_sz, bool isundoOptional));
  MOCK_CONST_METHOD3(SetTrackStateChunk, bool(MediaTrack* track, const char* str, bool isundoOptional));

  // Custom funcs
  MOCK_CONST_METHOD0(UpdateArrangeForAutomation,
    void());
  MOCK_CONST_METHOD7(RouteTrackToTrack,
      int(MediaTrack *srcTrack, int srcChOffset, MediaTrack *dstTrack, int dstChOffset, int busWidth, I_SENDMODE sendMode, bool evenIfAlreadyRouted));
  MOCK_CONST_METHOD2(setTrackChannelCount,
      void(MediaTrack* track, int channelCount));
  MOCK_CONST_METHOD1(disableTrackMasterSend,
      void(MediaTrack* track));
  MOCK_CONST_METHOD2(setTrackName,
      void(MediaTrack* track, std::string name));
  MOCK_CONST_METHOD2(setItemName,
      void(MediaItem* item, std::string name));
  MOCK_CONST_METHOD2(setTakeName,
      void(MediaItem_Take* take, std::string name));
  MOCK_CONST_METHOD3(setTrackSendBusWidth,
      void(MediaTrack* track, int sendIndex, int busWidth));
  MOCK_CONST_METHOD2(addPluginToTrack,
      int(MediaTrack* track, const char* pluginName));
  MOCK_CONST_METHOD2(createTrackAtIndex,
      MediaTrack*(int index, bool fromEnd));
  MOCK_CONST_METHOD3(getPluginEnvelope,
      TrackEnvelope*(MediaTrack* track, GUID* pluginGuid, int parameterIndex));
  MOCK_CONST_METHOD1(activateAndShowTrackVolumeEnvelope,
      void(MediaTrack* track));
  MOCK_CONST_METHOD1(getTrackIndexOfSelectedMediaItem,
      int(int selIndex));
  MOCK_CONST_METHOD3(moveTrackToBeforeIndex,
      void(MediaTrack* trk, int index, TrackMoveMode moveMode));
  MOCK_CONST_METHOD2(setAsVCAGroupMaster, void(MediaTrack* trk, int groupId));
  MOCK_CONST_METHOD2(setAsVCAGroupSlave, void(MediaTrack* trk, int groupId));
  MOCK_CONST_METHOD1(firstTrackWithPluginNamed, std::unique_ptr<Track>(std::string));
  MOCK_CONST_METHOD0(createTrack, std::unique_ptr<Track>());
  MOCK_CONST_METHOD1(createTrack, std::unique_ptr<Track>(MediaTrack*));
  MOCK_CONST_METHOD0(masterTrack, std::unique_ptr<Track>());
  MOCK_CONST_METHOD0(getCurrentProject, ReaProject*());
  MOCK_CONST_METHOD2(resetFxPinMap, void(MediaTrack* trk, int fxNum));
  MOCK_CONST_METHOD4(mapFxPin, void(MediaTrack* trk, int fxNum, int trackChannel, int fxChannel));
  MOCK_CONST_METHOD1(forceAmplitudeScaling, bool(TrackEnvelope * trackEnvelope));
  MOCK_CONST_METHOD2(getTrackAudioBounds, std::optional<std::pair<double, double>>(MediaTrack* tr, bool ignoreBeforeZero));
  MOCK_CONST_METHOD3(TrackFX_GetActualFXName, bool(MediaTrack* track, int fx, std::string& name));
  MOCK_CONST_METHOD1(TrackFX_GetActualFXNames, std::vector<std::string>(MediaTrack* track));
  MOCK_CONST_METHOD1(CleanFXName, void(std::string& name));
  MOCK_CONST_METHOD2(TrackFX_PositionByActualName, int(MediaTrack* track, const std::string& fxName));
  MOCK_CONST_METHOD4(TrackFX_AddByActualName, int(MediaTrack* track, const char* fxname, bool recFX, int instantiate));


};

}  // namespace admplug
