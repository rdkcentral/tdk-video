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


#include "StorageMgrAgent.h"
#include "rdkStorageMgr.h"
#include "libIBus.h"
#include "libIARM.h"
#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "libIARMCore.h"

#define DEVICEID_LENGTH	35
eSTMGRDeviceIDs getDeviceIDs;
eSTMGRReturns rc;

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/
std::string StorageMgrAgent::testmodulepre_requisites()
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr testmodule pre_requisites --> Entry\n");
	
	/*initializing IARMBUS library */
	IARM_Result_t retval;
	retval=IARM_Bus_Init("agent");
	DEBUG_PRINT(DEBUG_LOG,"\nInit retval:%d\n",retval);
	if(retval==0)
	{
		DEBUG_PRINT(DEBUG_LOG,"\n Application Successfully initializes the IARMBUS library\n");
	}
	else
	{
		DEBUG_PRINT(DEBUG_LOG,"\n Application failed to initializes the IARMBUS library\n");
	}
	DEBUG_PRINT(DEBUG_LOG,"\n Calling IARM_BUS_Connect\n");
	/*connecting application with IARM BUS*/
	IARM_Bus_Connect();
	DEBUG_PRINT(DEBUG_LOG,"\n Application Successfully connected with IARMBUS \n");

    	rdkStorage_init();
	
	
    	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr testmodule pre_requisites --> Exit\n");
    	return "SUCCESS";
}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Description   : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool StorageMgrAgent::testmodulepost_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "StorageMgr testmodule post_requisites --> Entry\n");
    DEBUG_PRINT(DEBUG_TRACE, "StorageMgr testmodule post_requisites --> Exit\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name   : CreateObject
Arguments       : NULL
Description     : This function is used to create a new object of the class "StorageMgrAgent".
**************************************************************************/
extern "C" StorageMgrAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new StorageMgrAgent(ptrtcpServer);
}



/***************************************************************************
 *Function name : initialize
 *Description    : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/
bool StorageMgrAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "StorageMgr Initialization Entry\n");
    DEBUG_PRINT (DEBUG_TRACE, "StorageMgr Initialization Exit\n");
    return TEST_SUCCESS;
}



void checkERROR(eSTMGRReturns rc,string *error)
{
	switch(rc)
	{
		case RDK_STMGR_RETURN_GENERIC_FAILURE: DEBUG_PRINT(DEBUG_TRACE, "Returns Generic Failure : RDK_STMGR_RETURN_GENERIC_FAILURE\n");
						       *error="ERROR : RDK_STMGR_RETURN_GENERIC_FAILURE\n";
						       break;
		case RDK_STMGR_RETURN_INIT_FAILURE: DEBUG_PRINT(DEBUG_TRACE, "Returns Init Failure : RDK_STMGR_RETURN_INIT_FAILURE\n");
						      *error="ERROR : RDK_STMGR_RETURN_INIT_FAILURE\n";
						       break;
		case RDK_STMGR_RETURN_INVALID_INPUT: DEBUG_PRINT(DEBUG_TRACE, "Return Invalid Input : RDK_STMGR_RETURN_INVALID_INPUT\n");
						       *error="ERROR : RDK_STMGR_RETURN_INVALID_INPUT\n";
						       break;
		case RDK_STMGR_RETURN_UNKNOWN_FAILURE: DEBUG_PRINT(DEBUG_TRACE, "Unknown Failure : RDK_STMGR_RETURN_UNKNOWN_FAILURE\n");
						       *error="ERROR : RDK_STMGR_RETURN_UNKNOWN_FAILURE\n";
						       break;
		default :DEBUG_PRINT(DEBUG_ERROR, "UNEXPECTED ERROR OBSERVED\n");
	      					       *error="ERROR:UNEXPECTED ERROR";
	}
}


void DeviceInfo (const eSTMGRDeviceInfo &deviceInfo)
{
	DEBUG_PRINT(DEBUG_TRACE, "Device ID           = %s\n", deviceInfo.m_deviceID);
        DEBUG_PRINT(DEBUG_TRACE, "Device Type         = %s\n", deviceInfo.m_type);
        DEBUG_PRINT(DEBUG_TRACE, "Capacity            = %u\n", deviceInfo.m_capacity); // Use %u for unsigned int
        DEBUG_PRINT(DEBUG_TRACE, "Status              = %s\n", deviceInfo.m_status);
        DEBUG_PRINT(DEBUG_TRACE, "Partitions          = %u\n", deviceInfo.m_partitions); // Assuming m_partitions is also unsigned int
        DEBUG_PRINT(DEBUG_TRACE, "Manufacturer        = %s\n", deviceInfo.m_manufacturer);
        DEBUG_PRINT(DEBUG_TRACE, "Model               = %s\n", deviceInfo.m_model);
        DEBUG_PRINT(DEBUG_TRACE, "SerialNumber        = %s\n", deviceInfo.m_serialNumber);
        DEBUG_PRINT(DEBUG_TRACE, "FirmwareVersion     = %s\n", deviceInfo.m_firmwareVersion);
        DEBUG_PRINT(DEBUG_TRACE, "IfATAstandard       = %s\n", deviceInfo.m_ifATAstandard);
        DEBUG_PRINT(DEBUG_TRACE, "HasSMARTSupport     = %u\n", deviceInfo.m_hasSMARTSupport); // Assuming m_hasSMARTSupport is unsigned int


}


/***************************************************************************
 *Function name  : getDeviceIds
 *Description    : This function is to get the device ID
 *****************************************************************************/

