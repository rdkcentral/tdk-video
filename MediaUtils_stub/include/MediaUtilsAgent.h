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

#ifndef __MEDIAUTILS_STUB_H__
#define __MEDIAUTILS_STUB_H__

#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include "rdkteststubintf.h"
#include "rdktestagentintf.h"
#include "rmfAudioCapture.h"
#include "rmf_error.h"
#include <iostream>
#include <stddef.h>
#include <string>
#include <sys/time.h>
#include <jsonrpccpp/server/connectors/tcpsocketserver.h>
#include <thread>
#include <chrono>

#define IN
#define OUT

#define BUFF_LENGTH 512
#define TEST_SUCCESS true
#define TEST_FAILURE false

using namespace std;

RMF_AudioCaptureHandle handle;
rmf_Error audiocapture_Open;
rmf_Error audiocapture_Close;
RMF_AudioCaptureHandle *retHandle;
RMF_AudioCapture_Status status;
RMF_AudioCapture_Settings settings;
string paramHandle;

class RDKTestAgent;
class MediaUtilsAgent : public RDKTestStubInterface  , public AbstractServer<MediaUtilsAgent>
{
        public:
                //Constructor
                MediaUtilsAgent(TcpSocketServer &ptrRpcServer) : AbstractServer <MediaUtilsAgent>(ptrRpcServer)
                {
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_AudioCapture_Open", PARAMS_BY_NAME, JSON_STRING,NULL), &MediaUtilsAgent::MediaUtils_AudioCapture_Open);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_Get_DefaultSettings", PARAMS_BY_NAME, JSON_STRING,"Isnullparamcheck", JSON_INTEGER, NULL), &MediaUtilsAgent::MediaUtils_Get_DefaultSettings);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_Get_Current_Settings", PARAMS_BY_NAME, JSON_STRING,"paramHandle", JSON_STRING,"Isnullparamcheck", JSON_INTEGER,NULL), &MediaUtilsAgent::MediaUtils_Get_Current_Settings);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_Get_Status", PARAMS_BY_NAME, JSON_STRING,"paramhandle", JSON_INTEGER, "Isnullparamcheck", JSON_INTEGER, "IsHandleInvalid", JSON_INTEGER,NULL), &MediaUtilsAgent::MediaUtils_Get_Status);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_AudioCaptureStart", PARAMS_BY_NAME, JSON_STRING,"paramBufferReady", JSON_STRING,"paramHandle", JSON_STRING,"paramFifosize", JSON_STRING,NULL), &MediaUtilsAgent::MediaUtils_AudioCaptureStart);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_AudioCaptureStop", PARAMS_BY_NAME, JSON_STRING,"paramHandle", JSON_STRING,NULL), &MediaUtilsAgent::MediaUtils_AudioCaptureStop);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_AudioCapture_Close", PARAMS_BY_NAME, JSON_STRING,"paramHandle", JSON_STRING,NULL), &MediaUtilsAgent::MediaUtils_AudioCapture_Close);
                 this->bindAndAddMethod(Procedure("TestMgr_MediaUtils_ExecuteCmd", PARAMS_BY_NAME, JSON_STRING,"command", JSON_STRING,NULL), &MediaUtilsAgent::MediaUtils_ExecuteCmd);
                }


                //Inherited functions
                bool initialize(IN const char* szVersion);

                bool cleanup(IN const char* szVersion);
                std::string testmodulepre_requisites();
                bool testmodulepost_requisites();

                //Stub functions
		void MediaUtils_AudioCapture_Open(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_Get_DefaultSettings(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_Get_Current_Settings(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_Get_Status(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_AudioCaptureStart(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_AudioCaptureStop(IN const Json::Value& req, OUT Json::Value& response);
		void MediaUtils_AudioCapture_Close(IN const Json::Value& req, OUT Json::Value& response);
                void MediaUtils_ExecuteCmd(IN const Json::Value& req, OUT Json::Value& response);
};
#endif //__MEDIAUTILS_STUB_H__

