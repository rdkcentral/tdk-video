/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include "DeepSleepHalAgent.h"
std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> stop;

/*************************************************************************************
 *Function name : DeepSleep_Return_Status_toString
 *Description   : This function is to check the ERROR return code of DeepSleep HAL API
 *
 *************************************************************************************/
std::string DeepSleep_Return_Status_toString(DeepSleep_Return_Status_t status) {
    switch (status) {
	    case DEEPSLEEPMGR_INVALID_ARGUMENT: return " Error : DEEPSLEEPMGR_INVALID_ARGUMENT";
	    case DEEPSLEEPMGR_ALREADY_INITIALIZED: return " Error : DEEPSLEEPMGR_ALREADY_INITIALIZED";
	    case DEEPSLEEPMGR_NOT_INITIALIZED: return " Error : DEEPSLEEPMGR_NOT_INITIALIZED";
	    case DEEPSLEEPMGR_INIT_FAILURE: return " Error : DEEPSLEEPMGR_INIT_FAILURE";
	    case DEEPSLEEPMGR_SET_FAILURE: return " Error : DEEPSLEEPMGR_SET_FAILURE";
	    case DEEPSLEEPMGR_WAKEUP_FAILURE: return " Error : DEEPSLEEPMGR_WAKEUP_FAILURE";
	    case DEEPSLEEPMGR_TERM_FAILURE: return " Error :DEEPSLEEPMGR_TERM_FAILURE";
	    case DEEPSLEEPMGR_MAX: return " Error : DEEPSLEEPMGR_MAX";
	    default: return " Error : Unknown DeepSleep_Return_Status_t";
    }
}

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/
std::string DeepSleepHalAgent::testmodulepre_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule pre_requisites --> Entry\n");
#ifdef ENABLE_DEEP_SLEEP
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleep init ....\n");
    DeepSleep_Return_Status_t init = PLAT_DS_INIT();
    if (init == DEEPSLEEPMGR_SUCCESS){
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_INIT call success\n");
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule pre_requisites --> Exit\n");
        return "SUCCESS";
    }
    else{
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_INIT call failed %s\n",DeepSleep_Return_Status_toString(init));
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule pre_requisites --> Exit\n");
        return "FAILURE";
    }
#else
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal Not Supported\n");
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule pre_requisites --> Exit\n");
    return "FAILURE";

#endif //ENABLE_DEEP_SLEEP

}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Description    : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool DeepSleepHalAgent::testmodulepost_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule post_requisites --> Entry\n");
#ifdef ENABLE_DEEP_SLEEP
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleep term ...\n");
    DeepSleep_Return_Status_t term = PLAT_DS_TERM();
    if (term == DEEPSLEEPMGR_SUCCESS){
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_TERM call success\n");
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule post_requisites --> Exit\n");
        return TEST_SUCCESS;
    }
    else{
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_TERM call failed %s\n",DeepSleep_Return_Status_toString(term));
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule post_requisites --> Exit\n");
        return TEST_FAILURE;
    }
#else
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal Not Supported\n");
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal testmodule post_requisites --> Exit\n");
    return TEST_FAILURE;

#endif //ENABLE_DEEP_SLEEP

}

/**************************************************************************
Function Name   : CreateObject
Arguments       : NULL
Description     : This function is used to create a new object of the class "DSHalAgent".
**************************************************************************/
extern "C" DeepSleepHalAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new DeepSleepHalAgent(ptrtcpServer);
}

/***************************************************************************
 *Function name : initialize
 *Description    : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/
bool DeepSleepHalAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "DeepSleepHal Initialization Entry\n");
    DEBUG_PRINT (DEBUG_TRACE, "DeepSleepHal Initialization Exit\n");
    return TEST_SUCCESS;
}

/***************************************************************************
 *Function name  : DeepSleepHal_SetDeepSleep
 *Description    : This function is to invoke PLAT_DS_SetDeepSleep
 *****************************************************************************/
