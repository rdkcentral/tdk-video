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

#include "MediaUtilsAgent.h"

typedef struct appDataStruct{
    RMF_AudioCaptureHandle hAudCap;
    FILE * file;
    unsigned long bytesWritten;
    unsigned samplerate;
    unsigned channels;
    unsigned bitsPerSample;
    unsigned int prevCbTime;
    unsigned int firstCbTime;
} appDataStruct;
appDataStruct appData;

/*************************************************************************************
 *Function name : MediaUtils_Return_Status_toString
 *Description   : This function is to check the ERROR return code of MediaUtils HAL API
 *
 *************************************************************************************/
std::string MediaUtils_Return_Status_toString(rmf_Error  status) 
{
    switch (status) 
	{
	    case 1/*RMF_ERROR*/: return " Error : RMF_ERROR";
	    case 2/*RMF_INVALID_PARM*/: return " Error : RMF_INVALID_PARM";
	    case 3/*RMF_INVALID_HANDLE*/: return " Error : RMF_INVALID_HANDLE";
	    case 4/*RMF_NOT_INITIALIZED*/: return " Error : RMF_NOT_INITIALIZED";
	    case 5/*RMF_INVALID_STATE*/: return " Error : RMF_INVALID_STATE";
	    default: return " Error : Unknown rmf_Error ";
    }
}

/****Helpers****/
/**Function to open a wave format file in to which the captured audio need to be saved**/
int fileOpen(appDataStruct *appData)
{
    char wav_file[200] = {'\0'};
    std::string g_tdkPath = getenv("TDK_PATH");
    sprintf (wav_file, "%s/tmp/audioCapture.wav",g_tdkPath.c_str());


    appData->file = 0;
    appData->file = fopen(wav_file, "wb+");

    if (!appData->file)
    {
        fprintf(stderr, "### unable to open the file /opt/TDK/tmp/audioCapture.wav");
        return 0;
    }
    else
    {
        printf("Capturing Audio to /opt/TDK/tmp/audioCapture.wav\n");
        return 1;
    }
}


static int writeDefaultWavHdr(appDataStruct *appData)
{
    //WAV header content
    unsigned char hdrdata[44] = {
	0x52,0x49,0x46,0x46,  // "RIFF" chunk identifier
	0x04,0xEE,0x02,0x00,  // chunk size
	0x57,0x41,0x56,0x45, // "WAVE" format
	0x66,0x6D,0x74,0x20, // "fmt "
	0x10,0x00,0x00,0x00,  // subchunk1 size (16 - PCM)
	0x01,0x00,                 // PCM audio format
	0x02,0x00,                 // num channels - stereo
	0x80,0xBB,0x00,0x00,  // sample rate - 48000 Hz
	0x00,0xEE,0x02,0x00,    // Byte rate
	0x04,0x00,            // Block Align
	0x10,0x00,            // Bits per sample - 16bits   
	0x64,0x61,0x74,0x61,  // "data"
	0x00,0xE4,0x57,0x00
    };

    fwrite(hdrdata, sizeof(unsigned char), 44, appData->file);

    return 0;
}

/******************************************************/
rmf_Error cbBufferReady (void *context, void *buf, unsigned int size)
{
    struct timeval tv;
    unsigned long int currTime = 0;

    appDataStruct *data = (appDataStruct*) context;

    gettimeofday(&tv,NULL);
    currTime = (1000 * tv.tv_sec) + tv.tv_usec/1000;

    fwrite(buf, sizeof(char), size, data->file);
    data->bytesWritten += size;

    printf("audioCaptureCb sz=%04d total=%08lu ", size, data->bytesWritten);

    if (data->prevCbTime) {
//        unsigned long int endTime;
//        gettimeofday(&tv,NULL);
//        endTime = (1000 * tv.tv_sec) + tv.tv_usec/1000;
        printf(" %03ums ", (unsigned int)(currTime - data->prevCbTime));
//        printf("execute=%02ums ", (unsigned int)(endTime - currTime));
        if (currTime>data->firstCbTime){
            printf("avg=%03ukhz", (unsigned int)((data->bytesWritten/4)/(currTime - data->firstCbTime)));
        }
    }
    else{
        data->firstCbTime = currTime;
    }

    data->prevCbTime = currTime;
    printf("\n");
    fflush(stdout);
    return RMF_SUCCESS;
}

