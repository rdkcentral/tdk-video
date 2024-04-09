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
#include "plat_power.h"

/* Power Manager HAL DUMMY APIs */
pmStatus_t PLAT_INIT()
{
	printf( "DUMMY PLAT_INIT\n");
	return PWRMGR_INIT_FAILURE;
}

pmStatus_t PLAT_API_SetPowerState(PWRMgr_PowerState_t newState)
{
	printf( "DUMMY PLAT_API_SetPowerState\n");
        return PWRMGR_INVALID_ARGUMENT;
}

pmStatus_t PLAT_INIT(pmStatus_t cec_enable)
{
        printf( "DUMMY PLAT_INIT\n");
        return PWRMGR_INVALID_ARGUMENT;
}

pmStatus_t PLAT_API_GetPowerState(PWRMgr_PowerState_t *curState)
{
	printf( "DUMMY PLAT_API_GetPowerState\n");
        return PWRMGR_INVALID_ARGUMENT;
}

int PLAT_API_GetTemperature(PWRMgr_ThermalState_t *curState, float *curTemperature, float *wifiTemperature)
{
	printf( "DUMMY PLAT_API_GetTemperature\n");
        return 1;
}

pmStatus_t PLAT_TERM()
{
	printf( "DUMMY PLAT_TERM\n");
	return PWRMGR_TERM_FAILURE;
}
