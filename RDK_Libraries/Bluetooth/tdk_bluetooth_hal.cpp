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

#include "btrCore.h"
#include <stdio.h>

enBTRCoreRet BTRCore_Init (tBTRCoreHandle* phBTRCore)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_DeInit (tBTRCoreHandle hBTRCore)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_RegisterAgent (tBTRCoreHandle hBTRCore, int iBTRCapMode)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_UnregisterAgent (tBTRCoreHandle hBTRCore)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetListOfAdapters (tBTRCoreHandle hBTRCore, stBTRCoreListAdapters* pstListAdapters)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapterPower (tBTRCoreHandle hBTRCore, const char* pAdapterPath, unsigned char powerStatus)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapterPower (tBTRCoreHandle hBTRCore, const char* pAdapterPath, unsigned char* pAdapterPower)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapters (tBTRCoreHandle hBTRCore, stBTRCoreGetAdapters* pstGetAdapters)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapter (tBTRCoreHandle hBTRCore, stBTRCoreAdapter* apstBTRCoreAdapter)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapter (tBTRCoreHandle hBTRCore, int adapter_number)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_EnableAdapter (tBTRCoreHandle hBTRCore, stBTRCoreAdapter* apstBTRCoreAdapter)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_DisableAdapter (tBTRCoreHandle hBTRCore, stBTRCoreAdapter* apstBTRCoreAdapter)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapterAddr (tBTRCoreHandle hBTRCore, unsigned char aui8adapterIdx, char* apui8adapterAddr)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapterDiscoverable (tBTRCoreHandle hBTRCore, const char* pAdapterPath, unsigned char discoverable)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapterDiscoverableTimeout (tBTRCoreHandle hBTRCore, const char* pAdapterPath, unsigned short timeout)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapterDiscoverableStatus (tBTRCoreHandle hBTRCore, const char* pAdapterPath, unsigned char* pDiscoverable)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapterDeviceName (tBTRCoreHandle hBTRCore, stBTRCoreAdapter* apstBTRCoreAdapter, char* apcAdapterDeviceName)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_SetAdapterName (tBTRCoreHandle hBTRCore, const char* pAdapterPath, const char* pAdapterName)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetAdapterName (tBTRCoreHandle hBTRCore, const char* pAdapterPath, char* pAdapterName)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_ResetAdapter(tBTRCoreHandle hBTRCore, stBTRCoreAdapter* apstBTRCoreAdapter)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetVersionInfo(tBTRCoreHandle hBTRCore, char* apcBtVersion)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_StartDiscovery (tBTRCoreHandle hBTRCore, const char* pAdapterPath, enBTRCoreDeviceType aenBTRCoreDevType, unsigned int aui32DiscDuration)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_StopDiscovery (tBTRCoreHandle hBTRCore, const char* pAdapterPath, enBTRCoreDeviceType aenBTRCoreDevType)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetListOfScannedDevices (tBTRCoreHandle hBTRCore, stBTRCoreScannedDevicesCount *pListOfScannedDevices)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_PairDevice (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_UnPairDevice (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetListOfPairedDevices (tBTRCoreHandle hBTRCore, stBTRCorePairedDevicesCount *pListOfDevices)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_FindDevice (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_FindService (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, const char* UUID, char* XMLdata, int* found)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetSupportedServices (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, stBTRCoreSupportedServiceList *pProfileList)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_IsDeviceConnectable (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_ConnectDevice (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, enBTRCoreDeviceType aenBTRCoreDevType)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_DisconnectDevice (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, enBTRCoreDeviceType aenBTRCoreDevType)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetDeviceConnected (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, enBTRCoreDeviceType aenBTRCoreDevType)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetDeviceDisconnected (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, enBTRCoreDeviceType aenBTRCoreDevType)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}

enBTRCoreRet BTRCore_GetDeviceTypeClass (tBTRCoreHandle hBTRCore, tBTRCoreDevId aBTRCoreDevId, enBTRCoreDeviceType* apenBTRCoreDevTy, enBTRCoreDeviceClass* apenBTRCoreDevCl)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return enBTRCoreFailure;
}