rmf_Error cbStatusChange (void *context)
{
	printf("cbStatusChange callback getting invoked");
	return RMF_SUCCESS;
}

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *                1.
 *****************************************************************************/

std::string MediaUtilsAgent::testmodulepre_requisites()
{
	char cmdstring[200] = {'\0'};
	system("systemctl stop audiocapturemgr.service");
        std::string g_tdkPath = getenv("TDK_PATH"); 
        sprintf (cmdstring, "mkdir %s/tmp",g_tdkPath.c_str());
	system(cmdstring);
        sprintf (cmdstring, "touch %s/tmp/audioCapture.wav",g_tdkPath.c_str());
	system(cmdstring);

	memset(&appData, 0, sizeof(appData));
        appData.bitsPerSample = 16;
        appData.samplerate = 48000;
        appData.channels = 10;
//	return "SUCCESS";
        if(!fileOpen(&appData))
            {
            printf("Could not open the .wav file\n");
            return "FAILURE";
            }
        else
            {
            printf(".wave file opened\n");
	    //return "SUCCESS";
	    }

            if(!writeDefaultWavHdr(&appData))
                {
                printf("Wave header written\n");
                return "SUCCESS";
                }
            else
                {
                printf("Wave header not written\n");
                return "FAILURE";
                }
}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Descrption    : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool MediaUtilsAgent::testmodulepost_requisites()
{
	return "SUCCESS";
}

/**************************************************************************
Function Name   : CreateObject

Arguments       : NULL

Description     : This function is used to create a new object of the class "MediaUtilsAgent".
**************************************************************************/

extern "C" MediaUtilsAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new MediaUtilsAgent(ptrtcpServer);
}

