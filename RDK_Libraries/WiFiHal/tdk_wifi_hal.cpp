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
#include "wifi_common_hal.h"
#include "wifi_client_hal.h"

INT wifi_init()
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_down()
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_uninit()
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT  wifi_getRadioSupportedFrequencyBands(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioIfName(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT  wifi_getRadioOperatingFrequencyBand(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioSupportedStandards(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioPossibleChannels(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioChannelsInUse(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioOperatingChannelBandwidth(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRegulatoryDomain(INT radioIndex, CHAR* output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getSSIDName(INT apIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getBaseBSSID(INT ssidIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getSSIDMACAddress(INT ssidIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioStatus(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioExtChannel(INT radioIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getHalVersion(CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getCliWpsConfigMethodsSupported(INT ssidIndex, CHAR *methods)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getCliWpsConfigMethodsEnabled(INT ssidIndex, CHAR *output_string)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_setCliWpsConfigMethodsEnabled(INT ssidIndex, CHAR *methodString)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioChannel(INT radioIndex,ULONG *output_ulong)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioNumberOfEntries(ULONG *output)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getSSIDNumberOfEntries(ULONG *output)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioEnable(INT radioIndex, BOOL *output_bool)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioStandard(INT radioIndex, CHAR *output_string, BOOL *gOnly, BOOL *nOnly, BOOL *acOnly)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getRadioTrafficStats(INT radioIndex, wifi_radioTrafficStats_t *output_struct)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getSSIDTrafficStats(INT ssidIndex, wifi_ssidTrafficStats_t *output_struct)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getNeighboringWiFiDiagnosticResult(INT radioIndex, wifi_neighbor_ap_t **neighbor_ap_array, UINT *output_array_size)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_connectEndpoint(INT ssidIndex, CHAR *AP_SSID, wifiSecurityMode_t AP_security_mode, CHAR *AP_security_WEPKey, CHAR *AP_security_PreSharedKey, CHAR *AP_security_KeyPassphrase,int saveSSID,CHAR * eapIdentity,CHAR * carootcert,CHAR * clientcert,CHAR * privatekey)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

void wifi_getStats(INT radioIndex, wifi_sta_stats_t *wifi_sta_stats)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
}

INT wifi_getDualBandSupport()
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_getSpecificSSIDInfo(const char* SSID, WIFI_HAL_FREQ_BAND band, wifi_neighbor_ap_t **filtered_ap_array, UINT *output_array_size)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_setRadioScanningFreqList(INT radioIndex, const CHAR *freqList)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_clearSSIDInfo(INT ssidIndex)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_lastConnected_Endpoint(wifi_pairedSSIDInfo_t *pairedSSIDInfo)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_disconnectEndpoint(INT ssidIndex, CHAR *AP_SSID)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}

INT wifi_setCliWpsButtonPush(INT ssidIndex)
{
    printf("\n DUMMY  %s\n", __FUNCTION__);
    return 1;
}