bool printDeviceIDs(std::string &DeviceIDs)
{
    	if (getDeviceIDs.m_numOfDevices > 0)
    	{
        	for (int i = 0; i < getDeviceIDs.m_numOfDevices; i++)
        	{
			char buffer[256]; 
            		sprintf(buffer, "%d : %s\n", (i + 1), getDeviceIDs.m_deviceIDs[i]);
            		DEBUG_PRINT(DEBUG_TRACE, "%s", buffer);
            		DeviceIDs += buffer;
        	}
    	}
    	else
    	{
        	DEBUG_PRINT (DEBUG_TRACE, "Devices are not found\n");
		return false;
    	}
    	return true;
}


/***************************************************************************
 *Function name  : getDeviceIds
 *Description    : This function is to get the device ID
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetDeviceID(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GETDEVICEID --->Entry\n");
	eSTMGRReturns rc;
    	rc = rdkStorage_getDeviceIds(&getDeviceIDs);
    	if (rc != RDK_STMGR_RETURN_SUCCESS)
    	{
        	DEBUG_PRINT (DEBUG_TRACE, "failed to get DeviceId\n");
		response["result"] = "FAILURE";
		checkERROR(rc,&error);
		response["details"] = "rdkStorage_getDeviceIds failed"+error;
    	}
    	else
    	{
		std::string DeviceIDs;
        	if (printDeviceIDs(DeviceIDs))
		{
			response["result"] = "SUCCESS";
			response["details"] = DeviceIDs;
			return;
		}
		else
		{
			response["result"] = "FAILURE";
			response["details"] = "No Devices are found";
			return;
		}

    	}

}



/***************************************************************************
 *Function name  : getTSBStatus
 *Description    : This function is to get TSB Status
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetTSBStatus(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetTSBStatus --->Entry\n");
	eSTMGRTSBStatus TSBStatus;
    	eSTMGRReturns rc;
	string status;
    	rc = rdkStorage_getTSBStatus (&TSBStatus);
	DEBUG_PRINT(DEBUG_TRACE, "TSB Status = %d \n", TSBStatus);


	switch(TSBStatus)
        {
                        case RDK_STMGR_TSB_STATUS_OK:
                                DEBUG_PRINT(DEBUG_TRACE, "SUCCESS : Status OK\n");
				status = "SUCCESS : Status OK";
                                break;
                        case RDK_STMGR_TSB_STATUS_READ_ONLY:
                                DEBUG_PRINT(DEBUG_TRACE, "SUCCESS : Read Only Status\n");
				status = "SUCCESS : Read Only Status";
                                break;
                        case RDK_STMGR_TSB_STATUS_NOT_PRESENT:
                                DEBUG_PRINT(DEBUG_TRACE, "FAILURE : Status Not Present\n");
				status = "FAILURE : Status Not Present";
                                break;
                        case RDK_STMGR_TSB_STATUS_NOT_QUALIFIED:
                                DEBUG_PRINT(DEBUG_TRACE, "Status Not Qualified\n");
				status = "FAILURE : Status Not Qualified";
                                break;
                        case RDK_STMGR_TSB_STATUS_DISK_FULL:
                                DEBUG_PRINT(DEBUG_TRACE, "Status Disk Full\n");
				status = "FAILURE : Status Disk Full";
                                break;
                        case RDK_STMGR_TSB_STATUS_READ_FAILURE:
                                DEBUG_PRINT(DEBUG_TRACE, "Status Read Failure\n");
				status = "FAILURE : Status Read Failure";
                                break;
                        case RDK_STMGR_TSB_STATUS_WRITE_FAILURE:
                                DEBUG_PRINT(DEBUG_TRACE, "Status Write Failure\n");
				status = "FAILURE : Status Write Failure";
                                break;
                        case RDK_STMGR_TSB_STATUS_UNKNOWN:
                                DEBUG_PRINT(DEBUG_TRACE, "Status Unknown\n");
				status = "FAILURE : Status Unknown";
                                break;
                        default: DEBUG_PRINT(DEBUG_TRACE, "Invalid Status\n");
				 status = "FAILURE : Invalid Status";

        }

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
		response["result"] = "SUCCESS";
                response["details"] = status;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the TSB Status\n");
		checkERROR(rc,&error);
		response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getTSBStatus failed"+error;

    	}
}

/***************************************************************************
 *Function name  : getTSBCapacity
 *Description    : This function is to get TSB Capacity
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetTSBCapacity(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetTSBCapacity --->Entry\n");
	unsigned long long TSBCapacity =0;
    	rc = rdkStorage_getTSBCapacity(&TSBCapacity);
	DEBUG_PRINT(DEBUG_TRACE, "TSB Capacity = %llu \n", TSBCapacity);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
                response["result"] = "SUCCESS";
                response["details"] = TSBCapacity;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the TSB Capacity\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getTSBCapacity failed"+error;
    	}
}


/***************************************************************************
 *Function name  : getTSBCapacityMinutes
 *Description    : This function is to get TSB Capacity minutes
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetTSBCapacityMinutes(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetTSBCapacityMinutes --->Entry\n");
	unsigned int capacityminutes =0;
    	rc = rdkStorage_getTSBCapacityMinutes(&capacityminutes);
	
	DEBUG_PRINT(DEBUG_TRACE, "TSB Capacity Minutes (in Mins) = %u \n", capacityminutes);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
                response["result"] = "SUCCESS";
                response["details"] = capacityminutes;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the TSB Capacity Minutes\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getTSBCapacityMinutes failed"+error;
    	}
}


/***************************************************************************
 *Function name  : getTSBFreeSpace
 *Description    : This function is to get free space in TSB
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetTSBFreeSpace(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetTSBFreeSpace --->Entry\n");
	unsigned long long freeSpaceTSB =0;

    	rc = rdkStorage_getTSBFreeSpace(&freeSpaceTSB);

	DEBUG_PRINT(DEBUG_TRACE, "Free space in TSB  = %llu \n", freeSpaceTSB);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
                response["result"] = "SUCCESS";
                response["details"] = freeSpaceTSB;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the Free space in TSB\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getTSBFreeSpace failed"+error;
    	}
}


/***************************************************************************
 *Function name  : getDVRCapacity
 *Description    : This function is to get TSB capacity
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetDVRCapacity(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetDVRCapacity --->Entry\n");
	unsigned long long DVRcapacity =0;

    	rc = rdkStorage_getDVRCapacity(&DVRcapacity);
	
	DEBUG_PRINT(DEBUG_TRACE, "DVR Capacity  = %llu \n", DVRcapacity);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
                response["result"] = "SUCCESS";
                response["details"] = DVRcapacity;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the DVR Capacity\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getDVRcapacity failed"+error;
        	
    	}
}


/***************************************************************************
 *Function name  : getDVRFreeSpace
 *Description    : This function is to get the free space in DVR
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetDVRFreeSpace(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetDVRFreeSpace --->Entry\n");
	unsigned long long freeSpaceDVR =0;
    	
    	rc = rdkStorage_getDVRFreeSpace(&freeSpaceDVR);
	DEBUG_PRINT(DEBUG_TRACE, "DVR Free Space  = %llu \n", freeSpaceDVR);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
                response["result"] = "SUCCESS";
                response["details"] = freeSpaceDVR;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the DVR Free Space\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getDVRFreeSpace failed"+error;
    	}
}



/***************************************************************************
 *Function name  : getDeviceInfo
 *Description    : This function is to get Device Info
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetDeviceInfo(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetDeviceInfo --->Entry\n");

	eSTMGRDeviceInfo deviceInfo;

	memset (&deviceInfo, 0, sizeof(deviceInfo));

	if(&req["DeviecID"] == NULL)
        {
                return;
        }

	char DeviceID[DEVICEID_LENGTH] = {'\0'};
        strcpy (DeviceID, req["DeviceID"].asCString());

	rc = rdkStorage_getDeviceInfo(DeviceID, &deviceInfo);

   	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
		DeviceInfo (deviceInfo);
		response["result"] = "SUCCESS";
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "No devices are present to get the info\n");
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getDeviceInfo failed"+error;
    	}
}

	



/***************************************************************************
 *Function name  : getDeviceInfoList
 *Description    : This function is to get Device Info list
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetDeviceInfoList(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetDeviceInfoList --->Entry\n");
	eSTMGRDeviceInfoList deviceInfolist;
    	rc = rdkStorage_getDeviceInfoList(&deviceInfolist);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{

		if (deviceInfolist.m_numOfDevices > 0)
                {
                        for (int i = 0; i < deviceInfolist.m_numOfDevices; i++)
                        {
				DeviceInfo (deviceInfolist.m_devices[i]);
				response["result"] = "SUCCESS";
			}
		}
		else
		{
			DEBUG_PRINT(DEBUG_TRACE, "No devices are present to get the info\n");
                        response["result"] = "FAILURE";
                        response["details"] = "rdkStorage_getDeviceInfoList failed";
		}
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the Device Info\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getDeviceInfoList failed"+error;
    	}
	return;
}



/***************************************************************************
 *Function name  : getTSBMaxMinutes
 *Description    : This function is to get the TSB Maximum minutes
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetTSBMaxMinutes(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetTSBMaxMinutes --->Entry\n");

	unsigned int TSBminutes =0;
	rc = rdkStorage_getTSBMaxMinutes (&TSBminutes);

	DEBUG_PRINT(DEBUG_TRACE, "TSB Max Minutes = %u \n", TSBminutes);

    	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
		response["result"] = "SUCCESS";
                response["details"] = TSBminutes;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to get the TSB Max minutes\n");
		checkERROR(rc,&error);
		response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getTSBMaxMinutes failed"+error;
    	}
	return;
}


/***************************************************************************
 *Function name  : setTSBMaxMinutes
 *Description    : This function is to set the TSB Maximum minutes
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_SetTSBMaxMinutes(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_SetTSBMaxMinutes --->Entry\n");

	if(&req["TSBminutes"] == NULL)
    	{
        	return;
    	}

	unsigned int TSBminutes = req["TSBminutes"].asInt();

	rc = rdkStorage_setTSBMaxMinutes (TSBminutes);

	DEBUG_PRINT(DEBUG_TRACE, "Setting TSB Max Minutes = %u \n", TSBminutes);

	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
		response["result"] = "SUCCESS";
                response["details"] = TSBminutes;
    	}
   	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to set the TSB Max minutes\n");
		checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_setTSBMaxMinutes failed"+error;
    	}
	return;
}


/***************************************************************************
 *Function name  : isTSBEnabled
 *Description    : This function is to check TSB is enabled or not 
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_IsTSBEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_IsTSBEnabled --->Entry\n");

	bool rc = rdkStorage_isTSBEnabled();

	DEBUG_PRINT(DEBUG_TRACE, "Is TSB Enabled %d \n", rc);

	response["result"] = "SUCCESS";
	response["details"] = rc;
}

/***************************************************************************
 *Function name  : setTSBEnabled
 *Description    : This function is to set TSB is enable
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_SetTSBEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_SetTSBEnabled --->Entry\n");

	if(&req["isEnabled"] == NULL)
        {
                return;
        }
	bool isEnabled = req["isEnabled"].asInt();

	rc = rdkStorage_setTSBEnabled (isEnabled);

	DEBUG_PRINT(DEBUG_TRACE, "Setting TSB Enabled = %d \n", isEnabled);

	if (rc == RDK_STMGR_RETURN_SUCCESS)
    	{
		response["result"] = "SUCCESS";	
		response["details"] = isEnabled;
    	}
    	else
    	{
		DEBUG_PRINT(DEBUG_TRACE, "Failed to set the TSB Enabled\n");
                checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_setTSBEnabled failed"+error;
    	}
}



/***************************************************************************
 *Function name  : isDVREnabled
 *Description    : This function is to check DVR is enabled or not
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_IsDVREnabled(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_IsDVREnabled --->Entry\n");

        bool rc = rdkStorage_isDVREnabled();

        response["result"] = "SUCCESS";
        response["details"] = rc;
}


/***************************************************************************
 *Function name  : setTSBEnabled
 *Description    : This function is to set TSB is enable
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_SetDVREnabled(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_SetDVREnabled --->Entry\n");

        if(&req["isDVREnabled"] == NULL)
        {
                return;
        }

	bool isDVREnabled = req["isDVREnabled"].asInt();

        rc = rdkStorage_setDVREnabled (isDVREnabled);

        DEBUG_PRINT(DEBUG_TRACE, "Setting DVR Enabled = %d \n", isDVREnabled);

        if (rc == RDK_STMGR_RETURN_SUCCESS)
        {
                response["result"] = "SUCCESS";
                response["details"] = isDVREnabled;
        }
        else
        {
                DEBUG_PRINT(DEBUG_TRACE, "Failed to set the DVR Enabled\n");
                checkERROR(rc,&error);
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_setDVREnabled failed"+error;
        }
}


/***************************************************************************
 *Function name  : setTSBEnabled
 *Description    : This function is to set TSB is enable
 *****************************************************************************/
