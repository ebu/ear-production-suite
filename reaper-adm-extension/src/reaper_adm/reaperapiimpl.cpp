#include <cassert>
#include <stdexcept>
#include <bitset>
#include <string>
#include <sstream>

#include "reaperapiimpl.h"
#include "reaperapivalues.h"
#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"
#include "track.h"
#include "plugin.h"

#define TWO_TO_THE_POWER_OF(power) (1<<power)

extern void (*InsertTrackAtIndex)(int index, bool wantDefaults);

using namespace admplug;

admplug::ReaperAPIImpl::~ReaperAPIImpl() = default;

admplug::ReaperAPIImpl::ReaperAPIImpl(reaper_plugin_info_t& plugin_info) : plugin_info{plugin_info}
{

}

const char* ReaperAPIImpl::GetResourcePath() const {
    return ::GetResourcePath();
}

const char* ReaperAPIImpl::GetExePath() const {
    return ::GetExePath();
}

void ReaperAPIImpl::InsertTrackAtIndex(int index, bool wantDefaults) const {
    ::InsertTrackAtIndex(index, wantDefaults);
}

void ReaperAPIImpl::GetSetMediaTrackInfo_String(MediaTrack *track, const char *parameterName, char *newValue, bool setNewValue) const {
    ::GetSetMediaTrackInfo_String(track, parameterName, newValue, setNewValue);
}

void admplug::ReaperAPIImpl::GetSetMediaItemInfo_String(MediaItem * item, const char * parameterName, char * newValue, bool setNewValue) const {
    ::GetSetMediaItemInfo_String(item, parameterName, newValue, setNewValue);
}

void admplug::ReaperAPIImpl::GetSetMediaItemTakeInfo_String(MediaItem_Take * take, const char * parameterName, char * newValue, bool setNewValue) const {
    ::GetSetMediaItemTakeInfo_String(take, parameterName, newValue, setNewValue);
}

double ReaperAPIImpl::GetMediaTrackInfo_Value(MediaTrack* tr, const char* parmname) const {
    return ::GetMediaTrackInfo_Value(tr, parmname);
}

bool ReaperAPIImpl::GetTrackName(MediaTrack* track, char* buf, int buf_sz) const {
    return ::GetTrackName(track, buf, buf_sz);
}

bool ReaperAPIImpl::SetMediaTrackInfo_Value(MediaTrack *track, const char *parameterName, double newValue) const {
    return ::SetMediaTrackInfo_Value(track, parameterName, newValue);
}

MediaTrack* ReaperAPIImpl::GetTrack(ReaProject *project, int trackIndex) const {
    return ::GetTrack(project, trackIndex);
}

MediaTrack* ReaperAPIImpl::GetMediaItem_Track(MediaItem* item) const {
    return ::GetMediaItem_Track(item);
}

bool ReaperAPIImpl::GetUserFileNameForRead(char *filenameNeed4096, const char *title, const char *defext) const {
   return ::GetUserFileNameForRead(filenameNeed4096, title, defext);
}

int ReaperAPIImpl::Register(std::string name, void* infostruct) const {
    return plugin_info.Register(name.c_str(), infostruct);
}

int ReaperAPIImpl::CountSelectedTracks(ReaProject* proj) const {
    return ::CountSelectedTracks(proj);
}

MediaTrack* ReaperAPIImpl::GetSelectedTrack(ReaProject* proj, int seltrackidx) const {
    return ::GetSelectedTrack(proj, seltrackidx);
}

int ReaperAPIImpl::CountSelectedMediaItems(ReaProject *proj) const {
    return ::CountSelectedMediaItems(proj);
}

int ReaperAPIImpl::CountTracks(ReaProject *proj) const
{
    return ::CountTracks(proj);
}

MediaItem_Take *ReaperAPIImpl::GetActiveTake(MediaItem *item) const {
   return ::GetActiveTake(item);
}

MediaItem *ReaperAPIImpl::GetSelectedMediaItem(ReaProject *proj, int selitem) const {
    return ::GetSelectedMediaItem(proj, selitem);
}

PCM_source *ReaperAPIImpl::GetMediaItemTake_Source(MediaItem_Take *take) const {
    return ::GetMediaItemTake_Source(take);
}

PCM_source *ReaperAPIImpl::GetMediaSourceParent(PCM_source *src) const {
    return ::GetMediaSourceParent(src);
}