/***************************************************************************
 *Function name : initialize
 *Descrption    : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/
bool MediaUtilsAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "MediaUtils Initialization Entry\n");
    DEBUG_PRINT (DEBUG_TRACE, "MediaUtils Initialization Exit\n");

    return TEST_SUCCESS;
}

/***************************************************************************
 *Function name : MediaUtils_AudioCapture_Open
 *Descrption    : This function is to get the default settings of audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_AudioCapture_Open(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Open --->Entry\n");

    audiocapture_Open=RMF_AudioCapture_Open(&appData.hAudCap);
    if (!audiocapture_Open)
    {
		response["result"] = "SUCCESS";
		response["details"] = "RMF_AudioCapture_Open success\n";
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_AudioCapture_Open success\n");
		DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Open -->Exit\n");
        return;
    }
    else
    {
		response["result"] = "FAILURE";
		response["details"] = "RMF_AudioCapture_Open failed" + MediaUtils_Return_Status_toString(audiocapture_Open);
        DEBUG_PRINT(DEBUG_ERROR, "RMF_AudioCapture_Open failed %s\n", MediaUtils_Return_Status_toString(audiocapture_Open));
		DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Open -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : MediaUtils_Get_DefaultSettings
 *Descrption    : This function is to get the default settings of audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_Get_DefaultSettings(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_Get_DefaultSettings --->Entry\n");
    int returnStatus = 0;
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	
	if(Isnullparamcheck)
	{
		returnStatus = RMF_AudioCapture_GetDefaultSettings(NULL);
	}
	else
	{
		returnStatus = RMF_AudioCapture_GetDefaultSettings(&settings);
	}
	
    std::string outputDetails;
    outputDetails += "   fifoSize = " + std::to_string(settings.fifoSize);
    outputDetails += "   threshold =" + std::to_string(settings.threshold);
    outputDetails += "   format =" + std::to_string(settings.format);
    outputDetails += "   samplingFreq = " + std::to_string(settings.samplingFreq);
    outputDetails += "   delayCompensation_ms = "+ std::to_string(settings.delayCompensation_ms);
    cout << "\noutputDetails:" << outputDetails << endl;

    if(!returnStatus)
    {
        response["result"] = "SUCCESS";
        response["details"] = outputDetails;
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_GetDefaultSettings call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetDefaultSettings -->Exit\n");
    }
    else
    {
        response["result"] = "FAILURE";
		response["details"] = "MediaUtils_GetDefaultSettings call is failed" + MediaUtils_Return_Status_toString(returnStatus);
        DEBUG_PRINT(DEBUG_ERROR, "MediaUtils_GetDefaultSettings call is FAILURE %s\n", MediaUtils_Return_Status_toString(returnStatus));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetDefaultSettings -->Exit\n");
    }
	return;
}

/***************************************************************************
 *Function name : MediaUtils_GetCurrentSettings
 *Descrption    : This function is to get the current settings of audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_Get_Current_Settings(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetCurrentSettings --->Entry\n");
    int returnStatus = 0;
    paramHandle = req["paramHandle"].asCString();
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	
    if(paramHandle == "NULL")
	{
        handle = 0;
        returnStatus = RMF_AudioCapture_GetCurrentSettings(handle,&settings);
	}
    else if(paramHandle == "VALID")
	{
		if (Isnullparamcheck)
		{
			returnStatus = RMF_AudioCapture_GetCurrentSettings(appData.hAudCap,NULL);
		}
		else
		{
			returnStatus = RMF_AudioCapture_GetCurrentSettings(appData.hAudCap,&settings);
		}
	}
    else
	{
		return;
	}

    std::string outputDetails;
    outputDetails += "   fifoSize = " + std::to_string(settings.fifoSize);
    outputDetails += "   threshold =" + std::to_string(settings.threshold);
    outputDetails += "   format =" + std::to_string(settings.format);
    outputDetails += "   samplingFreq = " + std::to_string(settings.samplingFreq);
    outputDetails += "   delayCompensation_ms = "+ std::to_string(settings.delayCompensation_ms);
    cout << "\noutputDetails:" << outputDetails << endl;

    if(!returnStatus)
    {
        response["result"] = "SUCCESS";
        response["details"] = outputDetails;
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_GetCurrentSettings call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetCurrentSettings -->Exit\n");
        return;
    }
    else
    {
        response["result"] = "FAILURE";
		response["details"] = "MediaUtils_GetCurrentSettings call is failed" + MediaUtils_Return_Status_toString(returnStatus);
        DEBUG_PRINT(DEBUG_ERROR, "MediaUtils_GetCurrentSettings call is FAILURE %s\n", MediaUtils_Return_Status_toString(returnStatus));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetCurrentSettings -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : MediaUtils_GetStatus
 *Descrption    : This function is to get the status of audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_Get_Status(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetStatus --->Entry\n");
    int returnStatus = 0, paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)appData.hAudCap;
	}
	
	if (Isnullparamcheck)
	{
		returnStatus = RMF_AudioCapture_GetStatus((RMF_AudioCaptureHandle)paramhandle,NULL);
	}
	else
	{
		returnStatus = RMF_AudioCapture_GetStatus((RMF_AudioCaptureHandle)paramhandle,&status);
	}

    std::string outputDetails;
    outputDetails += "   capture started  is = " + std::to_string(status.started);
    outputDetails += "   Current capture format is =" + std::to_string(status.format);
    outputDetails += "   Current capture sample rate is =" + std::to_string(status.samplingFreq);
    outputDetails += "   number of bytes in local fifo is = " + std::to_string(status.fifoDepth);
    outputDetails += "   count of overflows is = "+ std::to_string(status.overflows);
    outputDetails += "   count of underflows is = "+ std::to_string(status.underflows);
    outputDetails += "   captured audio mute state = "+ std::to_string(status.muted);
    outputDetails += "   capture paused  is = "+ std::to_string(status.paused);
    outputDetails += "   Current capture volume is = "+ std::to_string(status.volume);
    cout << "\noutputDetails:" << outputDetails << endl;
	
    if(!returnStatus)
    {
        response["result"] = "SUCCESS";
        response["details"] = outputDetails;
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_GetStatus call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetStatus -->Exit\n");
    }
    else
    {
        response["result"] = "FAILURE";
		response["details"] = "MediaUtils_GetStatus call is failed" + MediaUtils_Return_Status_toString(returnStatus);
        DEBUG_PRINT(DEBUG_ERROR, "MediaUtils_GetStatus call is FAILURE %s\n", MediaUtils_Return_Status_toString(returnStatus));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_GetStatus -->Exit\n");
    }
	return;
}


/***************************************************************************
 *Function name : MediaUtils_AudioCaptureStart
 *Descrption    : This function is to start the audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_AudioCaptureStart(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStart --->Entry\n");

    /*For cbBufferReady*/
    appData.prevCbTime = 0;
    settings.cbBufferReadyParm  = &appData;
    settings.samplingFreq = racFreq_e48000;
    
    int returnStatus;

    string paramHandle, paramBufferReady,paramFifosize;
    paramBufferReady = req["paramBufferReady"].asCString();
    paramHandle = req["paramHandle"].asCString();
    paramFifosize = req["paramFifosize"].asCString();
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	
    if(paramBufferReady == "NOTREADY")
    {
		settings.cbBufferReady =0;
	}
    else if(paramBufferReady == "READY")
	{
		settings.cbBufferReady = cbBufferReady;
		settings.cbStatusChange = cbStatusChange;
	}
    else
	{
		return;
	}

    if(paramFifosize == "NULL")
    {
		settings.fifoSize = 0;
		settings.threshold = 0;
    }
    else if(paramFifosize == "VALID")
    {
        /* 8KB PCM data buffer */
        settings.threshold          =  8 * 1024;
        settings.fifoSize           =  64 * 1024;
    }
    else
    {
        return;
    }
	
    if(paramHandle == "NULL")
	{
		handle = 0;
		returnStatus = RMF_AudioCapture_Start(handle,&settings);
	}
    else if(paramHandle == "VALID")
	{
		if (Isnullparamcheck)
		{
			returnStatus = RMF_AudioCapture_Start(appData.hAudCap,NULL);
		}
		else
		{
			returnStatus = RMF_AudioCapture_Start(appData.hAudCap,&settings);
		}
	}
    else
	{
		return;
	}

    if(!returnStatus)
    {
        response["result"] = "SUCCESS";
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_AudioCaptureStart call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStart -->Exit\n");
        return;
    }
    else
    {
        response["result"] = "FAILURE";
		response["details"] = "MediaUtils_AudioCaptureStart call is failed" + MediaUtils_Return_Status_toString(returnStatus);
        DEBUG_PRINT(DEBUG_ERROR, "MediaUtils_AudioCaptureStart call is FAILURE %s\n", MediaUtils_Return_Status_toString(returnStatus));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStart -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : MediaUtils_AudioCaptureStop
 *Descrption    : This function is to stop the audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_AudioCaptureStop(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStop --->Entry\n");
    int returnStatus;
    paramHandle = req["paramHandle"].asCString();
    if(paramHandle == "NULL")
	{
        handle =0;
        returnStatus = RMF_AudioCapture_Stop(handle);
	}
    else if(paramHandle == "VALID")
	{
	
        returnStatus = RMF_AudioCapture_Stop(appData.hAudCap);
	}

    if(!returnStatus)
    {
        response["result"] = "SUCCESS";
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_AudioCaptureStop call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStop -->Exit\n");
        return;
    }
    else
    {
        response["result"] = "FAILURE";
		response["details"] = "MediaUtils_AudioCaptureStop call is failed" + MediaUtils_Return_Status_toString(returnStatus);
        DEBUG_PRINT(DEBUG_ERROR, "MediaUtils_AudioCaptureStop call is FAILURE %s\n", MediaUtils_Return_Status_toString(returnStatus));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCaptureStop -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : MediaUtils_AudioCapture_Close
 *Descrption    : This function is to get the default settings of audio capture
 *****************************************************************************/
void MediaUtilsAgent::MediaUtils_AudioCapture_Close(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Close --->Entry\n");
    paramHandle = req["paramHandle"].asCString();
    if(paramHandle == "NULL")
	{
	handle = 0;
        audiocapture_Close=RMF_AudioCapture_Close(handle);
	}
    else if(paramHandle == "VALID")
	{
        audiocapture_Close=RMF_AudioCapture_Close(appData.hAudCap);
	}
    printf("audiocapture_Close: %d \n",audiocapture_Close);
    if (!audiocapture_Close)
    {
        response["result"] = "SUCCESS";
        response["details"] = "RMF_AudioCapture_Close success\n";
        DEBUG_PRINT(DEBUG_LOG, "MediaUtils_AudioCapture_Close success\n");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Close -->Exit\n");
        return;
    }
    else
    {
        response["result"] = "FAILURE";
        response["details"] = "RMF_AudioCapture_Close failed" + MediaUtils_Return_Status_toString(audiocapture_Close);
        DEBUG_PRINT(DEBUG_ERROR, "RMF_AudioCapture_Close failed %s\n", MediaUtils_Return_Status_toString(audiocapture_Close));
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_AudioCapture_Close -->Exit\n");
        return;
    }

}
/***************************************************************************
 * Function name : MediaUtilsAgent::MediaUtils_ExecuteCmd()
 *
 * Arguments     : Input arguments are command to execute in box
 *
 * Description   : This will execute linux commands in box
 * ***************************************************************************/
void MediaUtilsAgent::MediaUtils_ExecuteCmd(IN const Json::Value& request, OUT Json::Value& response)
{
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_ExecuteCmd ---> Entry\n");
        string fileinfo = request["command"].asCString();
        FILE *fp = NULL;
        char readRespBuff[BUFF_LENGTH];

        /*Frame the command  */
        string path = "";
        path.append(fileinfo);

        DEBUG_PRINT(DEBUG_TRACE, "Command Request Framed: %s\n",path.c_str());

        fp = popen(path.c_str(),"r");

        /*Check for popen failure*/
        if(fp == NULL)
        {
                response["result"] = "FAILURE";
                response["details"] = "popen() failure";
                DEBUG_PRINT(DEBUG_ERROR, "popen() failure\n");

                return;
        }

        /*copy the response to a buffer */
        while(fgets(readRespBuff,sizeof(readRespBuff),fp) != NULL)
        {
		DEBUG_PRINT(DEBUG_TRACE, "Command Response:\n");
		cout<<"readRespBuff:"<<readRespBuff<<endl;
        }

        pclose(fp);

	string respResult(readRespBuff);
        DEBUG_PRINT(DEBUG_TRACE, "\n\nResponse: %s\n",respResult.c_str());
        response["result"] = "SUCCESS";
        response["details"] = respResult;
        DEBUG_PRINT(DEBUG_LOG, "Execution success\n");
        DEBUG_PRINT(DEBUG_TRACE, "MediaUtils_ExecuteCmd -->Exit\n");
        return;
}

/**************************************************************************
Function Name   : cleanup

Arguments       : NULL

Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool MediaUtilsAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name : DestroyObject

Arguments     : Input argument is MediaUtilsAgent Object

Description   : This function will be used to destory the MediaUtilsAgent object.
**************************************************************************/
extern "C" void DestroyObject(MediaUtilsAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying MediaUtils Agent object\n");
        delete stubobj;
}

