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

#include "rdkStorageMgr.h"
#include <stdio.h>

void rdkStorage_init (void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

eSTMGRReturns rdkStorage_getDeviceIds(eSTMGRDeviceIDs* pDeviceIDs)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getDeviceInfo(char* pDeviceID, eSTMGRDeviceInfo* pDeviceInfo)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getDeviceInfoList(eSTMGRDeviceInfoList* pDeviceInfoList)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getPartitionInfo (char* pDeviceID, char* pPartitionId, eSTMGRPartitionInfo* pPartitionInfo)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBStatus (eSTMGRTSBStatus *pTSBStatus)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_setTSBMaxMinutes (unsigned int minutes)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBMaxMinutes (unsigned int *pMinutes)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBCapacityMinutes(unsigned int *pMinutes)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBCapacity(unsigned long long *pCapacity)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBFreeSpace(unsigned long long *pFreeSpace)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getDVRCapacity(unsigned long long *pCapacity)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getDVRFreeSpace(unsigned long long *pFreeSpace)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

bool rdkStorage_isTSBEnabled()
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return false;
}

eSTMGRReturns rdkStorage_setTSBEnabled (bool isEnabled)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

bool rdkStorage_isDVREnabled()
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return false;
}

bool rdkStorage_isSDCard()
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return false;
}

eSTMGRReturns rdkStorage_setDVREnabled (bool isEnabled)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getHealth (char* pDeviceID, eSTMGRHealthInfo* pHealthInfo)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_RegisterEventCallback(fnSTMGR_EventCallback eventCallback)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

eSTMGRReturns rdkStorage_getTSBPartitionMountPath (char* pMountPath)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return RDK_STMGR_RETURN_GENERIC_FAILURE;
}

void rdkStorage_notifyMGRAboutFailure (eSTMGRErrorEvent failEvent)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}