void StorageMgrAgent::StorageMgr_GetHealth(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "StorageMgr_GetHealth --->Entry\n");

	eSTMGRHealthInfo healthInfo;

        if(&req["DeviecID"] == NULL)
        {
                return;
        }
	char DeviceID[DEVICEID_LENGTH] = {'\0'};
	strcpy (DeviceID, req["DeviceID"].asCString());

	DEBUG_PRINT(DEBUG_TRACE, "Querying Device_ID for get Health %s\n", DeviceID);

	rc = rdkStorage_getHealth (DeviceID, &healthInfo);

	if (rc == RDK_STMGR_RETURN_SUCCESS)
        {
		DEBUG_PRINT(DEBUG_TRACE, "Device_ID           = %s\n", healthInfo.m_deviceID);
		DEBUG_PRINT(DEBUG_TRACE, "Device_Type         = %s\n", healthInfo.m_deviceType);
		DEBUG_PRINT(DEBUG_TRACE, "Is_Operational      = %d\n", healthInfo.m_isOperational);
		DEBUG_PRINT(DEBUG_TRACE, "Is_Healthy          = %d\n", healthInfo.m_isHealthy);
                response["result"] = "SUCCESS";
        }
        else
        {
                DEBUG_PRINT(DEBUG_TRACE, "Failed to get the Device Health\n");
                response["result"] = "FAILURE";
                response["details"] = "rdkStorage_getHealth failed"+error;
        }
}

/**************************************************************************
Function Name   : cleanup
Arguments       : NULL
Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool StorageMgrAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Exit\n");
    return TEST_SUCCESS;
}
/**************************************************************************
Function Name : DestroyObject
Arguments     : Input argument is StorageMgrAgent Object
Description   : This function will be used to destory the StorageMgrAgent object.
**************************************************************************/
extern "C" void DestroyObject(StorageMgrAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying StorageMgr Agent object\n");
        delete stubobj;
}