void DeepSleepHalAgent::DeepSleepHal_SetDeepSleep(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_SetDeepSleep --->Entry\n");
    char details[100];
    if(&req["timeout"] == NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "Invalid Timeout Value";
        return;
    }

    bool networkStandby = req["networkStandby"].asBool();
#ifdef ENABLE_DEEP_SLEEP
#ifdef ENABLE_DEEPSLEEP_WAKEUP_EVT
    bool isGPIOWakeup = 0;
#endif //ENABLE_DEEPSLEEP_WAKEUP_EVT
    uint32_t deep_sleep_timeout = (uint32_t)req["timeout"].asInt();
    DEBUG_PRINT(DEBUG_TRACE, "SetDeepSleep for %u seconds\n",(unsigned int)deep_sleep_timeout);

    start = std::chrono::high_resolution_clock::now();
#ifdef ENABLE_DEEPSLEEP_WAKEUP_EVT
    DeepSleep_Return_Status_t ret = PLAT_DS_SetDeepSleep(deep_sleep_timeout, &isGPIOWakeup, networkStandby);
#else
    DeepSleep_Return_Status_t ret = PLAT_DS_SetDeepSleep(deep_sleep_timeout);
#endif //ENABLE_DEEPSLEEP_WAKEUP_EVT
    stop = std::chrono::high_resolution_clock::now();

    int freezeDuration = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();
    if (ret == DEEPSLEEPMGR_SUCCESS){
        DEBUG_PRINT(DEBUG_TRACE, "Resumed from DeepSleep\n");
#ifdef ENABLE_DEEPSLEEP_WAKEUP_EVT
        DEBUG_PRINT(DEBUG_TRACE, "CPU freeze duration : %d, isGPIOWakeup : %d networkStandby : %d\n",freezeDuration,isGPIOWakeup,networkStandby);
        sprintf(details,"CPU freeze duration : %d;isGPIOWakeup : %d;PLAT_DS_SetDeepSleep call is SUCCESS",freezeDuration,isGPIOWakeup);
#else
        DEBUG_PRINT(DEBUG_TRACE, "CPU freeze duration : %d\n",freezeDuration);
        sprintf(details,"CPU freeze duration : %d;PLAT_DS_SetDeepSleep call is SUCCESS",freezeDuration);
#endif //ENABLE_DEEPSLEEP_WAKEUP_EVT
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_SetDeepSleep call success\n");
        response["result"]="SUCCESS";
        response["details"]=details;
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_SetDeepSleep --> Exit\n");
    }
    else{
        response["result"]="FAILURE";
	response["details"]="PLAT_DS_SetDeepSleep call failed" + DeepSleep_Return_Status_toString(ret);
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_SetDeepSleep call failed %s\n",DeepSleep_Return_Status_toString(ret));
        DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_SetDeepSleep --> Exit\n");
    }
#else
    response["result"]="FAILURE";
    response["details"]="DeepSleepHal Not Supported";
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal Not Supported\n");
    DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_SetDeepSleep --> Exit\n");
#endif //ENABLE_DEEP_SLEEP

    return;
}

/***************************************************************************
 *Function name  : check_wakereason_status
 *Description    : This function is to return the string of PLAT_DS_GetLastWakeupReason
 *****************************************************************************/
void check_wakereason_status(string *wakeup_str, DeepSleep_WakeupReason_t wakeupreason)
{
	switch(wakeupreason)
	{
		case DEEPSLEEP_WAKEUPREASON_IR:
			*wakeup_str = "IR";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_RCU_BT:
			*wakeup_str = "RCU BT";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_RCU_RF4CE:
			*wakeup_str = "RCU RF4CE";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_GPIO:
			*wakeup_str = "GPIO";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_LAN:
			*wakeup_str = "LAN";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_WLAN:
			*wakeup_str = "WLAN";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_TIMER:
			*wakeup_str = "TIMER";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_FRONT_PANEL:
			*wakeup_str = "FP";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_WATCHDOG:
			*wakeup_str = "WATCH_DOG";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET:
			*wakeup_str = "SW_RESET";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_THERMAL_RESET:
			*wakeup_str = "THERMAL_RESET";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_WARM_RESET:
			*wakeup_str = "WARM_RESET";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_COLDBOOT:
			*wakeup_str = "COLDBOOT";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE:
			*wakeup_str = "STR_AUTH_FAIL";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_CEC:
			*wakeup_str = "CEC";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_PRESENCE:
			*wakeup_str = "MOTION_DETECT";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_VOICE:
			*wakeup_str = "VOICE";
		break;
		
		case DEEPSLEEP_WAKEUPREASON_UNKNOWN:
			*wakeup_str = "UNKNOWN";
		break;
		
		default:
			*wakeup_str = "NONE";
	}
	
	return;
}

