/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "main_aamp.h"
#include "AampConfig.h"
#include "AampCacheHandler.h"
#include "AampUtils.h"
#include "AampCCManager.h"

PlayerInstanceAAMP::~PlayerInstanceAAMP()
{
	printf("\nDUMMY Destructor\n");
}

void PlayerInstanceAAMP::Stop(bool sendStateChangeEvent)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::Tune(const char *mainManifestUrl, const char *contentType, bool bFirstAttempt,
                                bool bFinalAttempt,const char *traceUUID,bool audioDecoderStreamSync)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::detach()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::RegisterEvents(EventListener* eventListener)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::UnRegisterEvents(EventListener* eventListener)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLanguageFormat(LangCodePreference preferredFormat, bool useRole)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetMinimumBitrate(BitsPerSecond bitrate)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

BitsPerSecond PlayerInstanceAAMP::GetMinimumBitrate(void)
{
        BitsPerSecond bitrate = 0;
	printf("\nDUMMY %s\n", __FUNCTION__);
	return bitrate;
}

void PlayerInstanceAAMP::SetMaximumBitrate(BitsPerSecond bitrate)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

BitsPerSecond PlayerInstanceAAMP::GetMaximumBitrate(void)
{
	BitsPerSecond bitrate = 0;
        printf("\nDUMMY %s\n", __FUNCTION__);
        return bitrate;
}

bool PlayerInstanceAAMP::IsValidRate(int rate)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}

void PlayerInstanceAAMP::SetRate(float rate,int overshootcorrection)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::PauseAt(double position)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::Seek(double secondsRelativeToTuneTime, bool keepPaused)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SeekToLive(bool keepPaused)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetSlowMotionPlayRate( float rate )
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetRateAndSeek(int rate, double secondsRelativeToTuneTime)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetVideoRectangle(int x, int y, int w, int h)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetVideoZoom(VideoZoomMode zoom)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetVideoMute(bool muted)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetSubtitleMute(bool muted)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAudioVolume(int volume)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLanguage(const char* language)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

bool PlayerInstanceAAMP::IsLive()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}

const char* PlayerInstanceAAMP::GetCurrentAudioLanguage(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	static char lang[1];
	lang[0] = 0;
	return lang;
}

std::string PlayerInstanceAAMP::GetDRM()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "NONE";
}

void PlayerInstanceAAMP::SetLicenseServerURL(const char *url, DRMSystems type)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLiveOffset(double liveoffset)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLiveOffset4K(double liveoffset)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetStallErrorCode(int errorCode)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetStallTimeout(int timeoutMS)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetReportInterval(int reportInterval)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetInitFragTimeoutRetryCount(int count)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

double PlayerInstanceAAMP::GetPlaybackPosition()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 0;
}

double PlayerInstanceAAMP::GetPlaybackDuration()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 0;
}

int PlayerInstanceAAMP::GetId(void)
{
        int iPlayerId = -1;
        printf("\nDUMMY %s\n", __FUNCTION__);
        return iPlayerId;
}

PrivAAMPState PlayerInstanceAAMP::GetState(void)
{
	PrivAAMPState currentState = eSTATE_RELEASED;
	printf("\nDUMMY %s\n", __FUNCTION__);
	return currentState;
}

long PlayerInstanceAAMP::GetVideoBitrate(void)
{
	BitsPerSecond bitrate = 0;
	printf("\nDUMMY %s\n", __FUNCTION__);
	return bitrate;
}

void PlayerInstanceAAMP::SetVideoBitrate(BitsPerSecond bitrate)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

BitsPerSecond PlayerInstanceAAMP::GetAudioBitrate(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	BitsPerSecond bitrate = 0;
	return bitrate;
}