void ReaperAPIImpl::GetMediaSourceFileName(PCM_source *source, char *filenamebuf, int filenamebuf_sz) const {
    return ::GetMediaSourceFileName(source, filenamebuf, filenamebuf_sz);
}

void ReaperAPIImpl::GetProjectPath(char *buf, int buf_sz) const
{
    ::GetProjectPath(buf, buf_sz);
}

void ReaperAPIImpl::ShowConsoleMsg(const char *msg) const {
    return ::ShowConsoleMsg(msg);
}

MediaItem *ReaperAPIImpl::AddMediaItemToTrack(MediaTrack *tr) const
{
    return ::AddMediaItemToTrack(tr);
}

MediaItem_Take *ReaperAPIImpl::AddTakeToMediaItem(MediaItem *item) const
{
    return ::AddTakeToMediaItem(item);
}

bool ReaperAPIImpl::SetMediaItemTake_Source(MediaItem_Take *take, PCM_source *source) const
{
    return ::SetMediaItemTake_Source(take, source);
}

bool ReaperAPIImpl::SetMediaItemTakeInfo_Value(MediaItem_Take *take, const char *parmname, double newvalue) const
{
    return ::SetMediaItemTakeInfo_Value(take, parmname, newvalue);
}

bool ReaperAPIImpl::SetMediaItemInfo_Value(MediaItem* item, const char* parmname, double newvalue) const
{
  return ::SetMediaItemInfo_Value(item, parmname, newvalue);
}

double ReaperAPIImpl::GetMediaItemInfo_Value(MediaItem* item, const char* parmname) const
{
  return ::GetMediaItemInfo_Value(item, parmname);
}

bool ReaperAPIImpl::SetMediaItemLength(MediaItem *item, double length, bool refreshUI) const
{
    return ::SetMediaItemLength(item, length, refreshUI);
}

bool ReaperAPIImpl::SetMediaItemPosition(MediaItem *item, double position, bool refreshUI) const
{
  return ::SetMediaItemLength(item, position, refreshUI);
}

void ReaperAPIImpl::UpdateArrange() const
{
    ::UpdateArrange();
}

void ReaperAPIImpl::UpdateTimeline() const
{
  ::UpdateTimeline();
}

void ReaperAPIImpl::Main_OnCommandEx(int command, int flag, ReaProject *proj) const
{
    ::Main_OnCommandEx(command, flag, proj);
}

bool ReaperAPIImpl::AddExtensionsMainMenu() const
{
    return ::AddExtensionsMainMenu();
}

void ReaperAPIImpl::SelectAllMediaItems(ReaProject *proj, bool selected) const
{
    ::SelectAllMediaItems(proj, selected);
}

void ReaperAPIImpl::SetMediaItemSelected(MediaItem *item, bool selected) const
{
    ::SetMediaItemSelected(item, selected);
}

double ReaperAPIImpl::GetMediaSourceLength(PCM_source *source, bool *lengthIsQNOut) const
{
   return ::GetMediaSourceLength(source, lengthIsQNOut);
}

int ReaperAPIImpl::GetMediaSourceNumChannels(PCM_source* source) const
{
    return ::GetMediaSourceNumChannels(source);
}

PCM_source *ReaperAPIImpl::PCM_Source_CreateFromFile(const char *filename) const
{
    return ::PCM_Source_CreateFromFile(filename);
}

int ReaperAPIImpl::TrackFX_AddByName(MediaTrack *track, const char *fxname, bool recFX, int instantiate) const
{
    return ::TrackFX_AddByName(track, fxname, recFX, instantiate);
}

bool admplug::ReaperAPIImpl::TrackFX_Delete(MediaTrack * track, int fx) const
{
    return ::TrackFX_Delete(track, fx);
}

int ReaperAPIImpl::TrackFX_GetCount(MediaTrack* track) const
{
    return ::TrackFX_GetCount(track);
}

bool ReaperAPIImpl::TrackFX_GetFXName(MediaTrack *track, int fx, char *buf, int buf_sz) const
{
    return ::TrackFX_GetFXName(track, fx, buf, buf_sz);
}

MediaTrack *ReaperAPIImpl::GetLastTouchedTrack() const
{
    return ::GetLastTouchedTrack();
}

int ReaperAPIImpl::CreateTrackSend(MediaTrack *tr, MediaTrack *desttrInOptional) const
{
    return ::CreateTrackSend(tr, desttrInOptional);
}

