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

#include "AampAgent.h"

/*Work around to take care of getting AAMP_EVENT_TUNED print in agentconsole log. This will be removed later.*/
bool AAMP_EVENT_TUNED_received=false;
bool speed_changed_received=false;
bool bitrate_changed_received=false;
bool cc_handle_received=false;
bool check_rate_value=false;
char errordescription[ERROR_DESCRIPTION_LENGTH]={0};
pthread_cond_t cond1;
pthread_mutex_t aamp_pthread_lock;

char cur_systime [80];
//gmainloop thread starter
static void* test_StreamThread(void *arg)
{
        if (AAMPGstPlayerMainLoop_test)
        {
                g_main_loop_run(AAMPGstPlayerMainLoop_test); // blocks
                DEBUG_PRINT(DEBUG_TRACE,"AAMPGstPlayer_StreamThread: exited main event loop\n");
        }
        g_main_loop_unref(AAMPGstPlayerMainLoop_test);
        AAMPGstPlayerMainLoop_test = NULL;
        return NULL;
}

//Event listener class
class testAAMPEventListener :public AAMPEventListener
{
public:
        void Event(const AAMPEvent & e)
       {
               switch (e.type)
               {
                case AAMP_EVENT_TUNED:
			/*Work around to take care of getting AAMP_EVENT_TUNED print in agentconsole log. This will be removed later.*/
			AAMP_EVENT_TUNED_received=true;
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_TUNED\n");
                        break;
                case AAMP_EVENT_TUNE_FAILED:
                        sleep(1);//To make sure the tune_api is executed before any failure
                        pthread_mutex_lock(&aamp_pthread_lock);
                        strncpy(errordescription,e.data.mediaError.description,ERROR_DESCRIPTION_LENGTH);
                        pthread_cond_signal(&cond1);
                        pthread_mutex_unlock(&aamp_pthread_lock);
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_TUNE_FAILED\n");
                        break;
                case AAMP_EVENT_STATE_CHANGED:
                        if(e.data.stateChanged.state == eSTATE_PLAYING)
                        {
                            pthread_mutex_lock(&aamp_pthread_lock);
                            pthread_cond_signal(&cond1);
                            pthread_mutex_unlock(&aamp_pthread_lock);
                            DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_STATE_CHANGED\n");
                        }
                        break;
                case AAMP_EVENT_SPEED_CHANGED:
			DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_SPEED_CHANGED current rate=%f\n", e.data.speedChanged.rate);
			speed_changed_received = true;
			if ( (int(e.data.speedChanged.rate) == 1) && (check_rate_value) )
			{
		           DEBUG_PRINT(DEBUG_TRACE,"Speed changed event has rate value as 1 , so setting speed_changed_received to false");
                           speed_changed_received = false;
			}
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_SPEED_CHANGED\n");
                        break;
                case AAMP_EVENT_EOS:
                        pthread_mutex_lock(&aamp_pthread_lock);
                        pthread_cond_signal(&cond1);
                        pthread_mutex_unlock(&aamp_pthread_lock);
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_EOS\n");
                        break;
                case AAMP_EVENT_PLAYLIST_INDEXED:
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_PLAYLIST_INDEXED\n");
                        break;
                case AAMP_EVENT_PROGRESS:
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_PROGRESS\n");
                        break;
                case AAMP_EVENT_CC_HANDLE_RECEIVED:
                        cc_handle_received=true;
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_CC_HANDLE_RECEIVED\n");
                        break;
                case AAMP_EVENT_BITRATE_CHANGED:
                        bitrate_changed_received=true;
                        DEBUG_PRINT(DEBUG_TRACE,"AAMP_EVENT_BITRATE_CHANGED\n");
                        break;
               }
       }
};

testAAMPEventListener *myEventListener = NULL ;


static void create_player_instance_regevents()
{
    time_t rawtime;
    tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(cur_systime,80,"%Y-%m-%d %H:%M:%S",timeinfo);
    gst_init(NULL,NULL);
    //starting gmain loop
    AAMPGstPlayerMainLoop_test = g_main_loop_new(NULL, FALSE);
    aampMainLoopThread_test = g_thread_new("AAMPGstPlayerLoop_test", &test_StreamThread, NULL );
    memset(errordescription,'\0',ERROR_DESCRIPTION_LENGTH);
    //starting event listener
    mSingleton= new PlayerInstanceAAMP();
    myEventListener = new testAAMPEventListener();
    mSingleton->RegisterEvents(myEventListener);
    //Set PreferredDRM as Widevine
    mSingleton->SetPreferredDRM(eDRM_WideVine);
}


static void delete_player_instance()
{
        if (mSingleton !=NULL && myEventListener !=NULL)
        {
                delete mSingleton;
                delete myEventListener;
                mSingleton=NULL;
                myEventListener=NULL;
                DEBUG_PRINT(DEBUG_TRACE,"Successfully deleted player instance\n");
        }
}

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/

