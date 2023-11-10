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
#include "libIBus.h"
#include "libIARM.h"

IARM_Result_t IARM_Bus_Init(const char *name)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_Connect(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_event_registration(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_Disconnect(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}

IARM_Result_t IARM_Bus_Term(void)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return IARM_RESULT_INVALID_PARAM;
}