int admplug::ReaperAPIImpl::GetTrackNumSends(MediaTrack * tr, int category) const
{
    return ::GetTrackNumSends(tr, category);
}

bool admplug::ReaperAPIImpl::RemoveTrackSend(MediaTrack * tr, int category, int sendidx) const
{
    return ::RemoveTrackSend(tr, category, sendidx);
}

bool ReaperAPIImpl::SetTrackSendInfo_Value(MediaTrack *tr, int category, int sendidx, const char *parmname, double newvalue) const
{
    return ::SetTrackSendInfo_Value(tr, category, sendidx, parmname, newvalue);
}

TrackEnvelope *ReaperAPIImpl::GetFXEnvelope(MediaTrack *track, int fxindex, int parameterindex, bool create) const
{
    return ::GetFXEnvelope(track, fxindex, parameterindex, create);
}

int ReaperAPIImpl::TrackFX_GetByName(MediaTrack *track, const char *fxname, bool instantiate) const
{
    return ::TrackFX_GetByName(track, fxname, instantiate);
}

bool ReaperAPIImpl::TrackFX_EndParamEdit(MediaTrack* track, int fx, int param) const
{
    return ::TrackFX_EndParamEdit(track, fx, param);
}

void ReaperAPIImpl::TrackFX_SetEnabled(MediaTrack* track, int fx, bool enabled) const
{
    ::TrackFX_SetEnabled(track, fx, enabled);
}

bool ReaperAPIImpl::TrackFX_SetParam(MediaTrack* track, int fx, int param, double val) const {
    return ::TrackFX_SetParam(track, fx, param, val);
}

bool ReaperAPIImpl::TrackFX_SetParamNormalized(MediaTrack* track, int fx, int param, double val) const {
    return ::TrackFX_SetParam(track, fx, param, val);
}

double ReaperAPIImpl::TrackFX_GetParamNormalized(MediaTrack* track, int fx, int param) const {
    return ::TrackFX_GetParamNormalized(track, fx, param);
}

double ReaperAPIImpl::TrackFX_GetParam(MediaTrack* track, int fx, int param, double* minvalOut, double* maxvalOut) const {
    return  ::TrackFX_GetParam(track, fx, param, minvalOut, maxvalOut);
}

bool ReaperAPIImpl::TrackFX_GetEnabled(MediaTrack* track, int fx) const {
    return ::TrackFX_GetEnabled(track, fx);
}
bool ReaperAPIImpl::TrackFX_GetOffline(MediaTrack* track, int fx) const {
    return ::TrackFX_GetOffline(track, fx);
}

bool ReaperAPIImpl::InsertEnvelopePoint(TrackEnvelope *envelope, double time, double value, int shape, double tension, bool selected, bool *noSortInOptional) const
{
    return ::InsertEnvelopePoint(envelope, time, value, shape, tension, selected, noSortInOptional);
}

bool ReaperAPIImpl::DeleteEnvelopePointRange(TrackEnvelope* envelope, double time_start, double time_end) const
{
    return ::DeleteEnvelopePointRange(envelope, time_start, time_end);
}

bool ReaperAPIImpl::Envelope_SortPoints(TrackEnvelope *envelope) const
{
    return ::Envelope_SortPoints(envelope);
}

int ReaperAPIImpl::ShowMessageBox(const char* msg, const char* title, int type) const
{
  return ::ShowMessageBox(msg, title, type);
}

MediaTrack* ReaperAPIImpl::GetMasterTrack(ReaProject* proj) const
{
    return ::GetMasterTrack(proj);
}

int ReaperAPIImpl::GetMasterTrackVisibility() const
{
  return ::GetMasterTrackVisibility();
}

int ReaperAPIImpl::SetMasterTrackVisibility(int flag) const
{
  return ::SetMasterTrackVisibility(flag);
}

TrackEnvelope *ReaperAPIImpl::GetTrackEnvelopeByName(MediaTrack *track, const char *envname) const
{
    return ::GetTrackEnvelopeByName(track, envname);
}

void ReaperAPIImpl::SetOnlyTrackSelected(MediaTrack *track) const
{
    return ::SetOnlyTrackSelected(track);
}

bool ReaperAPIImpl::ReorderSelectedTracks(int beforeTrackIdx, int makePrevFolder) const
{
    return ::ReorderSelectedTracks(beforeTrackIdx, makePrevFolder);
}