std::string AampAgent::testmodulepre_requisites()
{

    DEBUG_PRINT(DEBUG_TRACE, "AAMP testmodule pre_requisites --> Entry\n");
#ifndef REF_PLATFORMS
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
            return "FAILURE";
    }
    DEBUG_PRINT(DEBUG_LOG,"\n Calling IARM_BUS_Connect\n");
    /*connecting application with IARM BUS*/
    retval=IARM_Bus_Connect();
    if(retval==0)
    {
            DEBUG_PRINT(DEBUG_LOG,"\n Application Successfully connected with IARMBUS \n");
    }
    else
    {
            DEBUG_PRINT(DEBUG_LOG,"\n Application Failed to connect with IARMBUS \n");
            return "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE,"\n managerInitialize ---->Entry\n");
    try
    {
            device::Manager::Initialize();
            DEBUG_PRINT(DEBUG_LOG,"\n DS manager successfully initialized \n");
    }
    catch(...)
    {
            DEBUG_PRINT(DEBUG_ERROR,"\n Exception Caught in DSmanagerinitialize\n");
            return "FAILURE";
    }
#endif
    create_player_instance_regevents();

    DEBUG_PRINT(DEBUG_TRACE, "AAMP testmodule pre_requisites -->Exit\n");
    return "SUCCESS";

}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Descrption    : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool AampAgent::testmodulepost_requisites()
{
        DEBUG_PRINT(DEBUG_TRACE, "AAMP testmodule post_requisites --> Entry\n");


  	//quit gmain loop
        g_main_loop_quit(AAMPGstPlayerMainLoop_test);
        g_thread_join(aampMainLoopThread_test);
#ifndef REF_PLATFORMS
        DEBUG_PRINT(DEBUG_LOG,"\n DeviceSettingsAgent shutting down \n");
        /* DS Manager De-initialize */
        DEBUG_PRINT(DEBUG_TRACE,"\n managerDeinitialize ---->Entry\n");
        try
        {
                device::Manager::DeInitialize();
                DEBUG_PRINT(DEBUG_LOG,"\n DS Manager Deinitialized \n");
        }
        catch(...)
        {
                DEBUG_PRINT(DEBUG_ERROR,"\n Exception Caught in DSmanagerDeinitialize\n");
            	return "FAILURE";
        }
        DEBUG_PRINT(DEBUG_TRACE,"\n managerDeinitialize ---->Exit\n");
        IARM_Result_t retval;
        retval=IARM_Bus_Disconnect();
        if(retval==0)
        {
                DEBUG_PRINT(DEBUG_LOG,"\n Application Disconnected from IARMBUS \n");
        }
        else
        {
                DEBUG_PRINT(DEBUG_ERROR,"\n Application failed to Disconnect from IARMBUS \n");
            	return "FAILURE";
        }
        retval=IARM_Bus_Term();
        if(retval==0)
        {
                DEBUG_PRINT(DEBUG_LOG,"\n IARMBUS terminated \n");
        }
        else
        {
                DEBUG_PRINT(DEBUG_ERROR,"\n IARMBUS failed to terminate \n");
            	return "FAILURE";
        }
#endif
        
	char line[128];
	string journalctl_log = "\n\t\t\t********************** TDK journalctl log : BEGIN **********************\t\t\t\n";
	string cmd = "journalctl -x --since \"";
	cmd += cur_systime;
	cmd += "\" -u tdk.service";
	DEBUG_PRINT(DEBUG_LOG, " Command executed: %s",cmd.c_str());
	FILE *pipe = popen(cmd.c_str(),"r");
	while(!feof(pipe))
	{
		if(fgets(line, 128, pipe)!=NULL)
                {
                  journalctl_log +=  line;
                }
	}
	pclose(pipe);
        journalctl_log += "\n\t\t\t********************** TDK journalctl log : END **********************\t\t\t\n" ;
	DEBUG_PRINT(DEBUG_LOG, "%s",journalctl_log.c_str());
        DEBUG_PRINT(DEBUG_TRACE, "AAMP testmodule post_requisites --> Exit\n");
        return "SUCCESS";

}

/**************************************************************************
Function Name   : CreateObject

Arguments       : NULL

Description     : This function is used to create a new object of the class "AampAgent".
**************************************************************************/

extern "C" AampAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new AampAgent(ptrtcpServer);
}
bool AampAgent::initialize(IN const char* szVersion)
{
        DEBUG_PRINT(DEBUG_TRACE, "AAMP Initialization Entry\n");
        cond1 = PTHREAD_COND_INITIALIZER;
        aamp_pthread_lock = PTHREAD_MUTEX_INITIALIZER;
	DEBUG_PRINT (DEBUG_TRACE, "AAMP Initialization Exit\n");
        return TEST_SUCCESS;
}

/*************************************************************************
Function Name   : AampTune

Arguments       : URL

Description     : This function is used to create start tuning

*************************************************************************/

void AampAgent::AampTune (IN const Json::Value& req, OUT Json::Value& response)

{
	
	DEBUG_PRINT (DEBUG_TRACE, "AampTune Entry \n");

	std::string Tune_URL = req["URL"].asCString();
	DEBUG_PRINT (DEBUG_TRACE, "Tuning URL for AAMP is: %s\n",Tune_URL.c_str());
        /* check if player instance is created. if not create an instance.
         * Player instance will be deleted in AampStop call incase tune error is observed so that new player instance should be created
         * before calling Tune. If there is no error observed in the previous tune call, then the same old player instance which got
         * created in pre-requisite is used
         */
	if (mSingleton == NULL)
	{
		DEBUG_PRINT (DEBUG_TRACE, "Creating new player instance before calling Tune API\n");
		create_player_instance_regevents();
	}
	//Tune call with proper URL
 	mSingleton->Tune(Tune_URL.c_str());
        pthread_mutex_lock(&aamp_pthread_lock);
        pthread_cond_wait(&cond1, &aamp_pthread_lock);
        pthread_mutex_unlock(&aamp_pthread_lock);

	/*Work around to take care of getting AAMP_EVENT_TUNED print in agentconsole log. This will be removed later.*/
	sleep(10);
	if (AAMP_EVENT_TUNED_received)
    {
        response["details"] = "AAMP TUNE call is success, received AAMP_EVENT_TUNED";
        if (cc_handle_received)
        {
            response["details"] = "AAMP TUNE call is success, received AAMP_EVENT_TUNED, AAMP_EVENT_CC_HANDLE_RECEIVED";
        }
    }
	else
    {
        response["details"] = "AAMP TUNE call is success";
    }

        if (strlen(errordescription)==0)
        {
                response["result"] = "SUCCESS";
                DEBUG_PRINT (DEBUG_TRACE, "AAMP TUNE call is success\n");
        }
        else
        {
                response["result"] = "FAILURE";
                response["details"] = errordescription;
                DEBUG_PRINT (DEBUG_TRACE, "Deleting player instance since current player instance is in ERROR state\n");
                delete_player_instance();
                DEBUG_PRINT (DEBUG_TRACE, "AAMP TUNE call is failed with error: %s\n",errordescription);
                memset(errordescription,'\0',ERROR_DESCRIPTION_LENGTH);
        }
	DEBUG_PRINT (DEBUG_TRACE, "AampTune Exit \n");
	return;

}
/*************************************************************************
Function Name   : AampStop

Description     : This function is used to stop Aamp tuning

*************************************************************************/

void AampAgent::AampStop (IN const Json::Value& req, OUT Json::Value& response)

{

        DEBUG_PRINT (DEBUG_TRACE, "AampStop Entry \n");

        //Stop tune
        mSingleton->Stop();
 	response["result"] = "SUCCESS";
        response["details"] = "AAMP Stop call is fine";
  	DEBUG_PRINT (DEBUG_TRACE, "AampStop Exit \n");
        return;

}

/*************************************************************************
Function Name   : AampPlayTillEof

Description     : This function is used to continue the playback either till failure or EOS is received

*************************************************************************/

void AampAgent::AampPlayTillEof(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampPlayTillEof Tune \n");
        pthread_mutex_lock(&aamp_pthread_lock);
        memset(errordescription,'\0',ERROR_DESCRIPTION_LENGTH);
        pthread_cond_wait(&cond1, &aamp_pthread_lock);
        pthread_mutex_unlock(&aamp_pthread_lock);
        response["result"] = "SUCCESS";
        response["details"] = errordescription;
        DEBUG_PRINT (DEBUG_TRACE, "AampPlayTillEof Exit \n");
}


