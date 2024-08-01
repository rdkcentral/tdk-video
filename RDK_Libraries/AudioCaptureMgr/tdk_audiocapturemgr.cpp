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
#include <iostream>
#include "rmfAudioCapture.h"

rmf_Error RMF_AudioCapture_Open (RMF_AudioCaptureHandle* handle)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_Open_Type (RMF_AudioCaptureHandle* handle, RMF_AudioCaptureType rmfAcType)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_GetStatus (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Status* status)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_GetDefaultSettings (RMF_AudioCapture_Settings* settings)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_GetCurrentSettings (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Settings* settings)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_Start (RMF_AudioCaptureHandle handle, RMF_AudioCapture_Settings* settings)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_Stop (RMF_AudioCaptureHandle handle)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}

rmf_Error RMF_AudioCapture_Close (RMF_AudioCaptureHandle handle)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return RMF_ERROR;
}