unsigned int ReaperAPIImpl::GetSetTrackGroupMembership(MediaTrack *tr, const char *groupname, unsigned int setmask, unsigned int setvalue) const
{
    return ::GetSetTrackGroupMembership(tr, groupname, setmask, setvalue);
}

unsigned int ReaperAPIImpl::GetSetTrackGroupMembershipHigh(MediaTrack *tr, const char *groupname, unsigned int setmask, unsigned int setvalue) const
{
    return ::GetSetTrackGroupMembershipHigh(tr, groupname, setmask, setvalue);
}

bool ReaperAPIImpl::ValidatePtr(void *pointer, const char *ctypename) const
{
    return ::ValidatePtr(pointer, ctypename);
}

void ReaperAPIImpl::SetTrackColor(MediaTrack *track, int color) const
{
    return ::SetTrackColor(track, color);
}

int ReaperAPIImpl::ColorToNative(int r, int g, int b) const
{
    return ::ColorToNative(r,g,b);
}

GUID *ReaperAPIImpl::GetTrackGUID(MediaTrack *track) const
{
    return ::GetTrackGUID(track);
}

GUID * admplug::ReaperAPIImpl::TrackFX_GetFXGUID(MediaTrack * track, int fxNum) const
{
    return ::TrackFX_GetFXGUID(track, fxNum);
}

ReaProject *ReaperAPIImpl::EnumProjects(int index, char *projectFileName, int projectFilenameSize) const
{
    return ::EnumProjects(index, projectFileName, projectFilenameSize);
}

void ReaperAPIImpl::Undo_BeginBlock2(ReaProject* project) const
{
    ::Undo_BeginBlock2(project);
}

void ReaperAPIImpl::Undo_EndBlock2(ReaProject *project, const char *changeDescription, int extraFlags) const
{
    ::Undo_EndBlock2(project, changeDescription, extraFlags);
}
bool ReaperAPIImpl::TrackFX_SetPreset(MediaTrack *track, int fx, const char *presetname) const
{
    return ::TrackFX_SetPreset(track, fx, presetname);
}

int admplug::ReaperAPIImpl::TrackFX_GetPresetIndex(MediaTrack * track, int fx, int * numberOfPresetsOut) const
{
    return ::TrackFX_GetPresetIndex(track, fx, numberOfPresetsOut);
}

bool admplug::ReaperAPIImpl::TrackFX_GetPreset(MediaTrack * track, int fx, char * presetname, int presetname_sz) const
{
    return ::TrackFX_GetPreset(track, fx, presetname, presetname_sz);
}

int admplug::ReaperAPIImpl::CountEnvelopePoints(TrackEnvelope * envelope) const
{
    return ::CountEnvelopePoints(envelope);
}

bool admplug::ReaperAPIImpl::GetEnvelopePoint(TrackEnvelope * envelope, int ptidx, double * timeOutOptional, double * valueOutOptional, int * shapeOutOptional, double * tensionOutOptional, bool * selectedOutOptional) const
{
    return ::GetEnvelopePoint(envelope, ptidx, timeOutOptional, valueOutOptional, shapeOutOptional, tensionOutOptional, selectedOutOptional);
}

bool admplug::ReaperAPIImpl::GetTrackUIVolPan(MediaTrack * track, double * volumeOut, double * panOut) const
{
    return ::GetTrackUIVolPan(track, volumeOut, panOut);
}

int admplug::ReaperAPIImpl::Envelope_Evaluate(TrackEnvelope * envelope, double time, double samplerate, int samplesRequested, double * valueOutOptional, double * dVdSOutOptional, double * ddVdSOutOptional, double * dddVdSOutOptional) const
{
    return ::Envelope_Evaluate(envelope, time, samplerate, samplesRequested, valueOutOptional, dVdSOutOptional, ddVdSOutOptional, dddVdSOutOptional);
}

bool admplug::ReaperAPIImpl::GetEnvelopeStateChunk(TrackEnvelope * env, char * strNeedBig, int strNeedBig_sz, bool isundoOptional) const
{
    return ::GetEnvelopeStateChunk(env, strNeedBig, strNeedBig_sz, isundoOptional);
}

bool admplug::ReaperAPIImpl::SetEnvelopeStateChunk(TrackEnvelope * env, const char * str, bool isundoOptional) const
{
    return ::SetEnvelopeStateChunk(env, str, isundoOptional);
}

