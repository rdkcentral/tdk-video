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

#include <stdio.h>
#include <dsAudio.h>
#include <dsCompositeIn.h>
#include <dsDisplay.h>
#include <dsFPD.h>
#include <dsHdmiIn.h>
#include <dsHost.h>
#include <dsVideoDevice.h>
#include <dsVideoPort.h>
/* Device Settings HAL DUMMY APIs */

/* dsAudio.h @file */
dsError_t  dsAudioPortInit()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioPort(dsAudioPortType_t type, int index, intptr_t *handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioEncoding(intptr_t handle, dsAudioEncoding_t *encoding)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioFormat(intptr_t handle, dsAudioFormat_t *audioFormat)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioCompression(intptr_t handle, int *compression)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetDialogEnhancement(intptr_t handle, int *level)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetDolbyVolumeMode(intptr_t handle, bool *mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetIntelligentEqualizerMode(intptr_t handle, int *mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetVolumeLeveller(intptr_t handle, dsVolumeLeveller_t* volLeveller)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetBassEnhancer(intptr_t handle, int *boost)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsSurroundDecoderEnabled(intptr_t handle, bool *enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetDRCMode(intptr_t handle, int *mode)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetSurroundVirtualizer(intptr_t handle, dsSurroundVirtualizer_t *virtualizer)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetMISteering(intptr_t handle, bool *enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetGraphicEqualizerMode(intptr_t handle, int *mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetMS12AudioProfileList(intptr_t handle, dsMS12AudioProfileList_t* profiles)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetMS12AudioProfile(intptr_t handle, char *profile)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetSupportedARCTypes(intptr_t handle, int *types)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioSetSAD(intptr_t handle, dsAudioSADList_t sad_list)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioEnableARC(intptr_t handle, dsAudioARCStatus_t arcStatus)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetStereoMode(intptr_t handle, dsAudioStereoMode_t *stereoMode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetStereoAuto(intptr_t handle, int *autoMode)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioGain(intptr_t handle, float *gain)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioDB(intptr_t handle, float *db)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioLevel(intptr_t handle, float *level)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioMaxDB(intptr_t handle, float *maxDb)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioMinDB(intptr_t handle, float *minDb)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAudioOptimalLevel(intptr_t handle, float *optimalLevel)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetAudioDelay(intptr_t handle, uint32_t *audioDelayMs)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetAudioDelayOffset(intptr_t handle, uint32_t *audioDelayOffsetMs)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetAudioAtmosOutputMode(intptr_t handle, bool enable)

{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetSinkDeviceAtmosCapability(intptr_t handle, dsATMOSCapability_t *capability)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsAudioLoopThru(intptr_t handle, bool *loopThru)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsAudioMute(intptr_t handle, bool *muted)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsAudioPortEnabled(intptr_t handle, bool *enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableAudioPort(intptr_t handle, bool enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableMS12Config(intptr_t handle, dsMS12FEATURE_t feature,const bool enable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableLEConfig(intptr_t handle, const bool enable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetLEConfig(intptr_t handle, bool *enable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioEncoding(intptr_t handle, dsAudioEncoding_t encoding)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioCompression(intptr_t handle, int compression)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetDialogEnhancement(intptr_t handle, int level)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetDolbyVolumeMode(intptr_t handle, bool mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetIntelligentEqualizerMode(intptr_t handle, int mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetVolumeLeveller(intptr_t handle, dsVolumeLeveller_t volLeveller)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetBassEnhancer(intptr_t handle, int boost)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableSurroundDecoder(intptr_t handle, bool enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetDRCMode(intptr_t handle, int mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetSurroundVirtualizer(intptr_t handle, dsSurroundVirtualizer_t virtualizer)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetMISteering(intptr_t handle, bool enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetGraphicEqualizerMode(intptr_t handle, int mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetMS12AudioProfile(intptr_t handle, const char* profile)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetStereoMode(intptr_t handle, dsAudioStereoMode_t mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetStereoAuto(intptr_t handle, int autoMode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioGain(intptr_t handle, float gain)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioDB(intptr_t handle, float db)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioLevel(intptr_t handle, float level)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioDucking(intptr_t handle, dsAudioDuckingAction_t action, dsAudioDuckingType_t, const unsigned char level)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableLoopThru(intptr_t handle, bool loopThru)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetAudioMute(intptr_t handle, bool mute)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsAudioMSDecode(intptr_t handle, bool *HasMS11Decode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsAudioMS12Decode(intptr_t handle, bool *HasMS12Decode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetAudioDelay(intptr_t handle, const uint32_t audioDelayMs)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetAudioDelayOffset(intptr_t handle, const uint32_t audioDelayOffsetMs)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsAudioPortTerm()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioOutIsConnected(intptr_t handle, bool* pisCon)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioOutRegisterConnectCB(dsAudioOutPortConnectCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioFormatUpdateRegisterCB (dsAudioFormatUpdateCB_t cbFun)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsAudioAtmosCapsChangeRegisterCB (dsAtmosCapsChangeCB_t cbFun)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetAudioCapabilities(intptr_t handle, int *capabilities)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetMS12Capabilities(intptr_t handle, int *capabilities)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsResetDialogEnhancement(intptr_t handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsResetBassEnhancer(intptr_t handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsResetSurroundVirtualizer(intptr_t handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsResetVolumeLeveller(intptr_t handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetMS12AudioProfileSetttingsOverride(intptr_t handle,const char* profileState,const char* profileName,const char* profileSettingsName,const char* profileSettingValue)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetAssociatedAudioMixing(intptr_t handle, bool mixing)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetAssociatedAudioMixing(intptr_t handle, bool *mixing)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetFaderControl(intptr_t handle, int mixerbalance)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetFaderControl(intptr_t handle, int* mixerbalance)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetPrimaryLanguage(intptr_t handle, const char* pLang)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetPrimaryLanguage(intptr_t handle, char* pLang)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetSecondaryLanguage(intptr_t handle, const char* sLang)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetSecondaryLanguage(intptr_t handle, char* sLang)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDMIARCPortId(int *portId)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsCompositeIn.h */
dsError_t dsCompositeInInit (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsCompositeInTerm (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsCompositeInGetNumberOfInputs (uint8_t *pNumberOfInputs)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsCompositeInGetStatus (dsCompositeInStatus_t *pStatus)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsCompositeInSelectPort (dsCompositeInPort_t Port)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsCompositeInScaleVideo (int32_t x, int32_t y, int32_t width, int32_t height)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsDisplay.h */

dsError_t dsDisplayInit()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}

 dsError_t dsGetDisplay(dsVideoPortType_t vType, int index, intptr_t *handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetEDID(intptr_t handle, dsDisplayEDID_t *edid)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetEDIDBytes(intptr_t handle, unsigned char *edid, int *length)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetDisplayAspectRatio(intptr_t handle, dsVideoAspectRatio_t *aspectRatio)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsDisplayTerm()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsFPD.h */
dsError_t dsFPInit (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPBlink (dsFPDIndicator_t eIndicator, unsigned int uBlinkDuration, unsigned int uBlinkIterations)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPBrightness (dsFPDIndicator_t eIndicator, dsFPDBrightness_t eBrightness)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFPState(dsFPDIndicator_t eIndicator, dsFPDState_t* state)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPState(dsFPDIndicator_t eIndicator, dsFPDState_t state)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFPBrightness (dsFPDIndicator_t eIndicator, dsFPDBrightness_t *pBrightness)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFPColor (dsFPDIndicator_t eIndicator, dsFPDColor_t *pColor)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPColor (dsFPDIndicator_t eIndicator, dsFPDColor_t eColor)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPTime (dsFPDTimeFormat_t eTimeFormat, const unsigned int uHour, const unsigned int uMinutes)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPText(const char* pText)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPTextBrightness (dsFPDTextDisplay_t eIndicator, dsFPDBrightness_t eBrightness)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFPTextBrightness (dsFPDTextDisplay_t eIndicator, dsFPDBrightness_t *eBrightness)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsFPEnableCLockDisplay (int enable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPScroll(unsigned int uScrollHoldOnDur, unsigned int uHorzScrollIterations, unsigned int uVertScrollIterations)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsFPTerm(void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPDBrightness(dsFPDIndicator_t eIndicator, dsFPDBrightness_t eBrightness,bool toPersist)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPDColor (dsFPDIndicator_t eIndicator, dsFPDColor_t eColor,bool toPersist)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPTimeFormat (dsFPDTimeFormat_t eTimeFormat)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFPTimeFormat (dsFPDTimeFormat_t *pTimeFormat)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFPDMode (dsFPDMode_t eMode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsFPGetSupportedLEDStates (unsigned int* states)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsFPGetLEDState (dsFPDLedState_t* state)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsFPSetLEDState (dsFPDLedState_t state)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsHdmiIn.h */
dsError_t dsHdmiInInit (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInTerm (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInGetNumberOfInputs (uint8_t *pNumberOfInputs)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInGetStatus (dsHdmiInStatus_t *pStatus)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInSelectPort (dsHdmiInPort_t Port)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInScaleVideo (int32_t x, int32_t y, int32_t width, int32_t height)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInSelectZoomMode (dsVideoZoom_t requestedZoomMode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInPauseAudio (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInResumeAudio (void)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInGetCurrentVideoMode (dsVideoPortResolution_t *resolution)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInRegisterConnectCB (dsHdmiInConnectCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInRegisterSignalChangeCB (dsHdmiInSignalChangeCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInRegisterStatusChangeCB (dsHdmiInStatusChangeCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInRegisterVideoModeUpdateCB(dsHdmiInVideoModeUpdateCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHdmiInRegisterAllmChangeCB (dsHdmiInAllmChangeCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetEDIDBytesInfo (int iHdmiPort, unsigned char **edid, int *length)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDMISPDInfo (int iHdmiPort, unsigned char **data)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetEdidVersion (int iHdmiPort, int iEdidVersion)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetEdidVersion (int iHdmiPort, int *iEdidVersion)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetEdid2AllmSupport (int iHdmiPort, bool allmSupport)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetEdid2AllmSupport (int iHdmiPort, bool *allmSupport)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetAllmStatus (int iHdmiPort, bool *allmStatus)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetSupportedGameFeaturesList (dsSupportedGameFeatureList_t* features)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetAVLatency(int *audio_latency, int *video_latency)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsHost.h */ 
dsError_t dsHostInit()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetHostPowerMode(int newPower)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHostPowerMode(int *currPower)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsHostTerm()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetPreferredSleepMode(dsSleepMode_t *pMode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetPreferredSleepMode(dsSleepMode_t mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetCPUTemperature(float *cpuTemperature)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetVersion(uint32_t *versionNumber)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetVersion(uint32_t versionNumber)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetSocIDFromSDK(char *socID)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHostEDID(unsigned char *edid, int *length)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetMS12ConfigType(const char *configType)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsVideoDevice.h */
dsError_t  dsVideoDeviceInit()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetVideoDevice(int index, intptr_t *handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetDFC(intptr_t handle, dsVideoZoom_t dfc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetDFC(intptr_t handle, dsVideoZoom_t *dfc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsVideoDeviceTerm()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDRCapabilities(intptr_t handle, int *capabilities)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetSupportedVideoCodingFormats(intptr_t handle, unsigned int * supported_formats)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetVideoCodecInfo(intptr_t handle, dsVideoCodingFormat_t codec, dsVideoCodecInfo_t * info)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsForceDisableHDRSupport(intptr_t handle, bool disable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetFRFMode(intptr_t handle, int frfmode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetFRFMode(intptr_t handle, int *frfmode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetCurrentDisplayframerate(intptr_t handle, char *framerate)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetDisplayframerate(intptr_t handle, char *framerate)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsRegisterFrameratePreChangeCB(dsRegisterFrameratePreChangeCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsRegisterFrameratePostChangeCB(dsRegisterFrameratePostChangeCB_t CBFunc)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
/* dsVideoPort.h */
dsError_t  dsVideoPortInit()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetVideoPort(dsVideoPortType_t type, int index, intptr_t *handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsVideoPortEnabled(intptr_t handle, bool *enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsDisplayConnected(intptr_t handle, bool *connected)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsDisplaySurround(intptr_t handle, bool *surround)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetSurroundMode(intptr_t handle, int *surround)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsVideoPortActive(intptr_t handle, bool *active)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableDTCP(intptr_t handle, bool contentProtect)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableHDCP(intptr_t handle, bool contentProtect, char *hdcpKey, size_t keySize)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsDTCPEnabled (intptr_t handle, bool* pContentProtected)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsIsHDCPEnabled (intptr_t handle, bool* pContentProtected)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsEnableVideoPort(intptr_t handle, bool enabled)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsSetResolution(intptr_t handle, dsVideoPortResolution_t *resolution)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsGetResolution(intptr_t handle, dsVideoPortResolution_t *resolution)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetActiveSource(intptr_t handle)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsVideoPortTerm()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t  dsInitResolution(dsVideoPortResolution_t *)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsRegisterHdcpStatusCallback (intptr_t handle, dsHDCPStatusCallback_t cb)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDCPStatus (intptr_t handle, dsHdcpStatus_t *status)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDCPProtocol (intptr_t handle,dsHdcpProtocolVersion_t *protocolVersion)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDCPReceiverProtocol (intptr_t handle,dsHdcpProtocolVersion_t *protocolVersion)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHDCPCurrentProtocol (intptr_t handle,dsHdcpProtocolVersion_t *protocolVersion)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetTVHDRCapabilities(intptr_t handle, int *capabilities)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSupportedTvResolutions(intptr_t handle, int *resolutions)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetForceDisable4KSupport(intptr_t handle, bool disable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetForceDisable4KSupport(intptr_t handle, bool *disable)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetScartParameter(intptr_t handle, const char* parameter_str, const char* value_str)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetVideoEOTF(intptr_t handle, dsHDRStandard_t *video_eotf)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetMatrixCoefficients(intptr_t handle, dsDisplayMatrixCoefficients_t *matrix_coefficients)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetColorDepth(intptr_t handle, unsigned int* color_depth)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetColorSpace(intptr_t handle, dsDisplayColorSpace_t* color_space)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetQuantizationRange(intptr_t handle, dsDisplayQuantizationRange_t* quantization_range)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetCurrentOutputSettings(intptr_t handle, dsHDRStandard_t* video_eotf, dsDisplayMatrixCoefficients_t* matrix_coefficients, dsDisplayColorSpace_t* color_space, unsigned int* color_depth, dsDisplayQuantizationRange_t* quantization_range)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsIsOutputHDR(intptr_t handle, bool *hdr)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsResetOutputToSDR()
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetHdmiPreference(intptr_t handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetHdmiPreference(intptr_t handle, dsHdcpProtocolVersion_t *hdcpCurrentProtocol)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetIgnoreEDIDStatus(intptr_t handle, bool* status)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetBackgroundColor(intptr_t handle, dsVideoBackgroundColor_t color)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetForceHDRMode(intptr_t handle, dsHDRStandard_t mode)
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsColorDepthCapabilities(intptr_t handle, unsigned int *colorDepthCapability )
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsGetPreferredColorDepth(intptr_t handle, dsDisplayColorDepth_t *colorDepth, bool persist )
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}
dsError_t dsSetPreferredColorDepth(intptr_t handle,dsDisplayColorDepth_t colorDepth, bool persist )
{
        printf("\nDUMMY %s\n", __FUNCTION__);
        return dsERR_GENERAL;

}



