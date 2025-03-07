/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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

#ifndef __RDKFWUPGRADER_STUB_H__
#define __RDKFWUPGRADER_STUB_H__

#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <csignal>
#include <csetjmp>
#include <cstring>
#include <sstream>
#include "rdkteststubintf.h"
#include "rdktestagentintf.h"

//Component specific header files
extern "C" {
   #include "deviceutils.h"
   #include "device_api.h"
}

#include <jsonrpccpp/server/connectors/tcpsocketserver.h>

#define IN
#define OUT

#define TEST_SUCCESS true
#define TEST_FAILURE false

#define BUFFER_SIZE_LONG          1024
#define DEFAULT_DATA_SIZE         1024

using namespace std;

class RDKTestAgent;
class RDK_fwupgradeAgent : public RDKTestStubInterface , public AbstractServer<RDK_fwupgradeAgent>
{
        public:
                //Constructor
                RDK_fwupgradeAgent(TcpSocketServer &ptrRpcServer) : AbstractServer <RDK_fwupgradeAgent>(ptrRpcServer)
                {
                    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_RunCommand", PARAMS_BY_NAME, JSON_STRING, "command", JSON_STRING, "argument",  JSON_STRING, "result_empty" , JSON_INTEGER , "result_size_empty" , JSON_INTEGER, "result_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_RunCommand);
                    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_makeHttpHttps", PARAMS_BY_NAME, JSON_STRING, "string", JSON_STRING, "string_size" , JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_makeHttpHttps);
                    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_stripinvalidchar", PARAMS_BY_NAME, JSON_STRING, "string", JSON_STRING, "string_size" , JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_stripinvalidchar);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_MemDLAlloc", PARAMS_BY_NAME, JSON_STRING, "allocate_size", JSON_INTEGER, "null_param", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_MemDLAlloc);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_getJsonRpc", PARAMS_BY_NAME, JSON_STRING, "string", JSON_STRING, "null_param" , JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_getJsonRpc);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_getJRPCTokenData", PARAMS_BY_NAME, JSON_STRING, "string", JSON_STRING, "null_token", JSON_INTEGER, "null_tokenSize", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_getJRPCTokenData);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_get_system_uptime", PARAMS_BY_NAME, JSON_STRING, "null_param" , JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_get_system_uptime);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_getMetaDataFile", PARAMS_BY_NAME, JSON_STRING, "null_param" , JSON_INTEGER, "directory", JSON_STRING, NULL), &RDK_fwupgradeAgent::rdkfwupdater_getMetaDataFile);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_mergeLists", PARAMS_BY_NAME, JSON_STRING, "list1", JSON_STRING, "list2" , JSON_STRING, "null_list1", JSON_INTEGER, "null_list2", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_mergeLists);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetServerURL", PARAMS_BY_NAME, JSON_STRING, "filename", JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, "filename_null_check", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetServerURL);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetTimezone", PARAMS_BY_NAME, JSON_STRING, "cpuarch", JSON_STRING, "null_param", JSON_INTEGER, "cpuarch_null_param_check", JSON_INTEGER, "buffer_size", JSON_INTEGER,  NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetTimezone);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetAdditionalFwVerInfo", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetAdditionalFwVerInfo);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetPDRIFileName", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetPDRIFileName);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetUTCTime", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetUTCTime);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetCapabilities", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetCapabilities);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetPartnerId", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetPartnerId);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetSerialNum", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetSerialNum);
		    this->bindAndAddMethod(Procedure("TestMgr_rdkfwupdater_GetAccountID", PARAMS_BY_NAME, JSON_STRING, "null_param", JSON_INTEGER, "buffer_size", JSON_INTEGER, NULL), &RDK_fwupgradeAgent::rdkfwupdater_GetAccountID);
                }


                //Inherited functions
                bool initialize(IN const char* szVersion);
                bool cleanup(const char* szVersion);
                std::string testmodulepre_requisites();
                bool testmodulepost_requisites();

                //Stub functions
                void rdkfwupdater_RunCommand(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_makeHttpHttps(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_stripinvalidchar(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_MemDLAlloc(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_getJsonRpc(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_getJRPCTokenData(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_get_system_uptime(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_getMetaDataFile(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_mergeLists(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetServerURL(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetTimezone(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetAdditionalFwVerInfo(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetPDRIFileName(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetUTCTime(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetCapabilities(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetPartnerId(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetSerialNum(IN const Json::Value& req, OUT Json::Value& response);
		void rdkfwupdater_GetAccountID(IN const Json::Value& req, OUT Json::Value& response);
};
#endif //__RDKFWUPGRADER_STUB_H__