void* admplug::ReaperAPIImpl::GetSetTrackSendInfo(MediaTrack* tr, int category, int sendidx, const char* parmname, void* setNewValue) const
{
    return ::GetSetTrackSendInfo(tr, category, sendidx, parmname, setNewValue);
}

bool admplug::ReaperAPIImpl::TrackFX_SetPinMappings(MediaTrack* tr, int fx, int isoutput, int pin, int low32bits, int hi32bits) const
{
    return ::TrackFX_SetPinMappings(tr, fx, isoutput, pin, low32bits, hi32bits);
}
int admplug::ReaperAPIImpl::GetEnvelopeScalingMode(TrackEnvelope * env) const
{
    return ::GetEnvelopeScalingMode(env);
}

double admplug::ReaperAPIImpl::ScaleFromEnvelopeMode(int scaling_mode, double val) const
{
    return ::ScaleFromEnvelopeMode(scaling_mode, val);
}

double admplug::ReaperAPIImpl::ScaleToEnvelopeMode(int scaling_mode, double val) const
{
    return ::ScaleToEnvelopeMode(scaling_mode, val);
}

int admplug::ReaperAPIImpl::IsProjectDirty(ReaProject * proj) const
{
    return ::IsProjectDirty(proj);
}

void admplug::ReaperAPIImpl::Main_openProject(const char * name) const
{
    ::Main_openProject(name);
}

double admplug::ReaperAPIImpl::GetProjectLength(ReaProject * proj) const
{
    return ::GetProjectLength(proj);
}

int admplug::ReaperAPIImpl::GetTrackNumMediaItems(MediaTrack * tr) const
{
    return ::GetTrackNumMediaItems(tr);
}

MediaItem * admplug::ReaperAPIImpl::GetTrackMediaItem(MediaTrack * tr, int itemidx) const
{
    return ::GetTrackMediaItem(tr, itemidx);
}

double admplug::ReaperAPIImpl::GetSetProjectInfo(ReaProject* project, const char* desc, double value, bool is_set) const
{
    return ::GetSetProjectInfo(project, desc, value, is_set);
}

const char* admplug::ReaperAPIImpl::GetAppVersion() const
{
    return ::GetAppVersion();
}

const char* admplug::ReaperAPIImpl::LocalizeString(const char* src_string, const char* section, int flagsOptional) const
{
    return ::LocalizeString(src_string, section, flagsOptional);
}

void ReaperAPIImpl::UpdateArrangeForAutomation() const {
  // UpdateArrange() does not force draw of automation. Flipping master track visibility does.
  std::bitset<4> mstTrkSetting(GetMasterTrackVisibility());
  mstTrkSetting.flip(0);
  SetMasterTrackVisibility((int)mstTrkSetting.to_ulong());
  mstTrkSetting.flip(0);
  SetMasterTrackVisibility((int)mstTrkSetting.to_ulong());
  UpdateArrange();
}

int ReaperAPIImpl::RouteTrackToTrack(MediaTrack *srcTrack, int srcChOffset, MediaTrack *dstTrack, int dstChOffset, int busWidth, I_SENDMODE sendMode, bool evenIfAlreadyRouted) const {
    auto iSrcChan = getSrcChannelValue(busWidth, srcChOffset);
    auto iDstChan = getDstChannelValue(busWidth, dstChOffset);
    if(!evenIfAlreadyRouted) {
        int numSends = GetTrackNumSends(srcTrack, TrackRouteTarget::onSend);
        for(int sendIndex = 0; sendIndex < numSends; sendIndex++) {
            MediaTrack* target = (MediaTrack*)GetSetTrackSendInfo(srcTrack, TrackRouteTarget::onSend, sendIndex, "P_DESTTRACK", nullptr); // nullptr = get
            if(target == dstTrack) {
                // track is the same, check channel
                auto src = *(int*)GetSetTrackSendInfo(srcTrack, TrackRouteTarget::onSend, sendIndex, "I_SRCCHAN", nullptr);
                auto dst = *(int*)GetSetTrackSendInfo(srcTrack, TrackRouteTarget::onSend, sendIndex, "I_DSTCHAN", nullptr);
                if(iSrcChan == src && iDstChan == dst) {
                    return sendIndex;
                }
            }
        }
    }

    int sendIndex = CreateTrackSend(srcTrack, dstTrack);
    if (sendIndex >= 0) {
        if(!SetTrackSendInfo_Value(srcTrack, TrackRouteTarget::onSend, sendIndex, "I_SENDMODE", sendMode)) sendIndex = -1;
        if(!SetTrackSendInfo_Value(srcTrack, TrackRouteTarget::onSend, sendIndex, "I_SRCCHAN", getSrcChannelValue(busWidth, srcChOffset))) sendIndex = -1;
        if(!SetTrackSendInfo_Value(srcTrack, TrackRouteTarget::onSend, sendIndex, "I_DSTCHAN", getDstChannelValue(busWidth, dstChOffset))) sendIndex = -1;
    }
    return sendIndex;
}

