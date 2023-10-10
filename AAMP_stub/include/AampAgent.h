/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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


#ifndef __AAMP_STUB_H__
#define __AAMP_STUB_H__

#include <time.h>
#include <unistd.h>
#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>
#include "rdkteststubintf.h"
#include "rdktestagentintf.h"
#include <jsonrpccpp/server/connectors/tcpsocketserver.h>
#define TEST_SUCCESS true
#define TEST_FAILURE false
#define ERROR_DESCRIPTION_LENGTH 256

#define AUDIO_VOL_MIN 0
#define AUDIO_VOL_MAX 100

#include "main_aamp.h"
#include <gst/gst.h>
#ifndef REF_PLATFORMS
#include "manager.hpp"
#include "libIBus.h"
#endif
PlayerInstanceAAMP *mSingleton = NULL ;
std::string *http_proxy = NULL;
std::string *no_proxy = NULL;
std::string *https_proxy = NULL;
static void* test_StreamThread(void *arg);
static GMainLoop *AAMPGstPlayerMainLoop_test = NULL;
GThread *aampMainLoopThread_test = NULL;

using namespace std;
class RDKTestAgent;
class AampAgent : public RDKTestStubInterface , public AbstractServer<AampAgent>
{
        public:
                //Constructor
                AampAgent(TcpSocketServer &ptrRpcServer) : AbstractServer <AampAgent>(ptrRpcServer)
                {
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampTune",PARAMS_BY_NAME,JSON_STRING,"URL",JSON_STRING,NULL), &AampAgent::AampTune);
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampPlayTillEof",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampPlayTillEof);
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampStop",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampStop);
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetRate",PARAMS_BY_NAME,JSON_STRING,"rate",JSON_REAL,"check_rate_value", JSON_STRING, NULL), &AampAgent::AampSetRate);
		    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetLanguage",PARAMS_BY_NAME,JSON_STRING,"language",JSON_STRING,NULL), &AampAgent::AampSetLanguage);
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampIsLive",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampIsLive);
                    this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetCurrentAudioLanguage",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetCurrentAudioLanguage);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSeekLive",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampSeekLive);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetPlaybackDuration",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetPlaybackDuration);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetRateAndSeek",PARAMS_BY_NAME,JSON_STRING,"rate",JSON_REAL,"seconds",JSON_REAL,NULL), &AampAgent::AampSetRateAndSeek);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampProxy",PARAMS_BY_NAME,JSON_STRING,"set",JSON_REAL,"http_proxy",JSON_STRING,"no_proxy",JSON_STRING,"https_proxy",JSON_STRING,NULL), &AampAgent::AampProxy);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAudioBitrates",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetAudioBitrates);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetVideoBitrates",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetVideoBitrates);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAvailableAudioTracks",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetAvailableAudioTracks);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAvailableTextTracks",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetAvailableTextTracks);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAudioVolume",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetAudioVolume);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetAudioVolume",PARAMS_BY_NAME,JSON_STRING,"volume",JSON_INTEGER,NULL), &AampAgent::AampSetAudioVolume);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetVideoZoom",PARAMS_BY_NAME,JSON_STRING,"zoom",JSON_STRING,NULL), &AampAgent::AampSetVideoZoom);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetVideoMute",PARAMS_BY_NAME,JSON_STRING,"mute",JSON_STRING,NULL), &AampAgent::AampSetVideoMute);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetPreferredLanguages",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetPreferredLanguages);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetPreferredLanguages",PARAMS_BY_NAME,JSON_STRING,"languages",JSON_STRING,NULL), &AampAgent::AampSetPreferredLanguages);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetVideoRectangle",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetVideoRectangle);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetVideoRectangle",PARAMS_BY_NAME,JSON_STRING,"x",JSON_INTEGER,"y",JSON_INTEGER,"w",JSON_INTEGER,"h",JSON_INTEGER,NULL), &AampAgent::AampSetVideoRectangle);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetVideoBitrate",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetVideoBitrate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetVideoBitrate",PARAMS_BY_NAME,JSON_STRING,"bitRate",JSON_INTEGER,NULL), &AampAgent::AampSetVideoBitrate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAudioBitrate",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetVideoBitrate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetAudioBitrate",PARAMS_BY_NAME,JSON_STRING,"bitRate",JSON_INTEGER,NULL), &AampAgent::AampSetAudioBitrate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetPlaybackRate",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetPlaybackRate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetState",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetState);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetPlaybackPosition",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetPlaybackPosition);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetCurrentDRM",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetCurrentDRM);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetPreferredDRM",PARAMS_BY_NAME,JSON_STRING,"preferredDrm",JSON_STRING,NULL), &AampAgent::AampSetPreferredDRM);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetPreferredDRM",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetPreferredDRM);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetAudioTrack",PARAMS_BY_NAME,JSON_STRING,"track_index",JSON_INTEGER,NULL), &AampAgent::AampSetAudioTrack);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampGetAudioTrack",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampGetAudioTrack);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampEnableVideoRectangle",PARAMS_BY_NAME,JSON_STRING,"enable",JSON_STRING,NULL), &AampAgent::AampEnableVideoRectangle);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetWesterosSinkConfig",PARAMS_BY_NAME,JSON_STRING,"enable",JSON_STRING,NULL), &AampAgent::AampSetWesterosSinkConfig);
		   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampCheckPlaybackRate",PARAMS_BY_NAME,JSON_STRING,NULL), &AampAgent::AampCheckPlaybackRate);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampPauseAtPosition",PARAMS_BY_NAME,JSON_STRING,"position",JSON_REAL,NULL), &AampAgent::AampPauseAtPosition);
                   this->bindAndAddMethod(Procedure("TestMgr_Aamp_AampSetLicenseServerURL",PARAMS_BY_NAME,JSON_STRING,"DRM",JSON_STRING,"license_server_url",JSON_STRING,NULL), &AampAgent::AampSetLicenseServerURL);

                }

                //Inherited functions
                bool initialize(IN const char* szVersion);
		void AampTune(IN const Json::Value& req, OUT Json::Value& response);
                void AampStop(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetRate(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetLanguage(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetCurrentAudioLanguage(IN const Json::Value& req, OUT Json::Value& response);
                void AampIsLive(IN const Json::Value& req, OUT Json::Value& response);
                void AampSeekLive(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetPlaybackDuration(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetRateAndSeek(IN const Json::Value& req, OUT Json::Value& response);
                void AampProxy(IN const Json::Value& req, OUT Json::Value& response);
                void AampPlayTillEof(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAudioBitrates(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetVideoBitrates(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAvailableAudioTracks(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAvailableTextTracks(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAudioVolume(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetAudioVolume(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetVideoZoom(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetVideoMute(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetPreferredLanguages(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetPreferredLanguages(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetVideoRectangle(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetVideoRectangle(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetVideoBitrate(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetVideoBitrate(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAudioBitrate(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetAudioBitrate(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetPlaybackRate(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetState(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetPlaybackPosition(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetCurrentDRM(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetPreferredDRM(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetPreferredDRM(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetAudioTrack(IN const Json::Value& req, OUT Json::Value& response);
                void AampGetAudioTrack(IN const Json::Value& req, OUT Json::Value& response);
                void AampEnableVideoRectangle(IN const Json::Value& req, OUT Json::Value& response);
                void AampSetWesterosSinkConfig(IN const Json::Value& req, OUT Json::Value& response);
		void AampCheckPlaybackRate(IN const Json::Value& req, OUT Json::Value& response);
                void AampPauseAtPosition(IN const Json::Value& req, OUT Json::Value& response);
		void AampSetLicenseServerURL(IN const Json::Value& req, OUT Json::Value& response);
                bool cleanup(const char*);
                std::string testmodulepre_requisites();
                bool testmodulepost_requisites();

};

#endif // __AAMP_STUB_H__
