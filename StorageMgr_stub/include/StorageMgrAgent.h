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
#ifndef __STORAGEMGR_STUB_H__
#define __STORAGEMGR_STUB_H__
#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include "rdkteststubintf.h"
#include "rdktestagentintf.h"
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "libIARMCore.h"
#include "rdkStorageMgr.h"

#include <jsonrpccpp/server/connectors/tcpsocketserver.h>

#define IN
#define OUT

#define TEST_SUCCESS true
#define TEST_FAILURE false


using namespace std;
string error = "";

class RDKTestAgent;
class StorageMgrAgent : public RDKTestStubInterface , public AbstractServer<StorageMgrAgent>
{
        public:
                //Constructor
                StorageMgrAgent(TcpSocketServer &ptrRpcServer) : AbstractServer <StorageMgrAgent>(ptrRpcServer)
                {
                    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetDeviceID", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetDeviceID);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetTSBStatus", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetTSBStatus);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetTSBCapacity", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetTSBCapacity);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetTSBCapacityMinutes", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetTSBCapacityMinutes);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetTSBFreeSpace", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetTSBFreeSpace);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetDVRCapacity", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetDVRCapacity);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetDVRFreeSpace", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetDVRFreeSpace);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetDeviceInfo", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetDeviceInfo);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetDeviceInfoList", PARAMS_BY_NAME, JSON_STRING, "DeviceID", JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetDeviceInfoList);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetTSBMaxMinutes", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetTSBMaxMinutes);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_SetTSBMaxMinutes", PARAMS_BY_NAME, JSON_STRING, "TSBminutes", JSON_INTEGER, NULL), &StorageMgrAgent::StorageMgr_SetTSBMaxMinutes);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_IsTSBEnabled", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_IsTSBEnabled);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_SetTSBEnabled", PARAMS_BY_NAME, JSON_STRING, "isEnabled", JSON_INTEGER, NULL), &StorageMgrAgent::StorageMgr_SetTSBEnabled);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_IsDVREnabled", PARAMS_BY_NAME, JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_IsDVREnabled);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_SetDVREnabled", PARAMS_BY_NAME, JSON_STRING, "isDVREnabled", JSON_INTEGER, NULL), &StorageMgrAgent::StorageMgr_SetDVREnabled);
		    this->bindAndAddMethod(Procedure("TestMgr_StorageMgr_GetHealth", PARAMS_BY_NAME, JSON_STRING, "DeviceID", JSON_STRING, NULL), &StorageMgrAgent::StorageMgr_GetHealth);
		}
				
				
				
		/*inherited functions*/
		bool initialize(IN const char* szVersion);
		std::string testmodulepre_requisites();
                bool testmodulepost_requisites();
				
		void StorageMgr_GetDeviceID(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetTSBStatus(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetTSBCapacity(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetTSBCapacityMinutes(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetTSBFreeSpace(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetDVRCapacity(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetDVRFreeSpace(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetDeviceInfo(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetDeviceInfoList(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetTSBMaxMinutes(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_SetTSBMaxMinutes(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_IsTSBEnabled(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_SetTSBEnabled(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_IsDVREnabled(IN const Json::Value& req, OUT Json::Value& response);
                void StorageMgr_SetDVREnabled(IN const Json::Value& req, OUT Json::Value& response);
		void StorageMgr_GetHealth(IN const Json::Value& req, OUT Json::Value& response);
				
		bool cleanup(IN const char* szVersion);
				
};
#endif //__STORAGEMGR_STUB_H__		