int ReaperAPIImpl::toReaperChannelValue(int busWidth, int startChNum) const {
    // Generates a Channel value for a range of channels as interpreted by Reaper
    // We use 0 - based indexes for startChNum
    if (busWidth == 0) return -1; // Reaper knows this as None
    return reaperChannelOffsetForBusWidth(busWidth) + startChNum;
}

int ReaperAPIImpl::reaperChannelOffsetForBusWidth(int busWidth) const {
    switch(busWidth){
    case 1:     return 1024;
    case 2:     return 0;
    default:    {
        if(busWidth > 64) {
            throw std::runtime_error("ReaperAPI::reaperChannelOffsetForBusWidth() bus width too large");
        }
        if(busWidth % 2) {
            busWidth += 1;
        }
        return (busWidth / 2) * 1024;
    }
    }
}

int ReaperAPIImpl::getSrcChannelValue(int busWidth, int startCh) const {
    //Simply compliments getDstChannelValue
    return toReaperChannelValue(busWidth, startCh);
}

int ReaperAPIImpl::getDstChannelValue(int busWidth, int startCh) const {
    /* We have to be careful because if busWidth is > 2, then I_DSTCHAN is just the starting channel number(think this may be a reaper bug)

    I.e, Mapping:

             chValue      |      srcChan             dstChan
        BusWidth, startCh | BusWidth, startCh   BusWidth, startCh
        ------------------|------------------------------------------
        y = 1      x      |    y         0         y         x
        y = 2      x      |    y         0         y         x
        y > 2      x      |    y         0         2         x

    */
    if (busWidth <= 2) return toReaperChannelValue(busWidth, startCh);
    return startCh;
}

void ReaperAPIImpl::setTrackChannelCount(MediaTrack* track, int channelCount) const {
    SetMediaTrackInfo_Value(track, "I_NCHAN", channelCount);
}

void ReaperAPIImpl::disableTrackMasterSend(MediaTrack* track) const {
    SetMediaTrackInfo_Value(track, "B_MAINSEND", false);
}

void ReaperAPIImpl::setTrackName(MediaTrack* track, std::string name) const {
    std::vector<char> nameBuffer{ name.begin(), name.end() };
    nameBuffer.push_back('\0');
    GetSetMediaTrackInfo_String(track, "P_NAME", &name[0], true);
}

void admplug::ReaperAPIImpl::setItemName(MediaItem * item, std::string name) const {
    std::vector<char> nameBuffer{ name.begin(), name.end() };
    nameBuffer.push_back('\0');
    GetSetMediaItemInfo_String(item, "P_NAME", &name[0], true);
}

void admplug::ReaperAPIImpl::setTakeName(MediaItem_Take * take, std::string name) const {
    std::vector<char> nameBuffer{ name.begin(), name.end() };
    nameBuffer.push_back('\0');
    GetSetMediaItemTakeInfo_String(take, "P_NAME", &name[0], true);
}

void ReaperAPIImpl::setTrackSendBusWidth(MediaTrack* track, int sendIndex, int busWidth) const {
    SetTrackSendInfo_Value(track, 0, sendIndex, "I_SRCCHAN", toReaperChannelValue(busWidth, 0));
}

int ReaperAPIImpl::addPluginToTrack(MediaTrack* track, const char* pluginName) const {
    return TrackFX_AddByName(track, pluginName, false, 1);
}

MediaTrack* ReaperAPIImpl::createTrackAtIndex(int index, bool fromEnd) const {
    int trackCount = CountTracks(nullptr);
    if (index > trackCount) index = trackCount;
    if (fromEnd) index = trackCount - index;
    InsertTrackAtIndex(index, false);
    return GetTrack(nullptr, index);
}


TrackEnvelope* ReaperAPIImpl::getPluginEnvelope(MediaTrack* track, const char* pluginName, int parameterIndex) const {
    auto spatialiserIndex = TrackFX_GetByName(track, pluginName, false);
    return GetFXEnvelope(track, spatialiserIndex, parameterIndex, true);
}

