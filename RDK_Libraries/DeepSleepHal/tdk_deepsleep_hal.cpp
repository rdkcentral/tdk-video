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
#include <cstdint>
#include "deepSleepMgr.h"

int PLAT_DS_INIT(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int PLAT_DS_SetDeepSleep(uint32_t deep_sleep_timeout, bool *isGPIOWakeup)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int PLAT_DS_SetDeepSleep(uint32_t deep_sleep_timeout)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

void PLAT_DS_DeepSleepWakeup(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

void PLAT_DS_TERM(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}