/*************************************************************************
Function Name   : AampProxy

Arguments       : for setting the proxy,http_proxy and no_proxy env variable
               
Description     : This function is used to Set/Reset http proxy

*************************************************************************/
void AampAgent::AampProxy (IN const Json::Value& req, OUT Json::Value& response)

{
        DEBUG_PRINT (DEBUG_TRACE, "AampProxy  Entry \n");
        //std::string hp = req["http_proxy"].asCString();
        //std::string np = req["no_proxy"].asCString();
        float set = req["set"].asFloat();
        if(1.0 == set)
        {
           DEBUG_PRINT (DEBUG_TRACE, "Aamp Proxy Set \n");
           //Request for setting the proxy
           std::string hp = req["http_proxy"].asCString();
           std::string np = req["no_proxy"].asCString();
           std::string sp = req["https_proxy"].asCString();
           if(http_proxy != NULL && no_proxy != NULL && https_proxy != NULL)
           {
              delete http_proxy;
              delete no_proxy;
              delete https_proxy;
              http_proxy = NULL;
              no_proxy = NULL;
              https_proxy = NULL;
           }
           http_proxy = new std::string;
           no_proxy = new std::string;
           https_proxy = new std::string;
           *http_proxy = hp;
           *no_proxy = np;
           *https_proxy = sp;
           if( (!putenv(const_cast<char*>(http_proxy->c_str())))  &&  (!putenv(const_cast<char*>(no_proxy->c_str())))  && (!putenv(const_cast<char*>(https_proxy->c_str())))  )
           {
              response["result"] = "SUCCESS";
           }
           else
           {
              response["result"]= "FAILURE";
           }

         }
         else
        {
           DEBUG_PRINT (DEBUG_TRACE, "Aamp Proxy Reset \n");
           if(http_proxy != NULL && no_proxy != NULL && https_proxy != NULL)
           {
              delete http_proxy;
              delete no_proxy;
              delete https_proxy;
              response["result"] = "SUCCESS";
           }
           else
           {
              response["result"]= "FAILURE";
           }
           http_proxy = NULL;
           no_proxy = NULL;
           https_proxy = NULL;
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampProxy  Exit \n");
        return;
}


/*************************************************************************
Function Name   : AampSetRate

Arguments       : rate - Rate of playback
               
Description     : This function is used to Set playback rate

*************************************************************************/

void AampAgent::AampSetRate (IN const Json::Value& req, OUT Json::Value& response)

{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetRate Entry \n");
	check_rate_value = false;
        float rate = req["rate"].asFloat();
	char check_rate[10];
        strcpy(check_rate,req["check_rate_value"].asCString());

        if (!strcmp(check_rate,"true"))
            check_rate_value = true;

        DEBUG_PRINT (DEBUG_TRACE, "AampSetRate Speed is: %f\n",rate);
        //SetRate call with given rate
        mSingleton->SetRate(rate);
        /* Sleep for 5 seconds to get AAMP_EVENT_SPEED_CHANGED event*/
        sleep(5);
        if (speed_changed_received)
        {
            response["details"] = "AAMP Setrate call is fine, received AAMP_EVENT_SPEED_CHANGED";
        }
        else
        {
            response["details"] = "AAMP Setrate call is fine";
        }
        response["result"] = "SUCCESS";

        DEBUG_PRINT (DEBUG_TRACE, "AampSetRate Exit \n");
        return;

}

/*************************************************************************
Function Name   : AampIsLive

Arguments       : None

Description     : This function is used to check if the stream is live

*************************************************************************/
void AampAgent::AampIsLive (IN const Json::Value& req, OUT Json::Value& response)
{
        bool status = false;
        DEBUG_PRINT (DEBUG_TRACE, "AampIsLive Entry ------------> \n");

        status = mSingleton->IsLive();
        if (true == status)
        {
                DEBUG_PRINT(DEBUG_LOG,"Stream is Live\n");
                response["result"]="SUCCESS";
                response["details"] = "Stream is Live";
        }
        else
        {
                DEBUG_PRINT(DEBUG_LOG,"Stream is not Live\n");
                response["result"]="FAILURE";
                response["details"] = "Stream is not Live";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampIsLive Exit ------------> \n");
        return;
}

/*************************************************************************
Function Name   : AampSetLanguage

Arguments       : language - language name

Description     : This function is used to Set language

*************************************************************************/
void AampAgent::AampSetLanguage (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetLanguage Entry --------------------> \n");
        std::string language=req["language"].asCString();
        DEBUG_PRINT (DEBUG_TRACE, "Trying to set language : %s\n",language.c_str());
        mSingleton->SetLanguage(language.c_str());
        response["result"] = "SUCCESS";
        response["details"] = "AAMP SetLanguage call invoked";

        DEBUG_PRINT (DEBUG_TRACE, "AampSetLanguage Exit ---------------------> \n");
        return;
}

/*************************************************************************
Function Name   : AampGetCurrentAudioLanguage

Arguments       : None

Description     : This function is used to retrieve the current audio language

*************************************************************************/
void AampAgent::AampGetCurrentAudioLanguage(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetCurrentAudioLanguage Entry ------------> \n");

        std::string language = mSingleton->GetCurrentAudioLanguage();
        if(!language.empty())
        {
                DEBUG_PRINT(DEBUG_LOG,"Current audio language retrieved\n");
                response["result"] = "SUCCESS";
                response["details"] = language.c_str();
        }
        else
        {
                DEBUG_PRINT(DEBUG_LOG,"Current audio language not retrieved\n");
                response["result"] = "FAILURE";
                response["details"] = "Current audio language not retrieved\n";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetCurrentAudioLanguage Exit ------------> \n");
        return;
}

/*************************************************************************
Function Name   : AampSeekLive

Arguments       : None

Description     : This function is used to seek the stream to live

*************************************************************************/
void AampAgent::AampSeekLive (IN const Json::Value& req, OUT Json::Value& response)
{
        bool status = false;
        DEBUG_PRINT (DEBUG_TRACE, "AampSeekLive Entry ------------> \n");
        mSingleton->SeekToLive();
        status = mSingleton->IsLive();
        if (true == status)
        {
                DEBUG_PRINT(DEBUG_LOG,"Stream is Sought to Live\n");
                response["result"]="SUCCESS";
                response["details"] = "Stream is Sought to Live";
        }
        else
        {
                DEBUG_PRINT(DEBUG_LOG,"Stream is not Sought to Live\n");
                response["result"]="FAILURE";
                response["details"] = "Stream is not Sought to Live";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampSeekLive Exit ------------> \n");
        return;
}


/*************************************************************************
Function Name   : AampGetPlayBackDuration

Arguments       : None

Description     : This function is used to get playback duration

*************************************************************************/
void AampAgent::AampGetPlaybackDuration (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackDuration Entry ------------> \n");
        double duration= mSingleton->GetPlaybackDuration();
        DEBUG_PRINT(DEBUG_LOG,"Playback duration obtained from API = %f\n",duration);
	if (duration > 0)
        {
                DEBUG_PRINT(DEBUG_LOG,"Duration is greater than 0\n");
                response["result"]="SUCCESS";
		std::string result = "Duration of the given URL is ";
		result += std::to_string(duration);
                response["details"] = result;
        }
        else
        {
                DEBUG_PRINT(DEBUG_LOG,"Duration is not equal to 0\n");
                response["result"]="FAILURE";
        	std::string result = "Duration of the given URL is ";
                result += std::to_string(duration);
                response["details"] = result;
	}

        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackDuration Exit ------------> \n");
        return;
}


/*************************************************************************
Function Name   : AampSetRateAndSeek

Arguments       : rate - Rate of playback , seek seconds

Description     : This function is used to Set playback rate and seek

*************************************************************************/
void AampAgent::AampSetRateAndSeek (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetRateAndSeek Entry \n");
        float rate = req["rate"].asFloat();
        double seconds = req["seconds"].asDouble();
        DEBUG_PRINT (DEBUG_TRACE, "AampSetRateAndSeek Speed is: %f\n",rate);
        DEBUG_PRINT (DEBUG_TRACE, "AampSetRateAndSeek Seconds is: %f\n",seconds);
        //SetRate call with given rate
        mSingleton->SetRateAndSeek(rate,seconds);
	/* Sleep for 5 seconds to get AAMP_EVENT_BITRATE_CHANGED event*/
	sleep(5);
        if (bitrate_changed_received)
        {
            response["details"] = "AAMP SetRateAndSeek call is fine, received AAMP_EVENT_BITRATE_CHANGED";
        }
        else
        {
            response["details"] = "AAMP SetRateAndSeek call is fine";
        }
        response["result"] = "SUCCESS";
        DEBUG_PRINT (DEBUG_TRACE, "AampSetRateAndSeek Exit \n");
        return;
}

/*************************************************************************
Function Name   : AampPauseAtPosition
Arguments       : position - position to pause at
Description     : This function is used to pause the pipeline at
*************************************************************************/
void AampAgent::AampPauseAtPosition (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampPauseAtPosition Entry \n");
        double position = req["position"].asDouble();
        DEBUG_PRINT (DEBUG_TRACE, "AampPauseAtPosition Seconds is: %f\n",position);
        //PauseAt call with given position
        mSingleton->PauseAt(position);
        /* Sleep for 5 seconds to get AAMP_EVENT_SPEED_CHANGED event*/
        sleep(5);
        if (speed_changed_received)
        {
            response["details"] = "AAMP PauseAt call is fine, received AAMP_EVENT_SPEED_CHANGED";
        }
        else
        {
            response["details"] = "AAMP PauseAt call is fine";
        }
        response["result"] = "SUCCESS";
        DEBUG_PRINT (DEBUG_TRACE, "AampPauseAtPosition Exit \n");
        return;
}

/*************************************************************************
Function Name   : AampGetAudioBitrates

Arguments       : None

Description     : This function is used to get the available audio bitrates

*************************************************************************/
void AampAgent::AampGetAudioBitrates(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioBitrates  Entry \n");
    char details[500];
    std::vector<long> bitrates_list = mSingleton->GetAudioBitrates();
    if (!bitrates_list.empty()){
        std::ostringstream audio_bitrates;
        std::copy(bitrates_list.begin(), bitrates_list.end(), std::ostream_iterator<long>(audio_bitrates, " "));
        DEBUG_PRINT (DEBUG_TRACE, "No. of Audio bitrates supported: %d\n",bitrates_list.size());
        DEBUG_PRINT (DEBUG_TRACE, "Audio bitrates supported: %s\n",audio_bitrates.str().c_str());
        sprintf(details,"Audio bitrates supported: %s",audio_bitrates.str().c_str());
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAudioBitrates call is fine\n");
    }
    else{
        response["result"]="FAILURE";
        response["details"] = "Audio bitrates supported is empty";
        DEBUG_PRINT (DEBUG_TRACE, "Audio bitrates supported is empty\n");
        DEBUG_PRINT (DEBUG_TRACE, "GetAudioBitrates call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioBitrates  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampGetVideoBitrates

Arguments       : None

Description     : This function is used to get the available video bitrates

*************************************************************************/
void AampAgent::AampGetVideoBitrates(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoBitrates  Entry \n");
    char details[500];
    std::vector<long> bitrates_list = mSingleton->GetVideoBitrates();
    if (!bitrates_list.empty()){
        std::ostringstream video_bitrates;
        std::copy(bitrates_list.begin(), bitrates_list.end(), std::ostream_iterator<long>(video_bitrates, " "));
        DEBUG_PRINT (DEBUG_TRACE, "No. of Video bitrates supported: %d\n",bitrates_list.size());
        DEBUG_PRINT (DEBUG_TRACE, "Video bitrates supported: %s\n",video_bitrates.str().c_str());
        sprintf(details,"Video bitrates supported: %s",video_bitrates.str().c_str());
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetVideoBitrates call is fine\n");
    }
    else{
        response["result"]="FAILURE";
        response["details"] = "Video bitrates supported is empty";
        DEBUG_PRINT (DEBUG_TRACE, "Video bitrates supported is empty\n");
        DEBUG_PRINT (DEBUG_TRACE, "GetVideoBitrates call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoBitrates  Exit \n");
    return;
}

/*************************************************************************
Function Name   : GetLanguageCodes

Arguments       : None

Description     : This function is used to get the language codes

*************************************************************************/
std::string GetLanguageCodes(std::string track_list)
{
    DEBUG_PRINT (DEBUG_TRACE, "GetLanguageCodes  Entry \n");

    std::string lang_codes = "";

    Json::Reader reader;
    Json::Value root;

    bool parse_status = reader.parse(track_list, root);
    if (!parse_status)
    {
        DEBUG_PRINT (DEBUG_TRACE, "Error in parsing json track info\n");
    }
    else
    {
        for(int index = 0; index < root.size(); ++index)
        {
            const Json::Value track_info = root[index];
            const Json::Value code = track_info["language"];
            if (!lang_codes.empty())
                lang_codes += ",";
            lang_codes += code.asString();
        }
        DEBUG_PRINT (DEBUG_TRACE, "Language codes from json track info : %s\n",lang_codes.c_str());
    }

    DEBUG_PRINT (DEBUG_TRACE, "GetLanguageCodes  Exit \n");
    return lang_codes;
}

/*************************************************************************
Function Name   : AampGetAvailableAudioTracks

Arguments       : None

Description     : This function is used to get the available audio tracks

*************************************************************************/
void AampAgent::AampGetAvailableAudioTracks(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAvailableAudioTracks  Entry \n");
    char details[200];
    std::string audiotracks_list = mSingleton->GetAvailableAudioTracks();
    if (!audiotracks_list.empty()){
        DEBUG_PRINT (DEBUG_TRACE, "Avaliable Audio tracks: %s\n",audiotracks_list.c_str());
        std::string lang_codes = GetLanguageCodes(audiotracks_list);
        sprintf(details,"Avaliable Audio tracks: %s",lang_codes.c_str());
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAvailableAudioTracks call is fine\n");
    }
    else{
        response["result"]="FAILURE";
        response["details"] = "Available Audio tracks is empty";
        DEBUG_PRINT (DEBUG_TRACE, "Available Audio tracks is empty\n");
        DEBUG_PRINT (DEBUG_TRACE, "GetAvailableAudioTracks call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAvailableAudioTracks  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampGetAvailableTextTracks

Arguments       : None

Description     : This function is used to get the available text tracks

*************************************************************************/
void AampAgent::AampGetAvailableTextTracks(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAvailableTextTracks  Entry \n");
    char details[200];
    std::string texttracks_list = mSingleton->GetAvailableTextTracks();
    if (!texttracks_list.empty()){
        DEBUG_PRINT (DEBUG_TRACE, "Avaliable Text tracks: %s\n",texttracks_list.c_str());
        std::string lang_codes = GetLanguageCodes(texttracks_list);
        sprintf(details,"Avaliable Text tracks: %s",lang_codes.c_str());
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAvailableTextTracks call is fine\n");
    }
    else{
        response["result"]="FAILURE";
        response["details"] = "Available Text tracks is empty";
        DEBUG_PRINT (DEBUG_TRACE, "Available Text tracks is empty\n");
        DEBUG_PRINT (DEBUG_TRACE, "GetAvailableTextTracks call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAvailableTextTracks  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampGetAudioVolume

Arguments       : None

Description     : This function is used to get the current audio volume

*************************************************************************/
void AampAgent::AampGetAudioVolume(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioVolume  Entry \n");
    char details[200];
    int audio_volume = mSingleton->GetAudioVolume();
    DEBUG_PRINT (DEBUG_TRACE, "Current Audio Volume: %d\n",audio_volume);
    sprintf(details,"Current Audio Volume: %d",audio_volume);

    /*checking whether audio_volume is withing the expected range
    As per aamp src code, minimum audio volume is 0 and maximum
    value is 100 */
    if (audio_volume >= AUDIO_VOL_MIN && audio_volume <= AUDIO_VOL_MAX){
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAudioVolume call is fine\n");
    }
    else{
        response["result"] = "FAILURE";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAudioVolume call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioVolume Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampSetAudioVolume

Arguments       : None

Description     : This function is used to set volume level

*************************************************************************/
void AampAgent::AampSetAudioVolume(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioVolume  Entry \n");
    if(&req["volume"]==NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "No Input Volume level";
        return;
    }
    int volume = req["volume"].asInt();

    DEBUG_PRINT (DEBUG_TRACE, "Audio Volume level: %d",volume);
    mSingleton->SetAudioVolume(volume);
    response["result"] = "SUCCESS";
    response["details"] = "SetAudioVolume call is fine";
    DEBUG_PRINT (DEBUG_TRACE, "SetAudioVolume call is fine\n");
    DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioVolume Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampSetVideoZoom

Arguments       : None

Description     : This function is used to set video zoom level

*************************************************************************/
void AampAgent::AampSetVideoZoom(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoZoom  Entry \n");
    if(&req["zoom"]==NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "No Input Video Zoom level";
        return;
    }
    char zoom[10];
    strcpy(zoom,req["zoom"].asCString());

    VideoZoomMode zoom_mode;
    if (!strcmp(zoom,"FULL"))
        zoom_mode = VIDEO_ZOOM_FULL;
    else if (!strcmp(zoom,"NONE"))
        zoom_mode = VIDEO_ZOOM_NONE;
    else{
        response["result"] = "FAILURE";
        response["details"] = "Invalid Video Zoom level";
        return;
    }

    DEBUG_PRINT (DEBUG_TRACE, "Video Zoom level: %s",zoom);
    mSingleton->SetVideoZoom(zoom_mode);
    response["result"] = "SUCCESS";
    response["details"] = "SetVideoZoom call is fine";
    DEBUG_PRINT (DEBUG_TRACE, "SetVideoZoom call is fine\n");
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoZoom Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampSetVideoMute

Arguments       : None

Description     : This function is used to set video mute

*************************************************************************/
void AampAgent::AampSetVideoMute(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoMute  Entry \n");
    if(&req["mute"]==NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "No Input Video mute";
        return;
    }
    char mute[10];
    strcpy(mute,req["mute"].asCString());

    bool video_mute;
    if (!strcmp(mute,"ON"))
        video_mute = true;
    else if (!strcmp(mute,"OFF"))
        video_mute = false;
    else{
        response["result"] = "FAILURE";
        response["details"] = "Invalid Video mute";
        return;
    }
    DEBUG_PRINT (DEBUG_TRACE, "Video Mute Status: %s",mute);
    mSingleton->SetVideoMute(video_mute);
    response["result"] = "SUCCESS";
    response["details"] = "SetVideoMute call is fine";
    DEBUG_PRINT (DEBUG_TRACE, "SetVideoMute call is fine\n");
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoMute Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampGetPreferredLanguages

Arguments       : None

Description     : This function is used to get current preferred language list

*************************************************************************/
void AampAgent::AampGetPreferredLanguages(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetPreferredLanguages  Entry \n");
    char details[200];
    const char *languages = mSingleton->GetPreferredLanguages();
    if (languages != NULL){
        DEBUG_PRINT (DEBUG_TRACE, "Preferred Languages: %s\n",languages);
        sprintf(details,"Preferred Languages: %s",languages);
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetPreferredLanguages call is fine\n");
    }
    else{
        DEBUG_PRINT (DEBUG_TRACE, "Preferred Languages not set\n");
        response["result"] = "SUCCESS";
        response["details"] = "Preferred Languages not set";
        DEBUG_PRINT (DEBUG_TRACE, "GetPreferredLanguages call is fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetPreferredLanguages  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampSetPreferredLanguages

Arguments       : None

Description     : This function is used to set preferred language list

*************************************************************************/
void AampAgent::AampSetPreferredLanguages(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampSetPreferredLanguages  Entry \n");
    if(&req["languages"]==NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "No Input preferred languages";
        return;
    }
    const char *languages = (char*) req["languages"].asCString();

    DEBUG_PRINT (DEBUG_TRACE, "Set Preferred Languages: %s\n",languages);
    mSingleton->SetPreferredLanguages(languages);
    response["result"] = "SUCCESS";
    response["details"] = "SetPreferredLanguages call is fine";
    DEBUG_PRINT (DEBUG_TRACE, "SetPreferredLanguages call is fine\n");
    DEBUG_PRINT (DEBUG_TRACE, "AampGetPreferredLanguages  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampGetVideoRectangle

Arguments       : None

Description     : This function is used to get the current video co-ordinates in x,y,w,h format

*************************************************************************/
void AampAgent::AampGetVideoRectangle(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoRectangle  Entry \n");
    char details[200];
    std::string co_ordinates = mSingleton->GetVideoRectangle();
    if (!co_ordinates.empty()){
        DEBUG_PRINT (DEBUG_TRACE, "Current Video Co-ordinates in x,y,w,h format: %s\n",co_ordinates.c_str());
        sprintf(details,"Current Video Co-ordinates in x,y,w,h format : %s",co_ordinates.c_str());
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetVideoRectangle call is fine\n");
    }
    else{
        response["result"]="FAILURE";
        response["details"] = "Current Video Co-ordinates are empty";
        DEBUG_PRINT (DEBUG_TRACE, "Current Video Co-ordinates are empty\n");
        DEBUG_PRINT (DEBUG_TRACE, "GetVideoRectangle call is not fine\n");
    }
    DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoRectangle  Exit \n");
    return;
}

/*************************************************************************
Function Name   : AampSetVideoRectangle

Arguments       : None

Description     : This function is used to set video display rectangle co-ordinates

*************************************************************************/
void AampAgent::AampSetVideoRectangle(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoRectangle  Entry \n");
    if(&req["x"]==NULL || &req["y"]==NULL || &req["w"]==NULL || &req["h"]==NULL)
    {
        response["result"] = "FAILURE";
        response["details"] = "No Input Rectangle co-ordinates";
        return;
    }
    int x = req["x"].asInt();
    int y = req["y"].asInt();
    int w = req["w"].asInt();
    int h = req["h"].asInt();

    DEBUG_PRINT (DEBUG_TRACE, "Video rectangle co-ordinates in x,y,w,h format: %d,%d,%d,%d",x,y,w,h);
    mSingleton->SetVideoRectangle(x,y,w,h);
    response["result"] = "SUCCESS";
    response["details"] = "SetVideoRectangle call is fine";
    DEBUG_PRINT (DEBUG_TRACE, "SetVideoRectangle call is fine\n");
    DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoRectangle Exit \n");
    return;
}
/*************************************************************************
Function Name   : AampGetVideoBitrate

Description     : This function is used to get the video bitrate

*************************************************************************/
void AampAgent::AampGetVideoBitrate (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoBitrate Entry \n");
        char details[100];
        long bitRate = 0;
        bitRate = mSingleton->GetVideoBitrate();

        if (bitRate != 0)
        {
                DEBUG_PRINT (DEBUG_TRACE, "Bitrate: %ld\n",bitRate);
                sprintf(details, "BitRate: %ld", bitRate);
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "Bitrate value not retrieved");
                response["result"] = "FAILURE";
                response["details"] = "Bitrate value not retrieved";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetVideoBitrate Exit \n");
        return;
}
/*************************************************************************
Function Name   : AampSetVideoBitrate

Arguments       : bitrate - Video bitrate

Description     : This function is used to set video bitrate

*************************************************************************/
void AampAgent::AampSetVideoBitrate (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoBitrate Entry \n");
        if(&req["bitRate"]==NULL)
        {
                return;
        }
        long bitRate = (long) req["bitRate"].asInt();
        DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoBitrate Speed is: %ld\n",bitRate);
        mSingleton->SetVideoBitrate(bitRate);
        response["result"] = "SUCCESS";
        response["details"] = "AAMP SetVideoBitrate call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "AAMP SetVideoBitrate call is fine \n");

        DEBUG_PRINT (DEBUG_TRACE, "AampSetVideoBitrate Exit \n");
        return;
}
/*************************************************************************
Function Name   : AampGetAudioBitrate

Description     : This function is used to get the audio bitrate

*************************************************************************/
void AampAgent::AampGetAudioBitrate (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioBitrate Entry \n");
        char details[100];
        long bitRate = 0;
        bitRate = mSingleton->GetAudioBitrate();
        
        if (bitRate != 0)
        {
                DEBUG_PRINT (DEBUG_TRACE, "Bitrate: %ld\n",bitRate);
                sprintf(details, "BitRate: %ld", bitRate);
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "Bitrate value not retrieved");
                response["result"] = "FAILURE";
                response["details"] = "Bitrate value not retrieved";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioBitrate Exit \n");
        return;
}
/*************************************************************************
Function Name   : AampSetAudioBitrate

Arguments       : bitrate - Audio bitrate

Description     : This function is used to Set audio bitrate
*************************************************************************/
void AampAgent::AampSetAudioBitrate (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioBitrate Entry \n");
        if(&req["bitRate"]==NULL)
        {
                return;
        }
        long bitRate = (long) req["bitRate"].asInt();
        DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioBitrate Speed is: %ld\n",bitRate);
        mSingleton->SetAudioBitrate(bitRate);
        response["result"] = "SUCCESS";
        response["details"] = "AAMP SetAudioBitrate call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "AAMP SetAudioBitrate call is fine \n");

        DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioBitrate Exit \n");
        return;
}
/*************************************************************************
Function Name   : GetPlaybackRate

Description     : This function is used to get playback rate

*************************************************************************/
void AampAgent::AampGetPlaybackRate (IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackRate Entry \n");

        char details[100];
        int rate = mSingleton->GetPlaybackRate();
        std::string playRate = std::to_string(rate);
        
        if (!playRate.empty())
        {
                DEBUG_PRINT (DEBUG_TRACE, "Playback rate: %d\n",rate);
                sprintf(details, "PlaybackRate: %d", rate);
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "PlaybackRate invalid");
                response["result"] = "FAILURE";
                response["details"] = "PlaybackRate invalid";
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackRate Exit \n");
        return;
}
/*************************************************************************
Function Name   : GetState

Description     : This function is used to get aamp play state

*************************************************************************/
void AampAgent::AampGetState(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetState Entry \n");
        string details = "INVALID";
        PrivAAMPState state = mSingleton->GetState();
		switch (state)
        {
                case eSTATE_IDLE:
                        details = "IDLE"; 
                        break;
                case eSTATE_INITIALIZING:
                        details = "INITIALIZING"; 
                        break;
                case eSTATE_INITIALIZED:
                        details = "INITIALIZED"; 
                        break;
                case eSTATE_PREPARING:
                        details = "PREPARING"; 
                        break;
                case eSTATE_PREPARED:
                        details = "PREPARED"; 
                        break;
                case eSTATE_BUFFERING:
                        details = "BUFFERING"; 
                        break;
                case eSTATE_PAUSED:
                        details = "PAUSED"; 
                        break;
                case eSTATE_SEEKING:
                        details = "SEEKING"; 
                        break;
                case eSTATE_PLAYING:
                        details = "PLAYING"; 
                        break;
                case eSTATE_STOPPING:
                        details = "STOPPING"; 
                        break;
                case eSTATE_STOPPED:
                        details = "STOPPED"; 
                        break;
                case eSTATE_COMPLETE:
                        details = "COMPLETE"; 
                        break;
                case eSTATE_ERROR:
                        details = "ERROR"; 
                        break;
                case eSTATE_RELEASED:
                        details = "RELEASED"; 
                        break;
                default:
                        details = "INVALID"; 
        }
           
        if (details != "INVALID") 
        {
                DEBUG_PRINT (DEBUG_TRACE, "Current State: %s\n", details.c_str());
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "GetCurrentState call failed");
                response["result"] = "FAILURE";
                response["details"] = details;
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetState Exit \n");
        return;
}
/*************************************************************************
Function Name   : AampGetPlaybackPosition

Description     : This function is used to get the current playback position.
*************************************************************************/
void AampAgent::AampGetPlaybackPosition(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackPosition Entry \n");
        char details[100];
        double position = mSingleton->GetPlaybackPosition();
        
        if (position > 0)
        {
                DEBUG_PRINT (DEBUG_TRACE, "PlaybackPosition: %lf\n",position);
                sprintf(details, "Position: %lf", position);
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "PlaybackPosition not retrieved");
                response["result"] = "FAILURE";
                response["details"] = "PlaybackPosition not retrieved";
        }
        
        DEBUG_PRINT (DEBUG_TRACE, "AampGetPlaybackPosition Exit \n");
        return;
}
/*************************************************************************
Function Name   : AampGetCurrentDRM

Description     : This function is used to get the current DRM
*************************************************************************/
void AampAgent::AampGetCurrentDRM(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetCurrentDRM Entry \n");
        std::string drm = mSingleton->GetCurrentDRM();
        if (!drm.empty())
        {
                DEBUG_PRINT(DEBUG_LOG,"Current DRM retrieved %s\n", drm.c_str());
                response["result"] = "SUCCESS";
                response["details"] = drm.c_str();
        }
        else
        {
                DEBUG_PRINT(DEBUG_LOG,"Current DRM not retrieved\n");
                response["result"] = "FAILURE";
                response["details"] = "Current DRM not retrieved";
        }
        DEBUG_PRINT (DEBUG_TRACE, "AampGetCurrentDRM Exit\n");
        return;
}
/**************************************************************************
Function Name   : cleanup

Arguments       : NULL

Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool AampAgent::cleanup(IN const char* szVersion)
{
	DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Entry\n");
	return TEST_SUCCESS;
}

/*************************************************************************
Function Name   : AampSetPreferredDRM

Description     : This function is used to Set the Preferred DRM
*************************************************************************/
void AampAgent::AampSetPreferredDRM(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetPreferredDRM Entry \n");
       DRMSystems pref_Drm;
       if(&req["preferredDrm"] == NULL )
        {
           return;
        }
       char drm[20] = {'\0'};
       strcpy(drm,req["preferredDrm"].asCString());
        if(!strcmp(drm, "PlayReady"))
            pref_Drm = eDRM_PlayReady;
        else if(!strcmp(drm,"WideVine"))
            pref_Drm = eDRM_WideVine;
        else if(!strcmp(drm,"ClearKey"))
            pref_Drm = eDRM_ClearKey;
        else{
            response["result"] = "FAILURE";
            response["details"] = "Invalid Drm";
            return;
        }

        mSingleton->SetPreferredDRM(pref_Drm);
        response["result"] = "SUCCESS";
        response["details"] = "AAMP AampSetPreferredDRM call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "AAMP AampSetPreferredDRM call is fine \n");
        DEBUG_PRINT (DEBUG_TRACE, "AampSetPreferredDRM Exit \n");
        return;
}

/*************************************************************************
Function Name   : AampSetLicenseServerURL

Description     : This function is used to Set the License Server URL
*************************************************************************/
void AampAgent::AampSetLicenseServerURL(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetLicenseServerURL Entry \n");
        DRMSystems pref_Drm;
        if(&req["DRM"] == NULL )
        {
           return;
        }
        char drm[20] = {'\0'};
        strcpy(drm,req["DRM"].asCString());
        if(!strcmp(drm, "PlayReady"))
            pref_Drm = eDRM_PlayReady;
        else if(!strcmp(drm,"WideVine"))
            pref_Drm = eDRM_WideVine;
        else if(!strcmp(drm,"ClearKey"))
            pref_Drm = eDRM_ClearKey;
        else{
            response["result"] = "FAILURE";
            response["details"] = "Invalid Drm";
            return;
        }

	const char *license_server_url = (char*) req["license_server_url"].asCString();
        mSingleton->SetLicenseServerURL(license_server_url,pref_Drm);
        response["result"] = "SUCCESS";
        response["details"] = "AAMP AampSetLicenseServerURL call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "AAMP SetLicenseServerURL call is fine \n");
        DEBUG_PRINT (DEBUG_TRACE, "AampSetLicenseServerURL Exit \n");
        return;
}

/***********************************************************************************************************
Function Name   : AampEnableVideoRectangle

Description     : This function is used to control the setting of SetVideoRectangle property of westerossink
************************************************************************************************************/
void AampAgent::AampEnableVideoRectangle(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampEnableVideoRectangle Entry \n");
	char enable[10];
        strcpy(enable,req["enable"].asCString());
	bool set_rectangle;
        if (!strcmp(enable,"true"))
            set_rectangle = true;
        else if (!strcmp(enable,"false"))
            set_rectangle = false;
        else{
            response["result"] = "FAILURE";
            response["details"] = "Invalid Enable param";
            return;
        }
	DEBUG_PRINT (DEBUG_TRACE, "Enable Video Rectangle : %s",enable);
	mSingleton->EnableVideoRectangle(set_rectangle);
	response["result"] = "SUCCESS";
        response["details"] = "EnableVideoRectangle call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "EnableVideoRectangle call is fine\n");
        DEBUG_PRINT (DEBUG_TRACE, "AampEnableVideoRectangle Exit \n");
        return;
}

/***********************************************************************************************************
Function Name   : AampSetWesterosSinkConfig

Description     : This function is used to enable aamp to use westerossink as video-sink
************************************************************************************************************/
void AampAgent::AampSetWesterosSinkConfig(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampSetWesterosSinkConfig Entry \n");
        char enable[10];
        strcpy(enable,req["enable"].asCString());
        bool set_westerossink;
        if (!strcmp(enable,"true"))
            set_westerossink = true;
        else if (!strcmp(enable,"false"))
            set_westerossink = false;
        else{
            response["result"] = "FAILURE";
            response["details"] = "Invalid Enable param";
            return;
        }
        DEBUG_PRINT (DEBUG_TRACE, "SetWesterosSinkConfig : %s",enable);
        mSingleton->SetWesterosSinkConfig(set_westerossink);
        response["result"] = "SUCCESS";
        response["details"] = "SetWesterosSinkConfig call is fine";
        DEBUG_PRINT (DEBUG_TRACE, "SetWesterosSinkConfig call is fine\n");
        DEBUG_PRINT (DEBUG_TRACE, "AampSetWesterosSinkConfig Exit \n");
        return;
}

/***********************************************************************************************************
Function Name   : AampSetAudioTrack

Description     : This function is used to set the desired audio track
************************************************************************************************************/
void AampAgent::AampSetAudioTrack(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioTrack Entry \n");
	int track_index = req["track_index"].asInt();
	DEBUG_PRINT (DEBUG_TRACE, "Trying to set audio track index as %d",track_index);
	mSingleton->SetAudioTrack(track_index);
	response["result"] = "SUCCESS";
	response["details"] = "SetAudioTrack call is fine";
	DEBUG_PRINT (DEBUG_TRACE, "SetAudioTrack call is fine\n");
	DEBUG_PRINT (DEBUG_TRACE, "AampSetAudioTrack Exit \n");
        return;
}

/***********************************************************************************************************
Function Name   : AampGetAudioTrack

Description     : This function is used to get the current audio track
************************************************************************************************************/
void AampAgent::AampGetAudioTrack(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioTrack Entry \n");
	char details[30];
        int track_index = 999;
        track_index = mSingleton->GetAudioTrack();
	sprintf(details, "TrackIndex:%d",track_index);
	DEBUG_PRINT (DEBUG_TRACE, "Got Track Index as %d",track_index);
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "GetAudioTrack call is fine\n");
        DEBUG_PRINT (DEBUG_TRACE, "AampGetAudioTrack Exit \n");
        return;
}

/*************************************************************************************************
Function Name   : AampCheckPlaybackRate

Description     : This function is used to calculate the playback rate using GetPlaybackPosition()
**************************************************************************************************/
void AampAgent::AampCheckPlaybackRate(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampCheckPlaybackRate Entry \n");
        char details[100];
        double position, previous_position, play_jump, average_play_rate, play_jump_cumulative = 0;
        int i;
        for (i =0 ; i < 5; i++)
        {
             position = mSingleton->GetPlaybackPosition();
             if (position < 0)
             {
                DEBUG_PRINT (DEBUG_TRACE, "PlaybackPosition not retrieved");
                DEBUG_PRINT (DEBUG_TRACE, "AampCheckPlaybackRate Exit \n");
                response["result"] = "FAILURE";
                response["details"] = "PlaybackPosition not retrieved";
                return;
             }
             DEBUG_PRINT (DEBUG_TRACE, "PlaybackPosition: %lf\n",position);
             if ( i!=0 )
             {
                play_jump =  position - previous_position;
                DEBUG_PRINT (DEBUG_TRACE, "Play jump %lf\n",play_jump);
                play_jump_cumulative += play_jump;
             }
             previous_position = position;
             sleep(1);
        }
        average_play_rate = play_jump_cumulative/(i-1);
        DEBUG_PRINT (DEBUG_TRACE, "Average Playback Rate obtained as %lf\n",average_play_rate);
        sprintf(details, "Average Playback Rate obtained as %lf",average_play_rate);
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT (DEBUG_TRACE, "AampCheckPlaybackRate Exit \n");
        return;
}

/*************************************************************************
Function Name   : AampGetPreferredDRM

Description     : This function is used to get the current PreferredDRM
*************************************************************************/
void AampAgent::AampGetPreferredDRM(IN const Json::Value& req, OUT Json::Value& response)
{
        DEBUG_PRINT (DEBUG_TRACE, "AampGetPreferredDRM Entry \n");
       DRMSystems pref_Drm;
       string details = "INVALID_DRM";
        pref_Drm = mSingleton->GetPreferredDRM();
        switch (pref_Drm)
        {
            case eDRM_PlayReady:
                details = "PlayReady";
                break;
            case eDRM_WideVine:
                details = "WideWine";
                break;
            case eDRM_ClearKey:
                details = "ClearKey";
                break;
            default:
                details = "INVALID_DRM";
               }
        if (details != "INVALID_DRM")
        {
                DEBUG_PRINT (DEBUG_TRACE, "Current PreferredDRM: %s\n", details.c_str());
                response["result"] = "SUCCESS";
                response["details"] = details;
        }
        else
        {
                DEBUG_PRINT (DEBUG_TRACE, "Current PreferredDRM call failed");
                response["result"] = "FAILURE";
                response["details"] = details;
        }

        DEBUG_PRINT (DEBUG_TRACE, "AampGetPreferredDRM Exit\n");
        return;
}


/**************************************************************************
Function Name : DestroyObject

Arguments     : Input argument is BluetoothAgent Object

Description   : This function will be used to destory the BluetoothAgent object.
**************************************************************************/
extern "C" void DestroyObject(AampAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying AAMP Agent object\n");
        pthread_mutex_destroy(&aamp_pthread_lock);
        pthread_cond_destroy(&cond1);
        if(http_proxy != NULL && no_proxy != NULL && https_proxy != NULL)
        {
            delete http_proxy;
            delete no_proxy;
            delete https_proxy;
            http_proxy = NULL;
            no_proxy = NULL;
            https_proxy = NULL;
        }
	
	if (mSingleton !=NULL && myEventListener !=NULL)
        {
	    delete myEventListener;
            delete mSingleton;
	}
        delete stubobj;
}