void ReaperAPIImpl::activateAndShowTrackVolumeEnvelope(MediaTrack* track) const {
    SetOnlyTrackSelected(track);
    Main_OnCommandEx(ReaperAPI::TOGGLE_VOLUME_ENVELOPE_VISIBLE, 0, nullptr);
}

int ReaperAPIImpl::getTrackIndexOfSelectedMediaItem(int selIndex) const {
    int sels = CountSelectedMediaItems(nullptr);
    if (sels > selIndex) {
        MediaTrack* selTrk = GetMediaItem_Track(GetSelectedMediaItem(nullptr, selIndex));
        return (int)GetMediaTrackInfo_Value(selTrk, "IP_TRACKNUMBER");
    }
    return -1;
}

void ReaperAPIImpl::moveTrackToBeforeIndex(MediaTrack* trk, int index, TrackMoveMode moveMode) const {
    // Get selected tracks
    int selCount = CountSelectedTracks(nullptr);
    std::vector<MediaTrack*> selTracks;
    for (int i = 0; i < selCount; i++) selTracks.push_back(GetSelectedTrack(nullptr, i));

    // Select trk
    SetOnlyTrackSelected(trk);
    // Reorder selected tracks
    ReorderSelectedTracks(index, moveMode);
    // Unselect trk
    SetTrackSelected(trk, false);

    // Reinstate original selection
    for (MediaTrack* selTrack : selTracks) SetTrackSelected(selTrack, true);
}

void ReaperAPIImpl::setAsVCAGroupMaster(MediaTrack *trk, int groupId) const
{
    std::bitset<32> bits;
    if(groupId >= 0 && groupId < 32) {
        bits.flip(static_cast<std::size_t>(groupId));
        auto groupMask = static_cast<unsigned int>(bits.to_ulong());
        GetSetTrackGroupMembership(trk, "VOLUME_VCA_MASTER", groupMask, 0xFFFF);
        GetSetTrackGroupMembership(trk, "MUTE_MASTER", groupMask, 0xFFFF);
        GetSetTrackGroupMembership(trk, "SOLO_MASTER", groupMask, 0xFFFF);
    } else if(groupId >= 32 && groupId < 64) {
        groupId -= 32;
        bits.flip(static_cast<std::size_t>(groupId));
        auto groupMask = static_cast<unsigned int>(bits.to_ulong());
        GetSetTrackGroupMembershipHigh(trk, "VOLUME_VCA_MASTER", groupMask, 0xFFFF);
        GetSetTrackGroupMembershipHigh(trk, "MUTE_MASTER", groupMask, 0xFFFF);
        GetSetTrackGroupMembershipHigh(trk, "SOLO_MASTER", groupMask, 0xFFFF);
    }
}

void ReaperAPIImpl::setAsVCAGroupSlave(MediaTrack *trk, int groupId) const
{
    std::bitset<32> bits;
    if(groupId >= 0 && groupId < 32) {
        bits.flip(static_cast<std::size_t>(groupId++));
        auto groupMask = static_cast<unsigned int>(bits.to_ulong());
        GetSetTrackGroupMembership(trk, "VOLUME_VCA_SLAVE", groupMask, 0xFFFF);
        GetSetTrackGroupMembership(trk, "MUTE_SLAVE", groupMask, 0xFFFF);
        GetSetTrackGroupMembership(trk, "SOLO_SLAVE", groupMask, 0xFFFF);
    } else if(groupId >= 32 && groupId < 64) {
        groupId -= 32;
        bits.flip(static_cast<std::size_t>(groupId++));
        auto groupMask = static_cast<unsigned int>(bits.to_ulong());
        GetSetTrackGroupMembershipHigh(trk, "VOLUME_VCA_SLAVE", groupMask, 0xFFFF);
        GetSetTrackGroupMembershipHigh(trk, "MUTE_SLAVE", groupMask, 0xFFFF);
        GetSetTrackGroupMembershipHigh(trk, "SOLO_SLAVE", groupMask, 0xFFFF);
    }

}

std::unique_ptr<Track> ReaperAPIImpl::firstTrackWithPluginNamed(std::string pluginName) const
{
    int trackCount = CountTracks(nullptr);

    for (int i = 0; i < trackCount; i++) {
        auto track = std::make_unique<TrackInstance>(GetTrack(nullptr, i), *this);
        auto plugin = track->getPlugin(pluginName);
        if (plugin) {
            return std::move(track);
        }
    }

    return nullptr;

}