int PlayerInstanceAAMP::GetVideoZoom(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

bool PlayerInstanceAAMP::GetVideoMute(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

int PlayerInstanceAAMP::GetAudioVolume(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

int PlayerInstanceAAMP::GetPlaybackRate(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

std::vector<BitsPerSecond> PlayerInstanceAAMP::GetVideoBitrates(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	std::vector<BitsPerSecond> bitrates;
	return bitrates;
}

std::string PlayerInstanceAAMP::GetManifest(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

std::vector<BitsPerSecond> PlayerInstanceAAMP::GetAudioBitrates(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	std::vector<BitsPerSecond> bitrates;
	return bitrates;
}

void PlayerInstanceAAMP::SetInitialBitrate(BitsPerSecond bitrate)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

BitsPerSecond PlayerInstanceAAMP::GetInitialBitrate(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	BitsPerSecond bitrate = 0;
        return bitrate;
}

void PlayerInstanceAAMP::SetInitialBitrate4K(BitsPerSecond bitrate4K)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
}

BitsPerSecond PlayerInstanceAAMP::GetInitialBitrate4k(void)
{
	BitsPerSecond bitrate = 0;
	printf("\nDUMMY %s\n", __FUNCTION__);
        return bitrate;
}

void PlayerInstanceAAMP::SetNetworkTimeout(double timeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetManifestTimeout(double timeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPlaylistTimeout(double timeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetDownloadBufferSize(int bufferSize)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPreferredDRM(DRMSystems drmType)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetStereoOnlyPlayback(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetDisable4K(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetBulkTimedMetaReport(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetRetuneForUnpairedDiscontinuity(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAlternateContents(const std::string &adBreakId, const std::string &adId, const std::string &url)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetNetworkProxy(const char * proxy)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLicenseReqProxy(const char * licenseProxy)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetDownloadStallTimeout(int stallTimeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetDownloadStartTimeout(int startTimeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetDownloadLowBWTimeout(int lowBWTimeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPreferredSubtitleLanguage(const char* language)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetParallelPlaylistDL(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetParallelPlaylistRefresh(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetWesterosSinkConfig(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLicenseCaching(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetOutputResolutionCheck(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetMatchingBaseUrlConfig(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetNewABRConfig(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPropagateUriParameters(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::ApplyArtificialDownloadDelay(unsigned int DownloadDelayInMs)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetSslVerifyPeerConfig(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAudioTrack(std::string language, std::string rendition, std::string type, std::string codec, unsigned int channel, std::string label)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}
						
void PlayerInstanceAAMP::SetPreferredCodec(const char *codecList)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPreferredLabels(const char *labelList)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPreferredRenditions(const char *renditionList)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

std::string PlayerInstanceAAMP::GetPreferredAudioProperties()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

std::string PlayerInstanceAAMP::GetPreferredTextProperties()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

void PlayerInstanceAAMP::SetPreferredTextLanguages(const char *param)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

DRMSystems PlayerInstanceAAMP::GetPreferredDRM()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return eDRM_NONE;
}

std::string PlayerInstanceAAMP::GetPreferredLanguages()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return NULL;
}

std::string PlayerInstanceAAMP::GetAvailableVideoTracks()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

void PlayerInstanceAAMP::SetVideoTracks(std::vector<BitsPerSecond> bitrates)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

std::string PlayerInstanceAAMP::GetAvailableAudioTracks(bool allTrack)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

std::string PlayerInstanceAAMP::GetAudioTrackInfo()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

std::string PlayerInstanceAAMP::GetTextTrackInfo()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

std::string PlayerInstanceAAMP::GetAvailableTextTracks(bool allTrack)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return "";
}

std::string PlayerInstanceAAMP::GetVideoRectangle()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return "";
}

void PlayerInstanceAAMP::SetAppName(std::string name)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetNativeCCRendering(bool enable)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetTuneEventConfig(int tuneEventType)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::EnableVideoRectangle(bool rectProperty)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAudioTrack(int trackId)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

int PlayerInstanceAAMP::GetAudioTrack()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

void PlayerInstanceAAMP::SetTextTrack(int trackId, char *ccData)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

int PlayerInstanceAAMP::GetTextTrack()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 1;
}

void PlayerInstanceAAMP::SetCCStatus(bool enabled)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

bool PlayerInstanceAAMP::GetCCStatus(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}

void PlayerInstanceAAMP::SetTextStyle(const std::string &options)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

std::string PlayerInstanceAAMP::GetTextStyle()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

void PlayerInstanceAAMP::SetInitRampdownLimit(int limit)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetCEAFormat(int format)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

std::string PlayerInstanceAAMP::GetAvailableThumbnailTracks(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

bool PlayerInstanceAAMP::SetThumbnailTrack(int thumbIndex)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}

std::string PlayerInstanceAAMP::GetThumbnails(double tStart, double tEnd)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}

void PlayerInstanceAAMP::SetSessionToken(std::string sessionToken)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::EnableSeekableRange(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetReportVideoPTS(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::DisableContentRestrictions(long grace, long time, bool eventChange)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::EnableContentRestrictions()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::ManageAsyncTuneConfig(const char* mainManifestUrl)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAsyncTuneConfig(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::AsyncStartStop()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::PersistBitRateOverSeek(bool bValue)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetPausedBehavior(int behavior)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetUseAbsoluteTimeline(bool configState)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetRepairIframes(bool configState)
{
	printf("\nDUMMY %s\n", __FUNCTION__);

}

void PlayerInstanceAAMP::XRESupportedTune(bool xreSupported)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetAuxiliaryLanguage(const std::string &language)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetLicenseCustomData(const char *customData)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

std::string PlayerInstanceAAMP::GetPlaybackStats()
{
	std::string stats;
	printf("\nDUMMY %s\n", __FUNCTION__);
	return stats;
}

void PlayerInstanceAAMP::ProcessContentProtectionDataConfig(const char *jsonbuffer)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetContentProtectionDataUpdateTimeout(int timeout)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

void PlayerInstanceAAMP::SetRuntimeDRMConfigSupport(bool DynamicDRMSupported)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}

bool PlayerInstanceAAMP::IsOOBCCRenderingSupported()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}

std::string PlayerInstanceAAMP::GetVideoPlaybackQuality(void)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return "";
}