/***************************************************************************
 *Function name  : DeepSleepHal_GetLastWakeupReason
 *Description    : This function is to invoke PLAT_DS_GetLastWakeupReason
 *****************************************************************************/
void DeepSleepHalAgent::DeepSleepHal_GetLastWakeupReason(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_GetLastWakeupReason --->Entry\n");
        char details[100];
        DeepSleep_WakeupReason_t wakeupreason;
        DeepSleep_Return_Status_t  ret = DEEPSLEEPMGR_SUCCESS;
        string wakeup_str;

        int Is_null_param_check = (int) req["Is_null_param_check"].asInt();
        if(Is_null_param_check)
        {
                ret = PLAT_DS_GetLastWakeupReason(NULL);
        }
        else
        {
                ret = PLAT_DS_GetLastWakeupReason(&wakeupreason);
        }

        if(DEEPSLEEPMGR_SUCCESS == ret)
        {
                DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupReason call success\n");
                DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupReason val: %d\n", wakeupreason);
                check_wakereason_status(&wakeup_str, wakeupreason);
                DEBUG_PRINT(DEBUG_TRACE, "Last Wakeup Reason:%s\n",wakeup_str.c_str());

                sprintf(details, "Last Wakeup Reason:%s",wakeup_str.c_str());

                response["result"] = "SUCCESS";
                response["details"] = details;
                DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupReason --> Exit\n");
        }
        else
        {
                response["result"]="FAILURE";
                response["details"] = "PLAT_DS_GetLastWakeupReason call failed" + DeepSleep_Return_Status_toString(ret);
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupReason call failed %s\n",DeepSleep_Return_Status_toString(ret));
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupReason --> Exit\n");
        }

        return;
}

/***************************************************************************
 *Function name  : DeepSleepHal_GetLastWakeupKeyCode
 *Description    : This function is to invoke PLAT_DS_GetLastWakeupKeyCode
 *****************************************************************************/
void DeepSleepHalAgent::DeepSleepHal_GetLastWakeupKeyCode(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "DeepSleepHal_GetLastWakeupKeyCode --->Entry\n");
	char details[100];
	DeepSleepMgr_WakeupKeyCode_Param_t wakeupkeycode;
	DeepSleep_Return_Status_t  ret = DEEPSLEEPMGR_SUCCESS;
	
	int Is_null_param_check = (int) req["Is_null_param_check"].asInt();
	if(Is_null_param_check)
	{
		ret = PLAT_DS_GetLastWakeupKeyCode(NULL);
	}
	else
	{
		ret = PLAT_DS_GetLastWakeupKeyCode(&wakeupkeycode);
	}
	
	if(DEEPSLEEPMGR_SUCCESS == ret)
	{
		DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupKeyCode call success\n");
		DEBUG_PRINT(DEBUG_TRACE, "Last Wakeup Keycode:0x%X\n",wakeupkeycode);
		
		sprintf(details,"Wakeup Keycode: hex:0x%X, dec: %u",wakeupkeycode,wakeupkeycode);
		response["result"] = "SUCCESS";
		response["details"] = details;
		DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupKeyCode --> Exit\n");
	}
	else
	{
		response["result"]="FAILURE";
		response["details"] = "PLAT_DS_GetLastWakeupKeyCode call failed" + DeepSleep_Return_Status_toString(ret);
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupKeyCode call failed %s\n",DeepSleep_Return_Status_toString(ret));
        DEBUG_PRINT(DEBUG_TRACE, "PLAT_DS_GetLastWakeupKeyCode --> Exit\n");
	}
	
	return;
}

/**************************************************************************
Function Name   : cleanup
Arguments       : NULL
Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool DeepSleepHalAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Exit\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name : DestroyObject
Arguments     : Input argument is DeepSleepHalAgent Object
Description   : This function will be used to destory the DSHalAgent object.
**************************************************************************/
extern "C" void DestroyObject(DeepSleepHalAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying DeepSleepHal Agent object\n");
        delete stubobj;
}