std::unique_ptr<Track> ReaperAPIImpl::createTrack() const
{
    return std::make_unique<TrackInstance>(createTrackAtIndex(0), *this);
}

std::unique_ptr<Track> ReaperAPIImpl::createTrack(MediaTrack* track) const
{
    return std::make_unique<TrackInstance>(track, *this);
}

std::unique_ptr<Track> ReaperAPIImpl::masterTrack() const
{
    return createTrack(GetMasterTrack(nullptr));
}

ReaProject* ReaperAPIImpl::getCurrentProject() const
{
    return EnumProjects(-1, NULL, 0);
}

void admplug::ReaperAPIImpl::resetFxPinMap(MediaTrack * trk, int fxNum) const
{
    for(int pinNum = 0; pinNum < 64; pinNum++) {
        mapFxPin(trk, fxNum, pinNum, pinNum);
    }
}

void admplug::ReaperAPIImpl::mapFxPin(MediaTrack * trk, int fxNum, int trackChannel, int fxChannel) const
{
    if(trackChannel < 32) {
        uint32_t low = TWO_TO_THE_POWER_OF(trackChannel);
        TrackFX_SetPinMappings(trk, fxNum, 0, fxChannel, low, 0);
        TrackFX_SetPinMappings(trk, fxNum, 1, fxChannel, low, 0);
    } else if(trackChannel < 64) {
        trackChannel -= 32;
        uint32_t high = TWO_TO_THE_POWER_OF(trackChannel);
        TrackFX_SetPinMappings(trk, fxNum, 0, fxChannel, 0, high);
        TrackFX_SetPinMappings(trk, fxNum, 1, fxChannel, 0, high);
    }

}

bool admplug::ReaperAPIImpl::forceAmplitudeScaling(TrackEnvelope * trackEnvelope) const
{
    char chunk[1024]; // A new envelope should only be about 100 bytes
    bool getRes = GetEnvelopeStateChunk(trackEnvelope, chunk, 1024, false);
    if(!getRes) return false;

    std::istringstream chunkSs(chunk);
    std::string line;
    bool chunkComplete = false; // Set when we find the end of the chunk so we know we read it all.

    bool paramInserted = false; // Set once we've inserted the parameter... we expect it before points and before the chunk end if no points
    std::string opChunk;

    while(std::getline(chunkSs, line)) {
        auto ptPos = line.rfind("PT ", 0);
        auto vtPos = line.rfind("VOLTYPE ", 0);
        auto closePos = line.rfind(">", 0);

        if(ptPos == 0 || closePos == 0) {
            if(!paramInserted) {
                opChunk.append("VOLTYPE 0\n");
                paramInserted = true;
            }
        }

        if(vtPos != 0) {
            opChunk.append(line);
            opChunk.append("\n");
        }

        if(closePos == 0) {
            chunkComplete = true;
            break;
        }
    }

    if(!paramInserted || !chunkComplete) return false;
    return SetEnvelopeStateChunk(trackEnvelope, opChunk.c_str(), false);
}

std::optional<std::pair<double, double>> admplug::ReaperAPIImpl::getTrackAudioBounds(MediaTrack * trk, bool ignoreBeforeZero) const
{
    std::optional<double> start;
    std::optional<double> end;

    auto count = CountTrackMediaItems(trk);
    for(int i = 0; i < count; i++) {
        auto mediaItem = GetTrackMediaItem(trk, i);

        double thisStart = GetMediaItemInfo_Value(mediaItem, "D_POSITION");
        double thisDuration = GetMediaItemInfo_Value(mediaItem, "D_LENGTH");
        double thisEnd = thisStart + thisDuration;

        if(!start.has_value() || thisStart < *start) start = thisStart;
        if(!end.has_value() || thisEnd > *end) end = thisEnd;
    }

    if(start.has_value() && end.has_value()) {
        if(ignoreBeforeZero) {
            // If end < 0, entire audio out of bounds
            if(*end < 0.0) return std::optional<std::pair<double, double>>();
            if(*start < 0.0) start = 0.0;
        }
        auto pair = std::make_pair(*start, *end);
        return std::optional<std::pair<double, double>>(pair);
    }
    return std::optional<std::pair<double, double>>();
}