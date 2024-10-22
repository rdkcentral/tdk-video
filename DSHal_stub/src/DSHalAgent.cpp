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

#include "DSHalAgent.h"

/*Not available the below MS12 capability lists(from Volumeleveller to LEConfig) in the latest HAL header/interface file. 
https://github.com/rdkcentral/rdk-halif-device_settings/blob/3.0.0/include/dsAVDTypes.h

so, currently defining here based on the reference from VTS to unblock the script developments. 
Need to remove this in future once its defined/added properly in the HAL header file. 
*/
typedef enum _dsMS12Capabilities {
	MS12SUPPORT_Volumeleveller             = 0x08,  ///< MS12 Volume leveller
	MS12SUPPORT_BassEnhancer               = 0x10,  ///< MS12 Bass Enhancer
	MS12SUPPORT_SurroundDecoder            = 0x20,  ///< MS12 Surround Decoder
	MS12SUPPORT_DRCMode                    = 0x40,  ///< MS12 DRC Mode
	MS12SUPPORT_SurroundVirtualizer        = 0x80,  ///< MS12 Surround Virtualizer
	MS12SUPPORT_MISteering                 = 0x100, ///< MS12 MI Steering
	MS12SUPPORT_GraphicEqualizer           = 0x200, ///< MS12 Graphic equalizer
	MS12SUPPORT_LEConfig                   = 0x400, ///< MS12 LE config
	MS12SUPPORT_Invalid		       = 0x800  ///< Invalid / Out of range
} dsMS12Capabilities;

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/
std::string DSHalAgent::testmodulepre_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule pre_requisites --> Entry\n");
    DEBUG_PRINT(DEBUG_LOG,"\nSeparate Initializations\n");
    dsError_t ret = dsERR_NONE;
    ret = dsHostInit();
    if (ret != dsERR_NONE)
    {
	DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsHostInit Failed\n");
        goto EXIT;
    }
    ret = dsDisplayInit();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsDisplayInit Failed\n");
        goto EXIT;
    }
    ret = dsAudioPortInit();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsAudioPortInit Failed\n");
        goto EXIT;
    }
    ret = dsVideoPortInit();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsVideoPortInit Failed\n");
        goto EXIT;
    }
    ret = dsVideoDeviceInit();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsVideoDeviceInit Failed\n");
        goto EXIT;
    }

EXIT :
    if (ret == dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule pre_requisites --> Exit\n");
        return "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule pre_requisites --> Exit\n");
        return "FAILURE";
    }
}
/***************************************************************************
 *Function name : testmodulepost_requisites
 *Description    : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool DSHalAgent::testmodulepost_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule post_requisites --> Entry\n");

    dsError_t ret = dsERR_NONE; 
    DEBUG_PRINT(DEBUG_TRACE, "\nSeparate Terminations\n");

    ret = dsVideoDeviceTerm();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsVideoDeviceTerm Failed\n");
        goto EXIT;
    }
    ret = dsVideoPortTerm();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsVideoPortTerm Failed\n");
        goto EXIT;
    }
    ret = dsAudioPortTerm();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsAudioPortTerm Failed\n");
        goto EXIT;
    }
    ret = dsDisplayTerm();
    if (ret != dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal : dsDisplayTerm Failed\n");
        goto EXIT;
    }
    vpHandle = 0;
    vdHandle = 0;
    apHandle = 0;
    dispHandle = 0;
    
EXIT :
    if (ret == dsERR_NONE)
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule post_requisites --> Exit\n");
        return TEST_SUCCESS;
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "DSHal testmodule post_requisites --> Exit\n");
        return TEST_FAILURE;
    }
}

/**************************************************************************
Function Name   : CreateObject

Arguments       : NULL

Description     : This function is used to create a new object of the class "DSHalAgent".
**************************************************************************/

extern "C" DSHalAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new DSHalAgent(ptrtcpServer);
}

/***************************************************************************
 *Function name : initialize
 *Description    : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/

bool DSHalAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "DSHal Initialization Entry\n");
    DEBUG_PRINT (DEBUG_TRACE, "DSHal Initialization Exit\n");
    return TEST_SUCCESS;
}

/***************************************************************************
 *Function name : checkERROR
 *Description    : This function is to check the ERROR return code of API
******************************************************************************/
void checkERROR(dsError_t ret,string *error)
{
    switch(ret)
    {
     case dsERR_NONE : DEBUG_PRINT(DEBUG_TRACE, "NO ERROR : dsERR_NONE\n");
		       *error="dsERR_NONE";
		       break;
     case dsERR_GENERAL : DEBUG_PRINT(DEBUG_ERROR, "ERROR : dsERR_GENERAL\n");
			  *error=" ERROR:dsERR_GENERAL";
                          break;
     case dsERR_INVALID_PARAM : DEBUG_PRINT(DEBUG_ERROR, "ERROR : dsERR_INVALID_PARAM\n");
				*error=" ERROR:dsERR_INVALID_PARAM";
                                break;
     case dsERR_INVALID_STATE : DEBUG_PRINT(DEBUG_ERROR, "ERROR : dsERR_INVALID_STATE\n");
				*error=" ERROR:dsERR_INVALID_STATE";
                                break;
     case dsERR_OPERATION_NOT_SUPPORTED : DEBUG_PRINT(DEBUG_ERROR, "ERROR : dsERR_OPERATION_NOT_SUPPORTED\n");
					  *error=" ERROR:dsERR_OPERATION_NOT_SUPPORTED";
                                          break;
     default :DEBUG_PRINT(DEBUG_ERROR, "UNEXPECTED ERROR OBSERVED\n");
	      *error="ERROR:UNEXPECTED ERROR";
    }
}
/***************************************************************************
 *Function name : DSHal_GetVideoPort
 *Description    : This function is to get the video port handle
 *****************************************************************************/
void DSHalAgent::DSHal_GetVideoPort(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoPort --->Entry\n");
    if(&req["portType"] == NULL || &req["index"] == NULL)
    {
        return;
    }

    dsVideoPortType_t portType = (dsVideoPortType_t) req["portType"].asInt();
    int index = req["index"].asInt();
    int get_DefaultType = req["get_DefaultType"].asInt();

    dsError_t ret = dsERR_NONE;
    ret = dsGetVideoPort(portType, index, &vpHandle);

    /*videoPortType = {"HDMI":6, "INTERNAL":8}

    if portName is HDMI0 , check if portInstance is obtained
    if portInstance is not obtained , try to obtain portInstance for secondary_portType

    In order to skip this functionality get_DefaultType must be set to 0 */

    if ((portType == 6) && (get_DefaultType == 1))
    {
        if (ret != dsERR_NONE)
        {
            DEBUG_PRINT(DEBUG_LOG, "Unable to GetVideoPort Instance as videoPort is unsupported");
            DEBUG_PRINT(DEBUG_LOG, "Getting secondary videoPort instance instead");
            portType = (dsVideoPortType_t) req["secondary_portType"].asInt();
            ret = dsGetVideoPort(portType, index, &vpHandle);
        }
    }


    if (ret == dsERR_NONE and vpHandle)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Video port handle retrieved";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetVideoPort call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoPort -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Videoport handle not retrieved" + error;
	DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",vpHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetVideoPort call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoPort -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetAudioPort
 *Description    : This function is to get the audio port handle
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioPort(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioPort --->Entry\n");
    if(&req["portType"] == NULL || &req["index"] == NULL)
    {
        return;
    }

    dsAudioPortType_t portType = (dsAudioPortType_t) req["portType"].asInt();
    int index = req["index"].asInt();
    int get_DefaultType = req["get_DefaultType"].asInt();

    dsError_t ret = dsERR_NONE;
    ret = dsGetAudioPort(portType, index, &apHandle);

    /*audioPortType = {"LR":0, "HDMI":1, "SPDIF":2, "SPEAKER":3, "HDMI_ARC":4, "HEADPHONE":5, "INVALID":9}

    if portName is HDMI0 , check if portInstance is obtained
    if portInstance is not obtained , try to obtain portInstance for secondary_portType

    In order to skip this functionality get_DefaultType must be set to 0 */

    if ((portType == 1) && (get_DefaultType == 1))
    {
        if (ret != dsERR_NONE)
        {
            DEBUG_PRINT(DEBUG_LOG, "Unable to GetAudioPort Instance as audioPort is unsupported");
            DEBUG_PRINT(DEBUG_LOG, "Getting secondary audioPort instance instead");
            portType = (dsAudioPortType_t) req["secondary_portType"].asInt();
            ret = dsGetAudioPort(portType, index, &apHandle);
        }
    }

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Audioport handle retrieved";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioPort call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioPort -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audioport handle not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioPort call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioPort -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetDisplay
 *Description    : This function is to get the handle of the video display device
 *****************************************************************************/
void DSHalAgent::DSHal_GetDisplay(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplay --->Entry\n");
    if(&req["portType"] == NULL || &req["index"] == NULL)
    {
        return;
    }

    dsVideoPortType_t portType = (dsVideoPortType_t) req["portType"].asInt();
    int index = req["index"].asInt();


    dsError_t ret = dsERR_NONE;
    ret = dsGetDisplay(portType, index, &dispHandle);

    if (ret == dsERR_NONE and dispHandle)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Display handle retrieved";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetDisplay call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplay -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Display handle not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",dispHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetDisplay call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplay -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetSurroundMode
 *Description    : This function is to get the surround mode of the video port
 *****************************************************************************/
void DSHalAgent::DSHal_GetSurroundMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundMode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    int surroundMode;
    ret = dsGetSurroundMode(vpHandle, &surroundMode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = surroundMode;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSurroundMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundMode -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Surround mode not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Mode : %d\n",surroundMode);	
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSurroundMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetStereoMode
 *Description    : This function is to get the stereo mode of the audio port
 *****************************************************************************/
void DSHalAgent::DSHal_GetStereoMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoMode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsAudioStereoMode_t stereoMode;

    ret = dsGetStereoMode(apHandle, &stereoMode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = stereoMode;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetStereoMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Stereo mode not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Mode : %d\n",stereoMode);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetStereoMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetStereoMode
 *Description    : This function is to set the stereo mode of the audio port
 *****************************************************************************/
void DSHalAgent::DSHal_SetStereoMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoMode --->Entry\n");
    if(&req["stereoMode"] == NULL)
    {
        return;
    }

    dsAudioStereoMode_t stereoMode = (dsAudioStereoMode_t) req["stereoMode"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetStereoMode(apHandle, stereoMode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetStereoMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetStereoMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetStereoMode not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Mode : %d\n",stereoMode);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetStereoMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoMode -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetAudioEncoding
 *Description    : This function is to get the current audio encoding setting for the audio port
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioEncoding(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioEncoding --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsAudioEncoding_t encoding;

#ifdef ENCODING_SUPPORT
    ret = dsGetAudioEncoding(apHandle, &encoding);
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_GetAudioEncoding is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioEncoding -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = encoding;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioEncoding call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioEncoding -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Encoding setting not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Encoding : %d\n",encoding);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioEncoding call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioEncoding -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_SetAudioEncoding
 *Description    : This function is to set the current audio encoding setting for the audio port
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioEncoding(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioEncoding --->Entry\n");
    dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

    if(&req["encoding"] == NULL)
    {
        return;
    }

    dsAudioEncoding_t encoding = (dsAudioEncoding_t) req["encoding"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}

#ifdef ENCODING_SUPPORT
    ret = dsSetAudioEncoding(paramhandle, encoding);
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_SetAudioEncoding is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioEncoding -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = encoding;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioEncoding call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioEncoding -->Exit\n");
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Encoding setting not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Encoding : %d\n",encoding);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioEncoding call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioEncoding -->Exit\n");
    }
	return;
}

/***************************************************************************
 *Function name : DSHal_IsAudioPortEnabled
 *Description    : This function is to indicate whether the specified Audio port is enabled or not
 *****************************************************************************/
void DSHalAgent::DSHal_IsAudioPortEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioPortEnabled --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool enabled;

    ret = dsIsAudioPortEnabled(apHandle, &enabled);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = enabled;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioPortEnabled call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioPortEnabled -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audio port enable status not retrieved"+ error;
	DEBUG_PRINT(DEBUG_TRACE1, "Port Status : %d\n",enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioPortEnabled call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioPortEnabled -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_EnableAudioPort
 *Description    : This function is to enable or disable the specified Audio port
 *****************************************************************************/
void DSHalAgent::DSHal_EnableAudioPort(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableAudioPort--->Entry\n");
    if(&req["enable"] == NULL)
    {
        return;
    }

    bool enable = req["enable"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsEnableAudioPort(apHandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "EnableAudioPort call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableAudioPort call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableAudioPort -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableAudioPort call failed"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Port Status : %d\n",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableAudioPort call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableAudioPort -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsDisplayConnected
 *Description    : This function is to find out whether the video port is connected to a display or not
 *****************************************************************************/
void DSHalAgent::DSHal_IsDisplayConnected(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplayConnected --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool connected;

    ret = dsIsDisplayConnected(vpHandle, &connected);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = connected;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsDisplayConnected call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplayConnected -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Display connection status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d\n",connected);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsDisplayConnected call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplayConnected -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsDisplaySurround
 *Description    : This function is to check if the connected sink device supports surround
 *****************************************************************************/
void DSHalAgent::DSHal_IsDisplaySurround(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplaySurround --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool surround;

    ret = dsIsDisplaySurround(vpHandle, &surround);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = surround;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsDisplaySurround call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplaySurround -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Display surround status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Surround : %d\n",surround);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsDisplaySurround call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsDisplaySurround -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetHDCPProtocol
 *Description    : This function is to get the STB HDCP protocol version
 *****************************************************************************/
void DSHalAgent::DSHal_GetHDCPProtocol(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPProtocol --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsHdcpProtocolVersion_t protocolVersion;

    ret = dsGetHDCPProtocol(vpHandle, &protocolVersion);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = protocolVersion;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetHDCPProtocol call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPProtocol -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HDCP protocol version not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Protocol : %d\n",protocolVersion);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetHDCPProtocol call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPProtocol -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetHDCPReceiverProtocol
 *Description    : This function is to get the Receiver/TV HDCP protocol version
 *****************************************************************************/
void DSHalAgent::DSHal_GetHDCPReceiverProtocol(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPReceiverProtocol --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsHdcpProtocolVersion_t protocolVersion;

    ret = dsGetHDCPReceiverProtocol(vpHandle, &protocolVersion);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = protocolVersion;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetHDCPReceiverProtocol call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPReceiverProtocol -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HDCP receiver protocol version not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Protocol : %d\n",protocolVersion);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetHDCPReceiverProtocol call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPReceiverProtocol -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetHDCPCurrentProtocol
 *Description    : This function is to get the current used HDCP protocol version
 *****************************************************************************/
void DSHalAgent::DSHal_GetHDCPCurrentProtocol(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPCurrentProtocol --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsHdcpProtocolVersion_t protocolVersion;

    ret = dsGetHDCPCurrentProtocol(vpHandle, &protocolVersion);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = protocolVersion;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetHDCPCurrentProtocol call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPCurrentProtocol -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HDCP receiver protocol version not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Protocol : %d\n",protocolVersion);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetHDCPCurrentProtocol call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDCPCurrentProtocol -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsVideoPortEnabled
 *Description    : This function is to indicate whether the specified Video port is enabled or not
 *****************************************************************************/
void DSHalAgent::DSHal_IsVideoPortEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortEnabled --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool enabled;

    ret = dsIsVideoPortEnabled(vpHandle, &enabled);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = enabled;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsVideoPortEnabled call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortEnabled -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audio port enable status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d\n",enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsVideoPortEnabled call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortEnabled -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_EnableVideoPort
 *Description    : This function is to enable or disable the specified Video port
 *****************************************************************************/
void DSHalAgent::DSHal_EnableVideoPort(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableVideoPort--->Entry\n");
    if(&req["enable"] == NULL)
    {
        return;
    }

    bool enable = req["enable"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsEnableVideoPort(vpHandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "EnableVideoPort call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableVideoPort call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableVideoPort -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableVideoPort call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d\n",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableVideoPort call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableVideoPort -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetDisplayAspectRatio
 *Description    : This function is to get the aspect ratio of the video display
 *****************************************************************************/
void DSHalAgent::DSHal_GetDisplayAspectRatio(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplayAspectRatio --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsVideoAspectRatio_t aspect;

    ret = dsGetDisplayAspectRatio(dispHandle, &aspect);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = aspect;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetDisplayAspectRatio call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplayAspectRatio -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Display aspect ratio not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Aspect Ratio : %d\n",aspect);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetDisplayAspectRatio call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDisplayAspectRatio -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetColorDepth
 *Description    : This function is to get the color depth value
 *****************************************************************************/
void DSHalAgent::DSHal_GetColorDepth(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorDepth --->Entry\n");

    dsError_t ret = dsERR_NONE;
    unsigned int depth;

    ret = dsGetColorDepth(vpHandle, &depth);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = depth;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetColorDepth call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorDepth -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Color depth value not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Color Depth : %d\n",depth);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetColorDepth call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorDepth -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetColorSpace
 *Description    : This function is to get the color space value
 *****************************************************************************/
void DSHalAgent::DSHal_GetColorSpace(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorSpace --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsDisplayColorSpace_t space;

    ret = dsGetColorSpace(vpHandle, &space);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = space;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetColorSpace call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorSpace -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Color space value not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Color Space : %d\n",space);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetColorSpace call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetColorSpace -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsVideoPortActive
 *Description    : This function is to find out whether a video port is connected to the active port of sink device
 *****************************************************************************/
void DSHalAgent::DSHal_IsVideoPortActive(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortActive --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool active;

    ret = dsIsVideoPortActive(vpHandle, &active);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = active;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsVideoPortActive call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortActive -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Video port active status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Port : %d\n",active);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsVideoPortActive call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsVideoPortActive -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_HdmiInGetNumberOfInputs
 *Description    : This function is to get the  number of HDMI inputs
 *****************************************************************************/
void DSHalAgent::DSHal_HdmiInGetNumberOfInputs(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetNumberOfInputs --->Entry\n");

#ifndef NO_HDMI_INPUT
    dsError_t ret = dsERR_NONE;
    uint8_t noOfInputs;

    ret = dsHdmiInGetNumberOfInputs(&noOfInputs);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = noOfInputs;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_HdmiInGetNumberOfInputs call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetNumberOfInputs -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Hdmi Number Of Inputs not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "noofInputs : %d\n",noOfInputs);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInGetNumberOfInputs call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetNumberOfInputs -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_HdmiInGetStatus
 *Description    : This function is to get the HDMI status
 *****************************************************************************/
void DSHalAgent::DSHal_HdmiInGetStatus(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus --->Entry\n");
#ifndef NO_HDMI_INPUT
    dsError_t ret = dsERR_NONE;
    dsHdmiInStatus_t pStatus;
    char status[200]= {'\0'};
    ret = dsHdmiInGetStatus(&pStatus);

    sprintf(status, "isPresented:%d,activePort:%d,isPortConnected_Port0:%d,isPortConnected_Port1:%d", pStatus.isPresented,pStatus.activePort,pStatus.isPortConnected[0], pStatus.isPortConnected[1]);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = status;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_HdmiInGetStatus call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Hdmi status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %s\n",status);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInGetStatus call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_IsOutputHDR
 *Description    : This function is to check if the video output is HDR or not
 *****************************************************************************/
void DSHalAgent::DSHal_IsOutputHDR(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsOutputHDR --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool hdr;

    ret = dsIsOutputHDR(vpHandle, &hdr);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = hdr;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsOutputHDR call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsOutputHDR -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Output HDR status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d\n",hdr);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsOutputHDR call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsOutputHDR -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsAudioMute
 *Description    : This function is to check whether the audio is muted or not
 *****************************************************************************/
void DSHalAgent::DSHal_IsAudioMute(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMute --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool muted;

    ret = dsIsAudioMute(apHandle, &muted);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = muted;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioMute call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMute -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audio mute status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d\n",muted);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioMute call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMute -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetAudioMute
 *Description    : This function is to set the audio to mute
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioMute(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMute--->Entry\n");
    if(&req["muted"] == NULL)
    {
        return;
    }

    bool muted = req["muted"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetAudioMute(apHandle, muted);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAudioMute call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioMute call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMute -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioMute call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioMute call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMute -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetAudioDelay
 *Description    : This function is to get the audio delay in milliseconds
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioDelay(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelay --->Entry\n");

    dsError_t ret = dsERR_NONE;
    uint32_t audioDelayMs;

    ret = dsGetAudioDelay(apHandle, &audioDelayMs);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = audioDelayMs;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioDelay call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelay -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "StereoMode not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Delay : %d\n",audioDelayMs);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioDelay call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelay -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetAudioDelay
 *Description    : This function is to set the audio delay in milliseconds
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioDelay(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelay --->Entry\n");
    if(&req["audioDelay"] == NULL)
    {
        return;
    }

    uint32_t audioDelayMs = req["audioDelay"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetAudioDelay(apHandle, audioDelayMs);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAudioDelay call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioDelay call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelay -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioDelay call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioDelay call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelay -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetAudioDelayOffset
 *Description    : This function is to get the audio delay offset in milliseconds
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioDelayOffset(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelayOffset --->Entry\n");

    dsError_t ret = dsERR_NONE;
    uint32_t audioDelayOffsetMs;

    //ret = dsGetAudioDelayOffset(apHandle, &audioDelayOffsetMs);
    response["result"] = "FAILURE";
    response["details"] = "dsGetAudioDelayOffset is deprecated";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelayOffset -->Exit\n");
    return;

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = audioDelayOffsetMs;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioDelayOffset call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelayOffset -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "StereoMode not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Delay : %d\n",audioDelayOffsetMs);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioDelayOffset call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDelayOffset -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetAudioDelayOffset
 *Description    : This function is to set the audio delay offset in milliseconds
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioDelayOffset(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelayOffset --->Entry\n");
    if(&req["offset"] == NULL)
    {
        return;
    }

    uint32_t audioDelayOffsetMs = req["offset"].asInt();
    dsError_t ret = dsERR_NONE;

    //ret = dsSetAudioDelayOffset(apHandle, audioDelayOffsetMs);
    response["result"] = "FAILURE";
    response["details"] = "dsSetAudioDelayOffset is deprecated";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelayOffset -->Exit\n");
    return;

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAudioDelayOffset call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioDelayOffset call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelayOffset -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioDelayOffset call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioDelayOffset call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDelayOffset -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsAudioMSDecode
 *Description    : This function is to check whether the audio port supports Dolby MS11 Multistream Decode
 *****************************************************************************/
void DSHalAgent::DSHal_IsAudioMSDecode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMSDecode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool ms11Enabled;

    ret = dsIsAudioMSDecode(apHandle, &ms11Enabled);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = ms11Enabled;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioMSDecode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMSDecode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "AudioMSDecode status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d", ms11Enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioMSDecode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMSDecode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_IsAudioMS12Decode
 *Description    : This function is to check whether whether the audio port supports MS12 Decode
 *****************************************************************************/
void DSHalAgent::DSHal_IsAudioMS12Decode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMS12Decode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool ms12Enabled;

    ret = dsIsAudioMS12Decode(apHandle, &ms12Enabled);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = ms12Enabled;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioMS12Decode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMS12Decode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "AudioMS12Decode status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d", ms12Enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioMS12Decode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioMS12Decode -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetHdmiPreference
 *Description    : This function is to get the Preferred HDMI Protocol
 *****************************************************************************/
void DSHalAgent::DSHal_GetHdmiPreference(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiPreference --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsHdcpProtocolVersion_t hdcpProtocol;

    ret = dsGetHdmiPreference(vpHandle, &hdcpProtocol);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = hdcpProtocol;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetHdmiPreference call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiPreference -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "StereoMode not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Protocol : %d", hdcpProtocol);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetHdmiPreference call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiPreference -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetHdmiPreference
 *Description    : This function is to set the Preferred HDMI Protocol
 *****************************************************************************/
void DSHalAgent::DSHal_SetHdmiPreference(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetHdmiPreference --->Entry\n");
    if(&req["stereoMode"] == NULL)
    {
        return;
    }

    dsHdcpProtocolVersion_t hdcpProtocol = (dsHdcpProtocolVersion_t) req["hdcpProtocol"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetHdmiPreference(vpHandle, &hdcpProtocol);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetHdmiPreference call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetHdmiPreference call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetHdmiPreference -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetHdmiPreference call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetHdmiPreference call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetHdmiPreference -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetBackgroundColor
 *Description    : This function is to set the back ground color
 *****************************************************************************/
void DSHalAgent::DSHal_SetBackgroundColor(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBackgroundColor --->Entry\n");
    if(&req["stereoMode"] == NULL)
    {
        return;
    }

    dsVideoBackgroundColor_t color = (dsVideoBackgroundColor_t) req["color"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetBackgroundColor(vpHandle, color);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetBackgroundColor call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetBackgroundColor call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBackgroundColor -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetBackgroundColor call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetBackgroundColor call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBackgroundColor -->Exit\n");
        return;
    }
}
/**************************************************************************
*Function name : DSHal_SetFPBrightness
*Description   : This function will set the brightness of the specified discrete LEDs on the Front
 Panel Display to the specified brightness level. The Power LED brightness setting
 will also be adjusted to this setting.
**************************************************************************/
void DSHalAgent::DSHal_SetFPBrightness(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBrightness --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsFPDIndicator_t eIndicator = (dsFPDIndicator_t) req["indicator"].asInt();
    dsFPDBrightness_t eBrightness = (dsFPDBrightness_t) req["brightness"].asInt();
    ret = dsSetFPBrightness( eIndicator,eBrightness);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Brightness set successfully to " + std::to_string(eBrightness);
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetFPBrightness call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBrightness -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        if(ret == dsERR_INVALID_PARAM)
        {
            response["details"] = "Given parameters are not supported for device";
        }
        else
        {
            response["details"] = "Brightness not set successfully"+error;
        }
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetFPBrightness call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBrightness -->Exit\n");
        return;
     }
}

/**************************************************************************
*Function name : DSHal_GetFPBrightness
*Description   : This function returns the brightness level of the specified discrete LEDs on the front
 panel
**************************************************************************/
void DSHalAgent::DSHal_GetFPBrightness(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetFPBrightness --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsFPDIndicator_t eIndicator = (dsFPDIndicator_t) req["indicator"].asInt();
    dsFPDBrightness_t eBrightness;
    ret = dsGetFPBrightness( eIndicator,&eBrightness);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = eBrightness;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetFPBrightness call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetFPBrightness -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        if(ret == dsERR_INVALID_PARAM)
        {
            response["details"] = "Given parameters are not supported for device";
        }
        else
        {
            response["details"] = "Brightness value not retrieved"+error;
        }
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetFPBrightness call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetFPBrightness -->Exit\n");
        return;
     }
}
/***************************************************************************
 *Function name  : DSHal_GetCPUTemperature
 *Description    : This function is to get the CPU temperature in centigrade
 *****************************************************************************/
void DSHalAgent::DSHal_GetCPUTemperature(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCPUTemperature --->Entry\n");
    float temp = 0.0;
    dsError_t ret = dsERR_NONE;
    ret = dsGetCPUTemperature(&temp);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = temp;
        DEBUG_PRINT(DEBUG_LOG, "dsGetCPUTemperature call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCPUTemperature -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "CPU Temperature value not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "TEMPERATURE :%f",temp);
        DEBUG_PRINT(DEBUG_ERROR, "dsGetCPUTemperature call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCPUTemperature -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_GetHDRCapabilities
 *Description    : This function is to get the STB HDR capabilities
 *****************************************************************************/
void DSHalAgent::DSHal_GetHDRCapabilities(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDRCapabilities --->Entry\n");
    int capability = 0;
    dsError_t ret = dsERR_NONE;
    ret = dsGetHDRCapabilities(vdHandle, &capability);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = capability;
        DEBUG_PRINT(DEBUG_LOG, "dsGetHDRCapabilities call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDRCapabilities -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "STB HDR capabilities not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Capability :%d",capability);
        DEBUG_PRINT(DEBUG_ERROR, "dsGetHDRCapabilities call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHDRCapabilities -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_GetSupportedVideoCodingFormats
 *Description    : This function is to get the supported Video Coding formats
 *****************************************************************************/
void DSHalAgent::DSHal_GetSupportedVideoCodingFormats(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedVideoCodingFormats --->Entry\n");
    unsigned int supportedFormat = 0;
    dsError_t ret = dsERR_NONE;
    ret = dsGetSupportedVideoCodingFormats(vdHandle, &supportedFormat);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = supportedFormat;
        DEBUG_PRINT(DEBUG_LOG, "dsGetSupportedVideoCodingFormats call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedVideoCodingFormats -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Supported video coding formats not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Format :%d",supportedFormat);
        DEBUG_PRINT(DEBUG_ERROR, "dsGetSupportedVideoCodingFormats call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedVideoCodingFormats -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_GetVideoCodecInfo
 *Description    : This function is to get supported video codec formats
 *****************************************************************************/
void DSHalAgent::DSHal_GetVideoCodecInfo(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoCodecInfo --->Entry\n");
    if(&req["format"] == NULL)
    {
        return;
    }
    char format[10];
    strcpy(format,req["format"].asCString());

    dsVideoCodecInfo_t info;
    info.num_entries = 0;
    dsVideoCodingFormat_t codingFormat;
    if (!strcmp(format,"MPEGH"))
        codingFormat = dsVIDEO_CODEC_MPEGHPART2;
    else if (!strcmp(format,"MPEG4"))
        codingFormat = dsVIDEO_CODEC_MPEG4PART10;
    else if (!strcmp(format,"MPEG2"))
        codingFormat = dsVIDEO_CODEC_MPEG2;
    else{
        response["result"] = "FAILURE";
        response["details"] = "Invalid Video Codec Format";
        return;
    }

    dsError_t ret = dsERR_NONE;
    ret = dsGetVideoCodecInfo(vdHandle,codingFormat,&info);
    if (ret == dsERR_NONE )
    {
        unsigned int entries = info.num_entries;
        if (entries > 0)
        {
            int basicInfoSize = 150; //size for the buffer
            char *details = (char*)malloc(basicInfoSize*entries);
            if (details == NULL)
            {
                response["result"]="FAILED";
                response["details"]="Failed to capture Video Codec Info Results";
                DEBUG_PRINT(DEBUG_TRACE,"\n Memory Allocation failed\n");
                return;
            }
            else
            {
                float level;
                int hevcProfile;
                string hevcProfileName;
                dsVideoCodecHevcProfiles_t profile;

                int dataLength;
                unsigned int count;
                char *details_ptr  = details;
                memset(details_ptr,'\0',basicInfoSize*entries);
                for (count=0; count<entries; count++)
                {
                    if (details[0] != '\0' )
                    {
                        sprintf(details_ptr,"|");
                        details_ptr++;
                    }
                    profile = info.entries[count].profile;
                    level   = info.entries[count].level;
                    hevcProfile = static_cast<int>(profile);
                    if ( profile == dsVIDEO_CODEC_HEVC_PROFILE_MAIN)
                        hevcProfileName = "dsVIDEO_CODEC_HEVC_PROFILE_MAIN";
                    else if ( profile == dsVIDEO_CODEC_HEVC_PROFILE_MAIN10 )
                        hevcProfileName = "dsVIDEO_CODEC_HEVC_PROFILE_MAIN10";
                    else
                        hevcProfileName = "dsVIDEO_CODEC_HEVC_PROFILE_MAINSTILLPICTURE";
                    dataLength = sprintf(details_ptr,"Profile:%d[%s],level:%f",hevcProfile,hevcProfileName.c_str(),level);
                    details_ptr = details_ptr + dataLength;
                }
                response["result"] = "SUCCESS";
                response["details"] = details;
                free(details);
                DEBUG_PRINT(DEBUG_LOG, "dsGetVideoCodecInfo call is SUCCESS");
                DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoCodecInfo -->Exit\n");
                return;
            }
        }
        else{
            response["result"]="SUCCESS";
            response["details"]="Codec info currently supports only HEVC[MPEGH] codec";
            DEBUG_PRINT(DEBUG_LOG, "dsGetVideoCodecInfo call is SUCCESS");
            DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoCodecInfo -->Exit\n");
            return;
        }
    }
    else if (ret == dsERR_OPERATION_NOT_SUPPORTED){
        response["result"] = "SUCCESS";
        response["details"] = "Operation Not Supported";
        DEBUG_PRINT(DEBUG_ERROR, "dsGetVideoCodecInfo call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoCodecInfo -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Video Codec Info not retrieved"+error;
        DEBUG_PRINT(DEBUG_ERROR, "dsGetVideoCodecInfo call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoCodecInfo -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_GetTVHDRCapabilities
 *Description    : This function is to get the TV HDR capabilities
 *****************************************************************************/
void DSHalAgent::DSHal_GetTVHDRCapabilities(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetTVHDRCapabilities --->Entry\n");
    int capabilities = 0;
    dsError_t ret = dsERR_NONE;
    ret = dsGetTVHDRCapabilities(vpHandle, &capabilities);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = capabilities;
        DEBUG_PRINT(DEBUG_LOG, "dsGetTVHDRCapabilities call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetTVHDRCapabilities -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "TV HDR capabilities not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Capability : %d",capabilities);
        DEBUG_PRINT(DEBUG_ERROR, "dsGetTVHDRCapabilities call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetTVHDRCapabilities -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_SetFPBlink
 *Description    : This function is to set the individual discrete LEDs to blink
                   for a specified number of times at the specified blink interval
 *****************************************************************************/
void DSHalAgent::DSHal_SetFPBlink(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBlink --->Entry\n");
    if(&req["indicator"] == NULL || &req["blinkDuration"] == NULL || &req["blinkIteration"] == NULL)
    {
        return;
    }

    dsFPDIndicator_t eIndicator = (dsFPDIndicator_t) req["indicator"].asInt();
    if (eIndicator > dsFPD_INDICATOR_MAX)
        DEBUG_PRINT(DEBUG_TRACE,"Invalid LED Indicator");

    unsigned int uBlinkDuration  = req["blinkDuration"].asUInt();
    unsigned int uBlinkIteration = req["blinkIteration"].asUInt();

    dsError_t ret = dsERR_NONE;
    ret = dsSetFPBlink(eIndicator,uBlinkDuration,uBlinkIteration);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "dsSetFPBlink call success";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPBlink call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBlink -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "Invalid LED Indicator";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPBlink call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBlink -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsSetFPBlink call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetFPBlink call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPBlink -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_SetFPColor
 *Description    : This function sets the color of the specified front panel
                   indicator LED, if the indicator supports it (i.e. is multi-colored)
 *****************************************************************************/
void DSHalAgent::DSHal_SetFPColor(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPColor --->Entry\n");
    if(&req["indicator"] == NULL || &req["color"] == NULL )
    {
        return;
    }

    char color[10] = {'\0'};
    strcpy(color,req["color"].asCString());

    dsFPDIndicator_t eIndicator = (dsFPDIndicator_t) req["indicator"].asInt();
    if (eIndicator > dsFPD_INDICATOR_MAX)
        DEBUG_PRINT(DEBUG_TRACE,"Invalid LED Indicator");

    dsFPDColor_t eColor;
    if(!strcmp(color, "BLUE"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_BLUE;
    else if(!strcmp(color,"GREEN"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_GREEN;
    else if(!strcmp(color,"RED"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_RED;
    else if(!strcmp(color,"YELLOW"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_YELLOW;
    else if(!strcmp(color,"ORANGE"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_ORANGE;
    else if(!strcmp(color,"WHITE"))
        eColor = (dsFPDColor_t)dsFPD_COLOR_WHITE;
    else{
        response["result"] = "FAILURE";
        response["details"] = "Invalid LED Indicator Color";
        return;
    }

    dsError_t ret = dsERR_NONE;
    ret = dsSetFPColor(eIndicator,eColor);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "dsSetFPColor call success";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPColor call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPColor -->Exit\n");
        return;
    }
    else if (ret == dsERR_OPERATION_NOT_SUPPORTED)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Operation not supported : LED indicator is single-colored";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPColor call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPColor -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "Invalid LED Indicator";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPColor call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPColor -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsSetFPColor call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "dsSetFPColor call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPColor -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_SetFPTime
 *Description    : This function sets the 7-segment display LEDs to show the time.
                   The format  (12/24-hour) must be specified
 *****************************************************************************/
void DSHalAgent::DSHal_SetFPTime(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTime --->Entry\n");
    if(&req["format"] == NULL || &req["hours"] == NULL || &req["minutes"] == NULL )
    {
        return;
    }
    char format[10] = {'\0'};
    strcpy(format,req["format"].asCString());
    unsigned int hours = req["hours"].asUInt();
    unsigned int mins = req["minutes"].asUInt();

    dsFPDTimeFormat_t timeFormat;
    if(!strcmp(format, "12_HOUR"))
        timeFormat = dsFPD_TIME_12_HOUR;
    else if(!strcmp(format,"24_HOUR"))
        timeFormat = dsFPD_TIME_24_HOUR;
    else if(!strcmp(format,"STRING"))
        timeFormat = dsFPD_TIME_MAX;
    else{
        response["result"] = "FAILURE";
        response["details"] = "Invalid Time format";
        return;
    }

    dsError_t ret = dsERR_NONE;
    ret = dsSetFPTime(timeFormat,hours,mins);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "dsSetFPTime call success";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPTime call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTime -->Exit\n");
        return;
    }
    else if (ret == dsERR_OPERATION_NOT_SUPPORTED)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Operation not supported : 7-Segment display LEDs not available";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPTime call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTime -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "Invalid hours/minutes or Time Format and hours values do not agree";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPTime call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTime -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsSetFPTime call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "dsSetFPTime call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTime -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_SetFPText
 *Description    : This function sets the 7-segment display LEDs to show the given text
 *****************************************************************************/
void DSHalAgent::DSHal_SetFPText(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPText --->Entry\n");
    if(&req["text"] == NULL)
    {
        return;
    }
    char text[50] = {'\0'};
    strcpy(text,req["text"].asCString());
    dsError_t ret = dsERR_NONE;
    ret = dsSetFPText(text);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "dsSetFPText call success";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPText call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPText -->Exit\n");
        return;
    }
    else if (ret == dsERR_OPERATION_NOT_SUPPORTED)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Operation not supported : 7-Segment display LEDs not available";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPText call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPText -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsSetFPText call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "dsSetFPText call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPText -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name  : DSHal_SetFPTextBrightness
 *Description    : This function will set the brightness of the specified 7-Segment Display LEDs
 *****************************************************************************/
void DSHalAgent::DSHal_SetFPTextBrightness(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTextBrightness --->Entry\n");
    if(&req["brightness"] == NULL )
    {
        return;
    }
    dsFPDBrightness_t eBrightness = (dsFPDBrightness_t) req["brightness"].asUInt();
    dsFPDTextDisplay_t eIndicator = dsFPD_TEXTDISP_TEXT;
    dsError_t ret = dsERR_NONE;
    ret = dsSetFPTextBrightness(eIndicator,eBrightness);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "dsSetFPTextBrightness call success";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPTextBrightness call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTextBrightness -->Exit\n");
        return;
    }
    else if (ret == dsERR_OPERATION_NOT_SUPPORTED)
    {
        response["result"] = "SUCCESS";
        response["details"] = "Operation not supported : 7-Segment display LEDs not available";
        DEBUG_PRINT(DEBUG_LOG, "dsSetFPTextBrightness call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTextBrightness -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsSetFPTextBrightness call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "dsSetFPTextBrightness call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetFPTextBrightness -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetVideoEOTF
 *Description    : This function is to get the video Electro-Optical Transfer Function
 *****************************************************************************/
void DSHalAgent::DSHal_GetVideoEOTF(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoEOTF --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsHDRStandard_t eotf;

    ret = dsGetVideoEOTF(vpHandle, &eotf);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = eotf;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetVideoEOTF call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoEOTF -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EOTF value not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "EOTF : %d",eotf);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetVideoEOTF call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVideoEOTF -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_HdmiInScaleVideo
 *Description    : This function is to scale video
 *****************************************************************************/
void DSHalAgent::DSHal_HdmiInScaleVideo(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInScaleVideo --->Entry\n");
#ifndef NO_HDMI_INPUT
    if(&req["x"] == NULL || &req["y"] == NULL || &req["width"] == NULL || &req["height"] == NULL )
    {
        return;
    }

    int32_t x = req["x"].asInt();
    int32_t y = req["y"].asInt();
    int32_t width = req["width"].asInt();
    int32_t height = req["height"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsHdmiInScaleVideo(x, y, width, height);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_HdmiInScaleVideo call is SUCCESS";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_HdmiInScaleVideo call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInScaleVideo -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_HdmiInScaleVideo call is FAILURE"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInScaleVideo call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInScaleVideo -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_IsHDCPEnabled
 *Description    : This function is to check if HDCP is enabled
 *****************************************************************************/
void DSHalAgent::DSHal_IsHDCPEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsHDCPEnabled --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool contentProtected;

    ret = dsIsHDCPEnabled(vpHandle, &contentProtected);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = contentProtected;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsHDCPEnabled call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsHDCPEnabled -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HDCP status not retrieved"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status: %d",contentProtected);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsHDCPEnabled call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsHDCPEnabled -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_EnableLEConfig
 *Description    : This function is to enable or disable the LE config
 *****************************************************************************/
void DSHalAgent::DSHal_EnableLEConfig(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLEConfig--->Entry\n");
    if(&req["enable"] == NULL)
    {
        return;
    }

    bool enable = req["enable"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsEnableLEConfig(apHandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "EnableLEConfig call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableLEConfig call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLEConfig -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLEConfig -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableLEConfig call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableLEConfig call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLEConfig -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetLEConfig
 *Description    : This function is to check if LE feature is enabled
 *****************************************************************************/
void DSHalAgent::DSHal_GetLEConfig(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetLEConfig --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool enable;

    ret = dsGetLEConfig(apHandle, &enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = enable;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetLEConfig call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetLEConfig -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetLEConfig -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetLEConfig call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Status : %d",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetLEConfig call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetLEConfig -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetSupportedTvResolutions
 *Description    : This function is to get the supported TV resolutions
 *****************************************************************************/
void DSHalAgent::DSHal_GetSupportedTvResolutions(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedTvResolutions --->Entry\n");

    dsError_t ret = dsERR_NONE;
    int resolutions;

    ret = dsSupportedTvResolutions(vpHandle, &resolutions);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = resolutions;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSupportedTvResolutions call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedTvResolutions -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid video handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedTvResolutions -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetSupportedTvResolutions call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Resolutions : %d",resolutions);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSupportedTvResolutions call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedTvResolutions -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetMatrixCoefficients
 *Description    : This function is to matrix coefficients setting
 *****************************************************************************/
void DSHalAgent::DSHal_GetMatrixCoefficients(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMatrixCoefficients --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsDisplayMatrixCoefficients_t coefficients;

    ret = dsGetMatrixCoefficients(vpHandle, &coefficients);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = coefficients;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMatrixCoefficients call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMatrixCoefficients -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetMatrixCoefficients call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Coefficients : %d",coefficients);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetMatrixCoefficients call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMatrixCoefficients -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetDolbyVolumeMode
 *Description    : This function is to set the dolby volume mode
 *****************************************************************************/
void DSHalAgent::DSHal_SetDolbyVolumeMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDolbyVolumeMode--->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    bool mode = req["mode"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetDolbyVolumeMode(apHandle, mode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetDolbyVolumeMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetDolbyVolumeMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDolbyVolumeMode -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDolbyVolumeMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetDolbyVolumeMode call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetDolbyVolumeMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDolbyVolumeMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetDolbyVolumeMode
 *Description    : This function is to get the dolby volume mode
 *****************************************************************************/
void DSHalAgent::DSHal_GetDolbyVolumeMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDolbyVolumeMode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    bool mode;

    ret = dsGetDolbyVolumeMode(apHandle, &mode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = mode;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetDolbyVolumeMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDolbyVolumeMode -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDolbyVolumeMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetDolbyVolumeMode call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Mode : %d",mode);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetDolbyVolumeMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDolbyVolumeMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetResolution
 *Description    : This function is to set video port's display resolution
 *****************************************************************************/
void DSHalAgent::DSHal_SetResolution(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetResolution--->Entry\n");
    if(&req["resolution"] == NULL)
    {
        return;
    }
    dsVideoPortResolution_t resolution;
    std::string resolutionName=req["resolution"].asCString();
    resolution.pixelResolution = (dsVideoResolution_t) req["pixelResolution"].asInt();
    resolution.aspectRatio = (dsVideoAspectRatio_t) req["aspectRatio"].asInt();
    resolution.stereoScopicMode = (dsVideoStereoScopicMode_t) req["stereoScopicMode"].asInt();
    resolution.frameRate = (dsVideoFrameRate_t) req["frameRate"].asInt();
    resolution.interlaced = req["interlaced"].asInt();
    strcpy(resolution.name,resolutionName.c_str());
    dsError_t ret = dsERR_NONE;

    ret = dsSetResolution(vpHandle, &resolution);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetResolution call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetResolution call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetResolution -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetResolution -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetResolution call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetResolution call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetResolution -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetResolution
 *Description    : This function is to get video port's display resolution
 *****************************************************************************/
void DSHalAgent::DSHal_GetResolution(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetResolution --->Entry\n");

    dsError_t ret = dsERR_NONE;
    char output[50];
    dsVideoPortResolution_t resolution;

    ret = dsGetResolution(vpHandle, &resolution);
    sprintf(output, "Name: %s", resolution.name);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetResolution call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetResolution -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid display handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetResolution -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetResolution call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Resolutions : %s",resolution);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetResolution call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetResolution -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetSocIDFromSDK
 *Description    : This function is to get the soc id
 *****************************************************************************/
void DSHalAgent::DSHal_GetSocIDFromSDK(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSocIDFromSDK --->Entry\n");

    dsError_t ret = dsERR_NONE;
    char id[20];

    ret = dsGetSocIDFromSDK(id);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = id;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSocIDFromSDK call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSocIDFromSDK -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetSocIDFromSDK call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "SocID : %s",id);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSocIDFromSDK call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSocIDFromSDK -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetIntelligentEqualizerMode
 *Description    : This function is to set the intelligent equalizer mode
 *****************************************************************************/
void DSHalAgent::DSHal_SetIntelligentEqualizerMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetIntelligentEqualizerMode--->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    int mode = req["mode"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetIntelligentEqualizerMode(apHandle, mode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetIntelligentEqualizerMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetIntelligentEqualizerMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetIntelligentEqualizerMode -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetIntelligentEqualizerMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetIntelligentEqualizerMode call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetIntelligentEqualizerMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetIntelligentEqualizerMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetIntelligentEqualizerMode
 *Description    : This function is to get the intelligent equalizer mode
 *****************************************************************************/
void DSHalAgent::DSHal_GetIntelligentEqualizerMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetIntelligentEqualizerMode --->Entry\n");

    dsError_t ret = dsERR_NONE;
    int mode;

    ret = dsGetIntelligentEqualizerMode(apHandle, &mode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = mode;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetIntelligentEqualizerMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetIntelligentEqualizerMode -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetIntelligentEqualizerMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetIntelligentEqualizerMode call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Mode : %d",mode);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetIntelligentEqualizerMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetIntelligentEqualizerMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetDialogEnhancement
 *Description    : This function is to set the dialogue enhancement level
 *****************************************************************************/
void DSHalAgent::DSHal_SetDialogEnhancement(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDialogEnhancement--->Entry\n");
    if(&req["level"] == NULL)
    {
        return;
    }

	//Level is within the range 1 to 15
    int level = req["level"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetDialogEnhancement(apHandle, level);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetDialogEnhancement call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetDialogEnhancement call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDialogEnhancement -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDialogEnhancement -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetDialogEnhancement call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetDialogEnhancement call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDialogEnhancement -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetDialogEnhancement
 *Description    : This function is to get the dialogue enhancement level
 *****************************************************************************/
void DSHalAgent::DSHal_GetDialogEnhancement(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDialogEnhancement --->Entry\n");

    dsError_t ret = dsERR_NONE;
    int level;

    ret = dsGetDialogEnhancement(apHandle, &level);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = level;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetDialogEnhancement call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDialogEnhancement -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDialogEnhancement -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetDialogEnhancement call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Level : %d",level);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetDialogEnhancement call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDialogEnhancement -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_HdmiInSelectZoomMode
 *Description    : This function is to select the zoom mode
 *****************************************************************************/
void DSHalAgent::DSHal_HdmiInSelectZoomMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectZoomMode--->Entry\n");
#ifndef NO_HDMI_INPUT
    if(&req["mode"] == NULL)
    {
        return;
    }

    dsVideoZoom_t zoomMode = (dsVideoZoom_t) req["mode"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsHdmiInSelectZoomMode(zoomMode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "HdmiInSelectZoomMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_HdmiInSelectZoomMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectZoomMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HdmiInSelectZoomMode call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInSelectZoomMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectZoomMode -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_GetEDID
 *Description    : This function is to get the EDID information of the connected display
 *****************************************************************************/
void DSHalAgent::DSHal_GetEDID(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDID --->Entry\n");

    dsError_t ret = dsERR_NONE;
    char output[300];
    dsDisplayEDID_t edidInfo;

    ret = dsGetEDID(dispHandle, &edidInfo);
    sprintf(output, "productCode:%d,serialNumber:%d,manufactureYear:%d,manufactureWeek:%d,hdmiDeviceType:%d.isRepeater:%d,physicalAddressA:%d,physicalAddressB:%d,physicalAddressC:%d,physicalAddressD:%d,numOfSupportedResolution:%d", edidInfo.productCode, edidInfo.serialNumber,edidInfo.manufactureYear,edidInfo.manufactureWeek,edidInfo.hdmiDeviceType,edidInfo.isRepeater,edidInfo.physicalAddressA,edidInfo.physicalAddressB,edidInfo.physicalAddressC,edidInfo.physicalAddressD,edidInfo.numOfSupportedResolution);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetEDID call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDID -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetEDID call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetEDID call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDID -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : EDID_Verify
 *Description   : Verify the EDID bytes
 ***************************************************************************/
bool EDID_Verify(unsigned char* bytes, size_t count) {
    if (!bytes || count < 128) {
        return false;
    }
    static const unsigned char header[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    if (memcmp(bytes, header, sizeof(header)) != 0) {
        DEBUG_PRINT(DEBUG_ERROR, "Incorrect input, header does not match: %02x %02x %02x %02x %02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]);
        return false;
    }
    return true;
}
/***************************************************************************
 *Function name : DSHal_GetEDIDBytes
 *Description    : This function is to get the EDID Bytes of the connected display
 *****************************************************************************/
void DSHalAgent::DSHal_GetEDIDBytes(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDIDBytes --->Entry\n");
    char details[512];
    dsError_t ret = dsERR_NONE;

    std::vector<unsigned char> edid;
    int length = 0;
    unsigned char edid_bytes_data[512] = {0};
    ret = dsGetEDIDBytes(dispHandle, edid_bytes_data, &length);
    if (ret == dsERR_NONE)
    {
        if(EDID_Verify(edid_bytes_data, length)==true)
        {
               edid.insert(edid.begin(), edid_bytes_data, edid_bytes_data + length);
               DEBUG_PRINT(DEBUG_TRACE, "\t Display [%s] has %d bytes EDID\r\n", "HDMI",  edid.size());
                /* Dump the bytes */
                for (int i = 0; i < edid.size(); i++) {
                   if (i % 16 == 0) {
                       printf("\r\n");
                   }
                   if (i % 128 == 0) {
                       printf("\r\n");
                   }
                   printf("%02X ", edid[i]);
                   if ((i*2) < sizeof(details))
                       sprintf(&details[i*2],"%02X",edid[i]);
                }
               response["result"] = "SUCCESS";
               response["details"] = details;
               DEBUG_PRINT(DEBUG_LOG, "EDIDBytes retrived successfully");
               DEBUG_PRINT(DEBUG_LOG, "dsGetEDIDBytes call is SUCCESS");
               DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDIDBytes -->Exit\n");
               return;
       }
       else
       {
               printf("Failed to verify EDIDbytes\n");
               response["result"] = "FAILURE";
               response["details"] = "EDIDBytes are not valid..";
               DEBUG_PRINT(DEBUG_LOG, "dsGetEDIDBytes call FAILED");
               DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDIDBytes -->Exit\n");
               return;
       }
    }
    else if (ret == dsERR_INVALID_PARAM){
        response["result"] = "FAILURE";
        response["details"] = "Invalid display handle";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid display handle");
        DEBUG_PRINT(DEBUG_LOG, "dsGetEDIDBytes call FAILED");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDIDBytes -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetEDIDBytes call failed"+error;
        DEBUG_PRINT(DEBUG_LOG, "dsGetEDIDBytes call FAILED");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetEDIDBytes -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetCurrentOutputSettings
 *Description    : This function is to get the EDID information of the connected display
 *****************************************************************************/
void DSHalAgent::DSHal_GetCurrentOutputSettings(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCurrentOutputSettings --->Entry\n");

    dsError_t ret = dsERR_NONE;
    char output[80];
    dsHDRStandard_t eotf;
    dsDisplayMatrixCoefficients_t coefficients;
    dsDisplayColorSpace_t colorSpace;
    unsigned int colorDepth;
    dsDisplayQuantizationRange_t quantizationRange = dsDISPLAY_QUANTIZATIONRANGE_UNKNOWN;

    ret = dsGetCurrentOutputSettings(vpHandle, &eotf, &coefficients, &colorSpace, &colorDepth, &quantizationRange);
    sprintf(output, "eotf:%d,coefficients:%d,colorSpace:%d,colorDepth:%d, quantizationRange:%d", eotf, coefficients, colorSpace, colorDepth, quantizationRange);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetCurrentOutputSettings call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCurrentOutputSettings -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetCurrentOutputSettings call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetCurrentOutputSettings call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetCurrentOutputSettings -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_HdmiInSelectPort
 *Description    : This function is to select the hdmiin port
 *****************************************************************************/
void DSHalAgent::DSHal_HdmiInSelectPort(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectPort--->Entry\n");
#ifndef NO_HDMI_INPUT
    if(&req["port"] == NULL)
    {
        return;
    }

    dsHdmiInPort_t port = (dsHdmiInPort_t) req["port"].asInt();
    dsError_t ret = dsERR_NONE;

    /*
     * dsHdmiInSelectPort declaration changed in RDK-42825
     * Commenting the existing API call to avoid build failure
     * TODO:Implement dsHdmiInSelectPort according to below declaration
     * dsHdmiInSelectPort (dsHdmiInPort_t Port, bool audioMix, dsVideoPlaneType_t evideoPlaneType,bool topMost);
     */

    //ret = dsHdmiInSelectPort(port);
    response["result"] = "FAILURE";
    response["details"] = "HdmiInSelectPort test not implemented";
    DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInSelectPort call is not implemented");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectPort -->Exit\n");
    return;

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "HdmiInSelectPort call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_HdmiInSelectPort call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectPort -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "HdmiInSelectPort call failed";
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_HdmiInSelectPort call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInSelectPort -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_GetHdmiInCurrentVideoMode
 *Description    : This function is to get the current hdmiin video mode
 *****************************************************************************/
void DSHalAgent::DSHal_GetHdmiInCurrentVideoMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiInCurrentVideoMode --->Entry\n");
#ifndef NO_HDMI_INPUT

    dsError_t ret = dsERR_NONE;
    char output[100];
    dsVideoPortResolution_t resolution;

    ret =  dsHdmiInGetCurrentVideoMode(&resolution);
    sprintf(output, "Name:%s,pixelResolution:%d,frameRate:%d,interlaced:%d ", resolution.name,resolution.pixelResolution,resolution.frameRate,resolution.interlaced);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetHdmiInCurrentVideoMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiInCurrentVideoMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetHdmiInCurrentVideoMode call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetHdmiInCurrentVideoMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetHdmiInCurrentVideoMode -->Exit\n");
        return;
    }
#else
    response["result"] = "FAILURE";
    response["details"] = "Hdmi Input Port is not present in this device";
    DEBUG_PRINT(DEBUG_LOG, "Hdmi Input Port is not present in this device");
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_HdmiInGetStatus -->Exit\n");
    return;
#endif
}
/***************************************************************************
 *Function name : DSHal_GetSinkDeviceAtmosCapability
 *Description    : This function is to get the audio sink device ATMOS capability
 *****************************************************************************/
void DSHalAgent::DSHal_GetSinkDeviceAtmosCapability(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSinkDeviceAtmosCapability --->Entry\n");

    dsError_t ret = dsERR_NONE;
    dsATMOSCapability_t capability;

    ret = dsGetSinkDeviceAtmosCapability(apHandle, &capability);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = capability;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSinkDeviceAtmosCapability call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSinkDeviceAtmosCapability -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSinkDeviceAtmosCapability -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetSinkDeviceAtmosCapability call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Capability :%d",capability);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSinkDeviceAtmosCapability call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSinkDeviceAtmosCapability -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetAudioAtmosOutputMode
 *Description    : This function is to set the Audio Atmos output mode
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioAtmosOutputMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioAtmosOutputMode--->Entry\n");
    if(&req["enable"] == NULL)
    {
        return;
    }

    bool enable = req["enable"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetAudioAtmosOutputMode(apHandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAudioAtmosOutputMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioAtmosOutputMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioAtmosOutputMode -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID HANDLE";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid audio handle");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioAtmosOutputMode -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioAtmosOutputMode call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioAtmosOutputMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioAtmosOutputMode -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetAudioCompression
 *Description    : This function is to get the audio compression
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioCompression(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCompression --->Entry\n");

    dsError_t ret = dsERR_NONE;
    int audioCompression;

    ret = dsGetAudioCompression(apHandle, &audioCompression);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = audioCompression;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioCompression call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCompression -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "AudioCompression not retrieved" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioCompression call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCompression -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetAudioCompression
 *Description    : This function is to set the audio compression
 *****************************************************************************/
void DSHalAgent::DSHal_SetAudioCompression(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioCompression --->Entry\n");
    if(&req["audioCompression"] == NULL)
    {
        return;
    }

    int audioCompression = req["audioCompression"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetAudioCompression(apHandle, audioCompression);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAudioCompression call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioCompression call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioCompression -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioCompression call failed" + error;
   	    DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioCompression call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioCompression -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_SetSurroundVirtualizer
 *Description    : This function is to set the Surround Virtualizer mode
 *****************************************************************************/
void DSHalAgent::DSHal_SetSurroundVirtualizer(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetSurroundVirtualizer --->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    dsSurroundVirtualizer_t virtualizer;
    virtualizer.mode = req["mode"].asInt();
    virtualizer.boost = req["boost"].asInt();

    dsError_t ret = dsERR_NONE;

    ret = dsSetSurroundVirtualizer(apHandle,virtualizer);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetSurroundVirtualizer call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetSurroundVirtualizer call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetSurroundVirtualizer -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetSurroundVirtualizer call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetSurroundVirtualizer call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetSurroundVirtualizer -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetSurroundVirtualizer
 *Description    : This function is to get the Surround Virtualizer mode
 *****************************************************************************/
void DSHalAgent::DSHal_GetSurroundVirtualizer(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundVirtualizer --->Entry\n");
    if(&req["virtualizer"] == NULL)
    {
        return;
    }

    char output[300];
    dsSurroundVirtualizer_t Virtualizer;
    dsError_t ret = dsERR_NONE;

    ret = dsGetSurroundVirtualizer(apHandle,&Virtualizer);
    sprintf(output, "Mode : %d , Boost : %d" , Virtualizer.mode, Virtualizer.boost);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSurroundVirtualizer call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundVirtualizer -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetSurroundVirtualizer call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSurroundVirtualizer call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSurroundVirtualizer -->Exit\n");
        return;
    }
}

/**************************************************************************************************
 *Function name : DSHal_SetVolumeLeveller
 *Description    : This function is to set Volume leveller mode & level used in a given audio port
 *************************************************************************************************/
void DSHalAgent::DSHal_SetVolumeLeveller(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetVolumeLeveller --->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }
    dsVolumeLeveller_t volLeveller;
    volLeveller.mode = req["mode"].asInt();
    volLeveller.level = req["level"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetVolumeLeveller(apHandle,volLeveller);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetVolumeLeveller call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetVolumeLeveller call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetVolumeLeveller -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetVolumeLeveller call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetVolumeLeveller call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetVolumeLeveller -->Exit\n");
        return;
    }
}

/**************************************************************************************************
 *Function name : DSHal_GetVolumeLeveller
 *Description    : This function is to set Volume leveller mode & level used in a given audio port
 *************************************************************************************************/
void DSHalAgent::DSHal_GetVolumeLeveller(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVolumeLeveller --->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    char output[300];
    dsVolumeLeveller_t VolLeveller;
    dsError_t ret = dsERR_NONE;

    ret = dsGetVolumeLeveller(apHandle,&VolLeveller);
    sprintf(output, "Mode : %d , Level : %d" , VolLeveller.mode, VolLeveller.level);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetVolumeLeveller call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVolumeLeveller -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetVolumeLeveller call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetVolumeLeveller call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetVolumeLeveller -->Exit\n");
        return;
    }
}

/**************************************************************************************************
 *Function name : DSHal_SetDRCMode
 *Description    : This function is to set dynamic range control mode used in a given audio port
 *************************************************************************************************/
void DSHalAgent::DSHal_SetDRCMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDRCMode --->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    int mode;
    mode = req["mode"].asInt();
    dsError_t ret = dsERR_NONE;

    ret = dsSetDRCMode(apHandle,mode);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetDRCMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetDRCMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDRCMode -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetDRCMOde call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetDRCMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetDRCMode -->Exit\n");
        return;
    }
}

/**************************************************************************************************
 *Function name : DSHal_GetDRCMode
 *Description    : This function is to get dynamic range control mode used in a given audio port
 *************************************************************************************************/
void DSHalAgent::DSHal_GetDRCMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDRCMode --->Entry\n");
    if(&req["mode"] == NULL)
    {
        return;
    }

    char output[300];
    int mode;
    dsError_t ret = dsERR_NONE;


    ret = dsGetDRCMode(apHandle,&mode);
    sprintf(output, "Mode : %d" , mode);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = output;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetDRCMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDRCMode -->Exit\n");
        return;
    }
    else
    {
        checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetDRCMode call failed" + error;
        DEBUG_PRINT(DEBUG_TRACE1, "Handle : %d\n",apHandle);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetDRCMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetDRCMode -->Exit\n");
        return;
    }
}


/***************************************************************************
 *Function name : DSHal_GetQuantizationRange
 *Description    : This function is to get the quantization_range
 *****************************************************************************/
void DSHalAgent::DSHal_GetQuantizationRange(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetQuantizationRange --->Entry\n");
    dsError_t ret = dsERR_NONE;
    dsDisplayQuantizationRange_t quantization_range = dsDISPLAY_QUANTIZATIONRANGE_UNKNOWN;
    ret = dsGetQuantizationRange(vpHandle, &quantization_range);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = quantization_range;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetQuantizationRange call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetQuantizationRange -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Quantization_range not retrieved"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Range : %d\n",quantization_range);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetQuantizationRange call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetQuantizationRange -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_SetBassEnhancer
 *Description    : This function is to set the Bass Enhancement value
 *****************************************************************************/
void DSHalAgent::DSHal_SetBassEnhancer(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBassEnhancer--->Entry\n");
    if(&req["boost"] == NULL)
    {
        return;
    }
    int boost = req["boost"].asInt();
    dsError_t ret = dsERR_NONE;
    ret = dsSetBassEnhancer(apHandle, boost);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetBassEnhancer success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetBassEnhancer call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBassEnhancer -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBassEnhancer -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetBassEnhancer call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetBassEnhancer call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetBassEnhancer -->Exit\n");
        return;
    }
}
/***************************************************************************
 *Function name : DSHal_GetBassEnhancer
 *Description    : This function is to get the bass enhancement value
 *****************************************************************************/
void DSHalAgent::DSHal_GetBassEnhancer(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetBassEnhancer --->Entry\n");
    dsError_t ret = dsERR_NONE;
    int boost;
    ret = dsGetBassEnhancer(apHandle, &boost);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = boost;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetBassEnhancer call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetBassEnhancer -->Exit\n");
        return;
    }
    else if (ret == dsERR_INVALID_PARAM)
    {
        response["result"] = "FAILURE";
        response["details"] = "INVALID PARAM";
        DEBUG_PRINT(DEBUG_ERROR, "Invalid parameter");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetBassEnhancer -->Exit\n");
        return;
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetBassEnhancer call failed"+error;
	DEBUG_PRINT(DEBUG_TRACE1, "Boost : %d",boost);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetBassEnhancer call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetBassEnhancer -->Exit\n");
        return;
    }
}

/***************************************************************************
 *Function name : DSHal_GetAudioCapabilities
 *Description    : This function is to get the platform audio capabilities
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioCapabilities(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCapabilities --->Entry\n");
    dsError_t ret = dsERR_NONE;
    int audcapabilities = 0, paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioCapabilities handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetAudioCapabilities((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetAudioCapabilities((intptr_t)paramhandle, &audcapabilities);
	}
	
    if (ret == dsERR_NONE)
    {
		std::string outputDetails;
		
		outputDetails += "   Supported Audio Capabilities of the platform: ";
		if(audcapabilities & dsAUDIOSUPPORT_NONE)
		{
			outputDetails += "   Not supported any audio types";
		}
		if(audcapabilities & dsAUDIOSUPPORT_ATMOS)
		{
			outputDetails += "   Dolby ATMOS,";
		}		
		if(audcapabilities & dsAUDIOSUPPORT_DD)
		{
			outputDetails += "   Dolby Digital,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_DDPLUS)
		{
			outputDetails += "   Dolby Digital Plus,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_DAD)
		{
			outputDetails += "   Digital Audio Delivery,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_DAPv2)
		{
			outputDetails += "   Digital Audio Processing version 2,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_MS12)
		{
			outputDetails += "   Multi Stream 12,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_MS12V2)
		{
			outputDetails += "   Multi Stream Version 2,";
		}
		if(audcapabilities & dsAUDIOSUPPORT_Invalid)
		{
			outputDetails += "   Invalid audio type";
		}
		DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioCapabilities %s\n", outputDetails);
		
        response["result"] = "SUCCESS";
        response["details"] = outputDetails;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioCapabilities call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCapabilities -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetAudioCapabilities call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioCapabilities call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioCapabilities -->Exit\n");
    }
	
	return;
}

/***************************************************************************
 *Function name : checkaudioFormat
 *Description    : This function is to check the ERROR return code of API
******************************************************************************/
void checkaudioFormat(dsAudioFormat_t audioformat,string *audformat)
{
    switch(audioformat)
    {
     case dsAUDIO_FORMAT_NONE : DEBUG_PRINT(DEBUG_LOG, "No audio format\n");
		       *audformat="Audio Format: dsAUDIO_FORMAT_NONE";
		       break;
     case dsAUDIO_FORMAT_PCM : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_PCM\n");
			  *audformat="Audio Format: dsAUDIO_FORMAT_PCM";
                          break;
     case dsAUDIO_FORMAT_DOLBY_AC3 : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_AC3\n");
				*audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_AC3";
                                break;
     case dsAUDIO_FORMAT_DOLBY_EAC3 : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_EAC3\n");
				*audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_EAC3";
                                break;
     case dsAUDIO_FORMAT_DOLBY_AC4 : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_AC4\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_AC4";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_MAT : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_MAT\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_AC4";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_TRUEHD : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_TRUEHD\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_TRUEHD";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_EAC3_ATMOS : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_EAC3_ATMOS\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_EAC3_ATMOS";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_TRUEHD_ATMOS : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_TRUEHD_ATMOS\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_TRUEHD_ATMOS";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_MAT_ATMOS : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_MAT_ATMOS\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_MAT_ATMOS";
                                          break;
     case dsAUDIO_FORMAT_DOLBY_AC4_ATMOS : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_DOLBY_AC4_ATMOS\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_DOLBY_AC4_ATMOS";
                                          break;
     case dsAUDIO_FORMAT_AAC : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_AAC\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_AAC";
                                          break;
     case dsAUDIO_FORMAT_VORBIS : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_VORBIS\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_VORBIS";
                                          break;
     case dsAUDIO_FORMAT_WMA : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_WMA\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_WMA";
                                          break;
     case dsAUDIO_FORMAT_UNKNOWN : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_UNKNOWN\n");
					  *audformat="Audio Format: dsAUDIO_FORMAT_UNKNOWN";
                                          break;
     case dsAUDIO_FORMAT_MAX : DEBUG_PRINT(DEBUG_LOG, "dsAUDIO_FORMAT_MAX\n");
					  *audformat="Audio Format: Out of Range";
                                          break;
     default :DEBUG_PRINT(DEBUG_ERROR, "UNEXPECTED ERROR OBSERVED\n");
	      *audformat="ERROR:UNEXPECTED ERROR";
    }
}

/***************************************************************************
 *Function name : DSHal_GetAudioFormat
 *Description    : This function is to get the current audio format
 *****************************************************************************/
void DSHalAgent::DSHal_GetAudioFormat(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioFormat --->Entry\n");
	dsAudioFormat_t enaudioFormat = dsAUDIO_FORMAT_NONE;
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioFormat handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
	  ret = dsGetAudioFormat((intptr_t)paramhandle, NULL);
	}
	else
	{
	  ret = dsGetAudioFormat((intptr_t)paramhandle, &enaudioFormat);
	}
	
	if (ret == dsERR_NONE)
	{
	  std::string audformat;
	  checkaudioFormat(enaudioFormat, &audformat);
	  DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioFormat %s\n", audformat);
	  response["result"] = "SUCCESS";
	  response["details"] = audformat;
	  DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioFormat call is SUCCESS");
	  DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioFormat -->Exit\n");
	}
	else
	{
	  checkERROR(ret,&error);
          response["result"] = "FAILURE";
          response["details"] = "GetAudioFormat call failed"+error;
          DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioFormat call is FAILURE");
          DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioFormat -->Exit\n");
	}	
	return;
}

/***************************************************************************
 *Function name : DSHal_EnableMS12Config
 *Description    : This function is to enable/disable the MS12 config features
 *****************************************************************************/
void DSHalAgent::DSHal_EnableMS12Config(IN const Json::Value& req, OUT Json::Value& response)
{
	DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableMS12Config --->Entry\n");
	dsMS12FEATURE_t enMS12feature = dsMS12FEATURE_DAPV2;
    dsError_t ret = dsERR_NONE;
	
	ret = dsEnableMS12Config(apHandle, enMS12feature, true);
	if (ret == dsERR_NONE)
    {	
        response["result"] = "SUCCESS";
        response["details"] = "MS12: DAPV2 feature got enabled successfully";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableMS12Config call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableMS12Config -->Exit\n");
	}
	else
	{
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableMS12Config call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableMS12Config call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableMS12Config -->Exit\n");
	}	
	return;
}

/*********************************************************************************************
 *Function name : DSHal_IsSurroundDecoderEnabled
 *Description    : This function is to get the enable/disable status of audio Surround Decoder
 **********************************************************************************************/
void DSHalAgent::DSHal_IsSurroundDecoderEnabled(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsSurroundDecoderEnabled --->Entry\n");
    dsError_t ret = dsERR_NONE;
	int paramhandle = 0;
    bool enabled = 0;

	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_IsSurroundDecoderEnabled handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsIsSurroundDecoderEnabled((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsIsSurroundDecoderEnabled((intptr_t)paramhandle, &enabled);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = enabled;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsSurroundDecoderEnabled call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsSurroundDecoderEnabled -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audio Surround Decoder status not retrieved "+ error;
		DEBUG_PRINT(DEBUG_TRACE1, "Audio Surround Decoder Status : %d\n",enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsSurroundDecoderEnabled call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsSurroundDecoderEnabled -->Exit\n");
    }
	return;
}

/************************************************************************************************************
 *Function name : DSHal_EnableSurroundDecoder
 *Description    : This function is to enable or disable audio surround decoder of the specified Audio port
 ************************************************************************************************************/
void DSHalAgent::DSHal_EnableSurroundDecoder(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableSurroundDecoder--->Entry\n");
    if(&req["enable"] == NULL)
    {
        return;
    }

	int paramhandle = 0;
    bool enable = req["enable"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
    dsError_t ret = dsERR_NONE;

	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsEnableSurroundDecoder((intptr_t)paramhandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "EnableSurroundDecoder call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableSurroundDecoder call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableSurroundDecoder -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableSurroundDecoder call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableSurroundDecoder call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableSurroundDecoder -->Exit\n");
    }
	return;
}

/*******************************************************************************
 *Function name : DSHal_GetMS12Capabilities
 *Description    : This function is used to get the supported MS12 capabilities
 ********************************************************************************/
void DSHalAgent::DSHal_GetMS12Capabilities(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12Capabilities --->Entry\n");
    dsError_t ret = dsERR_NONE;
    int audcapabilities = 0, paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12Capabilities handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetMS12Capabilities((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetMS12Capabilities((intptr_t)paramhandle, &audcapabilities);
	}
	
    if (ret == dsERR_NONE)
    {
		std::string outputDetails;
		
		outputDetails += "   Platform supported MS12 Capabilities: ";
		if(audcapabilities & dsMS12SUPPORT_NONE)
		{
			outputDetails += "   Not supported any audio types";
		}
		if(audcapabilities & dsMS12SUPPORT_DolbyVolume)
		{
			outputDetails += "   Dolby Volume,";
		}		
		if(audcapabilities & dsMS12SUPPORT_InteligentEqualizer)
		{
			outputDetails += "   Intelligent Equalizer,";
		}
		if(audcapabilities & dsMS12SUPPORT_DialogueEnhancer)
		{
			outputDetails += "   Dialogue Enhancer,";
		}
		if(audcapabilities & MS12SUPPORT_Volumeleveller)
		{
			outputDetails += "   Volume leveller,";
		}
		if(audcapabilities & MS12SUPPORT_BassEnhancer)
		{
			outputDetails += "   Bass Enhancer,";
		}
		if(audcapabilities & MS12SUPPORT_SurroundDecoder)
		{
			outputDetails += "   Surround Decoder,";
		}
		if(audcapabilities & MS12SUPPORT_DRCMode)
		{
			outputDetails += "   DRC Mode,";
		}
		if(audcapabilities & MS12SUPPORT_SurroundVirtualizer)
		{
			outputDetails += "   Surround Virtualizer,";
		}
		if(audcapabilities & MS12SUPPORT_MISteering)
		{
			outputDetails += "   MI Steering,";
		}
		if(audcapabilities & MS12SUPPORT_GraphicEqualizer)
		{
			outputDetails += "   Graphic equalizer,";
		}
		if(audcapabilities & MS12SUPPORT_LEConfig)
		{
			outputDetails += "   LE config,";
		}
		if(audcapabilities & MS12SUPPORT_Invalid)
		{
			outputDetails += "   Invalid/Out of range,";
		}
		DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12Capabilities %s\n", outputDetails);
		
        response["result"] = "SUCCESS";
        response["details"] = outputDetails;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12Capabilities call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12Capabilities -->Exit\n");
    }
    else
    {
	checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "GetMS12Capabilities call failed"+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetMS12Capabilities call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12Capabilities -->Exit\n");
    }	
	return;
}

/************************************************************************************************
 *Function name : DSHal_GetMISteering
 *Description    : This function is used to get the Media Intelligent steering of the audio port
 ************************************************************************************************/
void DSHalAgent::DSHal_GetMISteering(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMISteering --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	int paramhandle = 0;
    bool enabled = 0;

	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsGetMISteering((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetMISteering((intptr_t)paramhandle, &enabled);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details,"Media Intelligent Steering Enabled State : %d", enabled);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMISteering call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMISteering -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsGetMISteering call getting failed due to "+ error;
		DEBUG_PRINT(DEBUG_TRACE1, "Audio Surround Decoder Status : %d\n",enabled);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetMISteering call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMISteering -->Exit\n");
    }
	return;
}

/********************************************************************************************
 *Function name : DSHal_SetMISteering
 *Description    : This function is to set the Media Intelligent steering of the audio port
 ********************************************************************************************/
void DSHalAgent::DSHal_SetMISteering(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMISteering --->Entry\n");
	if(&req["enable"] == NULL)
    {
        return;
    }

	int paramhandle = 0;
    bool enable = req["enable"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
    dsError_t ret = dsERR_NONE;

	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetMISteering((intptr_t)paramhandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetMISteering call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetMISteering call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMISteering -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetMISteering call failed "+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Enable Status : %d\n",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetMISteering call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMISteering -->Exit\n");
    }

	return;
}

/********************************************************************************
 *Function name : DSHal_GetMS12AudioProfileList
 *Description    : This function is used to get the supported MS12 audio profiles
 *********************************************************************************/
void DSHalAgent::DSHal_GetMS12AudioProfileList(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfileList --->Entry\n");
    dsError_t ret = dsERR_NONE;
	dsMS12AudioProfileList_t stAudioprofiles = { 0 };
	char details[100];
    int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12AudioProfileList handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetMS12AudioProfileList((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetMS12AudioProfileList((intptr_t)paramhandle, &stAudioprofiles);
	}
	
    if (ret == dsERR_NONE)
    {
		sprintf(details,"Audio profile count: %d, Profile lists: %s",stAudioprofiles.audioProfileCount,stAudioprofiles.audioProfileList);
        response["result"] = "SUCCESS";
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12AudioProfileList call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfileList -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_GetMS12AudioProfileList call failed "+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetMS12AudioProfileList call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfileList -->Exit\n");
    }	
	return;
}

/**********************************************************************************
 *Function name : DSHal_GetMS12AudioProfile
 *Description    : This function is used to get the current audio profile selection
 **********************************************************************************/
void DSHalAgent::DSHal_GetMS12AudioProfile(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfile --->Entry\n");
    dsError_t ret = dsERR_NONE;
	char strProfile[50] = {'\0'};
    int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12AudioProfile handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetMS12AudioProfile((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetMS12AudioProfile((intptr_t)paramhandle, strProfile);
	}
	
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = strProfile;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetMS12AudioProfile call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfile -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_GetMS12AudioProfile call failed "+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetMS12AudioProfile call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetMS12AudioProfile -->Exit\n");
    }	
	return;
}

/*****************************************************************
 *Function name : DSHal_SetMS12AudioProfile
 *Description    : This function is to set the MS12 audio profile
 *****************************************************************/
void DSHalAgent::DSHal_SetMS12AudioProfile(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMS12AudioProfile --->Entry\n");
	int paramhandle = 0;
	//char aud_profiles[50] = "Music,Movie,Sports,Entertainment";
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	char aud_profiles[50] = {'\0'};
	dsError_t ret = dsERR_NONE;
	
	std::string audioprofile = req["audioprofile"].asCString();
	strcpy(aud_profiles,audioprofile.c_str());
	
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsSetMS12AudioProfile((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsSetMS12AudioProfile((intptr_t)paramhandle, aud_profiles);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetMS12AudioProfile call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetMS12AudioProfile call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMS12AudioProfile -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetMS12AudioProfile call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetMS12AudioProfile call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetMS12AudioProfile -->Exit\n");
    }
	return;
}

/*****************************************************************************************************
 *Function name : DSHal_GetAssociatedAudioMixing
 *Description    : This function is used to get the Associated Audio Mixing status - enabled/disabled
 ******************************************************************************************************/
void DSHalAgent::DSHal_GetAssociatedAudioMixing(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAssociatedAudioMixing --->Entry\n");
    dsError_t ret = dsERR_NONE;
	bool bAudioMixingStatus = 0;
    int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAssociatedAudioMixing handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetAssociatedAudioMixing((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetAssociatedAudioMixing((intptr_t)paramhandle, &bAudioMixingStatus);
	}
	
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = bAudioMixingStatus;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAssociatedAudioMixing call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAssociatedAudioMixing -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_GetAssociatedAudioMixing call failed "+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAssociatedAudioMixing call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAssociatedAudioMixing -->Exit\n");
    }	
	return;
}

/********************************************************************************************
 *Function name : DSHal_SetAssociatedAudioMixing
 *Description    : This function is used to enables/disables associated audio mixing feature
 ********************************************************************************************/
void DSHalAgent::DSHal_SetAssociatedAudioMixing(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAssociatedAudioMixing --->Entry\n");
	
	if(&req["enable"] == NULL)
    {
        return;
    }

	int paramhandle = 0;
    bool enable = req["enable"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
    dsError_t ret = dsERR_NONE;

	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetAssociatedAudioMixing((intptr_t)paramhandle, enable);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetAssociatedAudioMixing call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAssociatedAudioMixing call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAssociatedAudioMixing -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAssociatedAudioMixing call failed "+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Audio mixing enable status : %d\n",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAssociatedAudioMixing call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAssociatedAudioMixing -->Exit\n");
    }
	return;
}

/***********************************************************************************************************************************
 *Function name : DSHal_GetGraphicEqualizerMode
 *Description    : This function is used to get the Graphic Equalizer Mode(0 = EQ OFF, 1 = EQ Open, 2 = EQ Rich and 3 = EQ Focused)
 ***********************************************************************************************************************************/
void DSHalAgent::DSHal_GetGraphicEqualizerMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetGraphicEqualizerMode --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	int EqlzrMode = 0, paramhandle = 0;
  
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsGetGraphicEqualizerMode((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetGraphicEqualizerMode((intptr_t)paramhandle, &EqlzrMode);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Received Graphic Equalizer mode : %d", EqlzrMode);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetGraphicEqualizerMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetGraphicEqualizerMode -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsGetGraphicEqualizerMode call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetGraphicEqualizerMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetGraphicEqualizerMode -->Exit\n");
    }
	return;
}

/*****************************************************************************************************************************
 *Function name : DSHal_SetGraphicEqualizerMode
 *Description    : This function is to set the Graphic Equalizer Mode(0 = EQ OFF, 1 = EQ Open, 2 = EQ Rich and 3 = EQ Focused)
 ******************************************************************************************************************************/
void DSHalAgent::DSHal_SetGraphicEqualizerMode(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetGraphicEqualizerMode --->Entry\n");
	int paramhandle = 0;
	
	if(&req["mode"] == NULL)
    {
        return;
    }

    int mode = (int) req["mode"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
    dsError_t ret = dsERR_NONE;

	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetGraphicEqualizerMode((intptr_t)paramhandle, mode);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetGraphicEqualizerMode call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetGraphicEqualizerMode call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetGraphicEqualizerMode -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetGraphicEqualizerMode call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetGraphicEqualizerMode call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetGraphicEqualizerMode -->Exit\n");
    }

	return;
}

/********************************************************************************************************
 *Function name : DSHal_GetSupportedARCTypes
 *Description    : This function is used to get the supported ARC types of the connected ARC/eARC device
 *********************************************************************************************************/
void DSHalAgent::DSHal_GetSupportedARCTypes(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedARCTypes --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	int ArcTypes = 0, paramhandle = 0;
  
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsGetSupportedARCTypes((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetSupportedARCTypes((intptr_t)paramhandle, &ArcTypes);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Supported ARC Type : %d", ArcTypes);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetSupportedARCTypes call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedARCTypes -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsGetSupportedARCTypes call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetSupportedARCTypes call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetSupportedARCTypes -->Exit\n");
    }
	return;
}

/**************************************************************************************************
 *Function name : DSHal_AudioEnableARC
 *Description    : This function is to enable/disable ARC/eARC and route audio to connected device
 **************************************************************************************************/
void DSHalAgent::DSHal_AudioEnableARC(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_AudioEnableARC--->Entry\n");
 	dsAudioARCStatus_t stARCStatus;
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

    dsAudioARCTypes_t arctype = (dsAudioARCTypes_t) req["ARCType"].asInt();
	bool enable = req["enable"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
    
	stARCStatus.type = arctype;
	stARCStatus.status = enable;
	
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsAudioEnableARC(paramhandle, stARCStatus);

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "AudioEnableARC call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_AudioEnableARC call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_AudioEnableARC -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "AudioEnableARC call failed"+ error;
        DEBUG_PRINT(DEBUG_TRACE1, "Port Status : %d\n",enable);
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_AudioEnableARC call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_AudioEnableARC -->Exit\n");
    }
	return;
}

/***********************************************************************************************************
 *Function name : DSHal_GetStereoAuto
 *Description    : This function is used to check if auto mode is enabled or not for the digital interfaces
 ***********************************************************************************************************/
void DSHalAgent::DSHal_GetStereoAuto(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoAuto --->Entry\n");
	char details[100];
	int autoModeState = 0, paramhandle = 0;
	dsError_t ret = dsERR_NONE;
  
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsGetStereoAuto((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetStereoAuto((intptr_t)paramhandle, &autoModeState);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Stereo Auto mode enabled state : %d", autoModeState);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetStereoAuto call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoAuto -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsGetStereoAuto call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetStereoAuto call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetStereoAuto -->Exit\n");
    }
	return;
}

/*******************************************************************************************
 *Function name : DSHal_SetStereoAuto
 *Description    : This function is used to Sets the Auto Mode to be used on the audio port
 *******************************************************************************************/
void DSHalAgent::DSHal_SetStereoAuto(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoAuto--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	int enable = req["automode"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetStereoAuto((intptr_t)paramhandle, enable);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "SetStereoAuto call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetStereoAuto call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoAuto -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetStereoAuto call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetStereoAuto call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetStereoAuto -->Exit\n");
    }
	return;
}

/*******************************************************************************
 *Function name : DSHal_GetAudioGain
 *Description    : This function is used to get the audio gain of an audio port
 *******************************************************************************/
void DSHalAgent::DSHal_GetAudioGain(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioGain --->Entry\n");
	char details[100];
	int paramhandle = 0;
	float audiogain = 0.0;
	dsError_t ret = dsERR_NONE;
  
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
	if (Isnullparamcheck)
	{
		ret = dsGetAudioGain((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetAudioGain((intptr_t)paramhandle, &audiogain);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Audio gain value : %f", audiogain);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioGain call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioGain -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsGetAudioGain call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioGain call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioGain -->Exit\n");
    }
	return;
}

/**********************************************************************************
 *Function name : DSHal_SetAudioGain
 *Description    : This function is used to Sets the audio gain of an audio port
 **********************************************************************************/
void DSHalAgent::DSHal_SetAudioGain(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioGain--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	float audiogain = (float) req["audiogain"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetAudioGain((intptr_t)paramhandle, audiogain);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_SetAudioGain call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioGain call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioGain -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioGain call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioGain call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioGain -->Exit\n");
    }
	return;
}

/***************************************************************************************
 *Function name : DSHal_GetAudioDB
 *Description    : This function is used to get current audio dB level of an audio port
 ***************************************************************************************/
void DSHalAgent::DSHal_GetAudioDB(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDB --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	float fAudiodB_level = 0.0;
    int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioDB handle %d %d",paramhandle, apHandle );

#ifdef AUDIO_DB_SUPPORT
	if (Isnullparamcheck)
	{
		ret = dsGetAudioDB((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetAudioDB((intptr_t)paramhandle, &fAudiodB_level);
	}
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_GetAudioDB is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDB -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Current audio dB audio level : %f", fAudiodB_level);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioDB call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDB -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_GetAudioDB call failed "+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioDB call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioDB -->Exit\n");
    }	
	return;
}

/*******************************************************************************************
 *Function name : DSHal_SetAudioDB
 *Description    : This function is used to sets the current audio dB level of an audio port
 ********************************************************************************************/
void DSHalAgent::DSHal_SetAudioDB(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDB--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	float audiodBlevel = (float) req["audiodBlevel"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
#ifdef AUDIO_DB_SUPPORT
   ret = dsSetAudioDB((intptr_t)paramhandle, audiodBlevel);
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_SetAudioDB is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDB -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_SetAudioDB call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioDB call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDB -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioDB call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioDB call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioDB -->Exit\n");
    }
	return;
}

/***********************************************************************************************
 *Function name : DSHal_GetAudioLevel
 *Description    : This function is used to get the current audio volume level of an audio port
 ***********************************************************************************************/
void DSHalAgent::DSHal_GetAudioLevel(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioLevel --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	float fAudiolevel = 0.0;
    int paramhandle = 0;
	
	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioLevel handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsGetAudioLevel((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsGetAudioLevel((intptr_t)paramhandle, &fAudiolevel);
	}
	
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Current audio volume level : %f", fAudiolevel);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_GetAudioLevel call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioLevel -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "DSHal_GetAudioLevel call failed "+error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_GetAudioLevel call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_GetAudioLevel -->Exit\n");
    }	
	return;
}

/*****************************************************************************************
 *Function name : DSHal_SetAudioLevel
 *Description    : This function is used to  Sets the audio volume level of an audio port
 *****************************************************************************************/
void DSHalAgent::DSHal_SetAudioLevel(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioLevel--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	float audiolevel = (float) req["audiolevel"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetAudioLevel((intptr_t)paramhandle, audiolevel);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_SetAudioLevel call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioLevel call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioLevel -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioLevel call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioLevel call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioLevel -->Exit\n");
    }
	return;
}

/*****************************************************************************************************
 *Function name : DSHal_IsAudioLoopThru
 *Description    : This function is to used to check if the audio port is configured for loop-through
 *****************************************************************************************************/
void DSHalAgent::DSHal_IsAudioLoopThru(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioLoopThru --->Entry\n");
	char details[100];
	int paramhandle = 0;
    dsError_t ret = dsERR_NONE;
    bool enabled = 0;

	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioLoopThru handle %d %d",paramhandle, apHandle );
#ifdef AUDIO_LOOP_THRU_SUPPORT
	if (Isnullparamcheck)
	{
		ret = dsIsAudioLoopThru((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsIsAudioLoopThru((intptr_t)paramhandle, &enabled);
	}
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_IsAudioLoopThru is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioLoopThru -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Configured state of audio port for loop-through is : %d", enabled);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioLoopThru call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioLoopThru -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "Audio loop-through check call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioLoopThru call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioLoopThru -->Exit\n");
    }
	return;
}

/*****************************************************************************************
 *Function name : DSHal_EnableLoopThru
 *Description    : This function is used to  Sets the audio volume level of an audio port
 *****************************************************************************************/
void DSHalAgent::DSHal_EnableLoopThru(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLoopThru--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	int enableLoopThru = (int) req["enableLoopThru"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
#ifdef AUDIO_LOOP_THRU_SUPPORT	
    ret = dsEnableLoopThru((intptr_t)paramhandle, (bool)enableLoopThru);
#else
    response["result"] = "FAILURE";
    response["details"] = "DSHal_EnableLoopThru is not supported by DUT";
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLoopThru -->Exit\n");
    return;
#endif

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_EnableLoopThru call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_EnableLoopThru call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLoopThru -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "EnableLoopThru call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_EnableLoopThru call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_EnableLoopThru -->Exit\n");
    }
	return;
}

/**************************************************************************************************
 *Function name : DSHal_IsAudioOutConnected
 *Description    : This function is to used to check  if the audio output port is connected or not
 **************************************************************************************************/
void DSHalAgent::DSHal_IsAudioOutConnected(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioOutConnected --->Entry\n");
	char details[100];
    dsError_t ret = dsERR_NONE;
	int paramhandle = 0;
    bool Isconnected = 0;

	int Isnullparamcheck = (int) req["Isnullparamcheck"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioOutConnected handle %d %d",paramhandle, apHandle );
	if (Isnullparamcheck)
	{
		ret = dsAudioOutIsConnected((intptr_t)paramhandle, NULL);
	}
	else
	{
		ret = dsAudioOutIsConnected((intptr_t)paramhandle, &Isconnected);
	}

    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
		sprintf(details, "Audio port connected state : %d", Isconnected);
        response["details"] = details;
        DEBUG_PRINT(DEBUG_LOG, "DSHal_IsAudioOutConnected call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioOutConnected -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "dsAudioOutIsConnected call getting failed due to "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_IsAudioOutConnected call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_IsAudioOutConnected -->Exit\n");
    }
	return;
}

/***********************************************************************************************************
 *Function name : DSHal_SetAudioMixerLevels
 *Description    : This function is used to  Sets the Mixer Volume level of sink device for the given input
 ***********************************************************************************************************/
void DSHalAgent::DSHal_SetAudioMixerLevels(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMixerLevels--->Entry\n");
	dsError_t ret = dsERR_NONE;
	int paramhandle = 0;

	dsAudioInput_t aInput = (dsAudioInput_t) req["AudioInput"].asInt();
	int audiolevel = (int) req["audiolevel"].asInt();
	int IsHandleInvalid = (int) req["IsHandleInvalid"].asInt();
	if (IsHandleInvalid)
	{
		paramhandle = (int) req["paramhandle"].asInt();
	}
	else
	{
		paramhandle = (int)apHandle;
	}
	
    ret = dsSetAudioMixerLevels((intptr_t)paramhandle, aInput, audiolevel);
    if (ret == dsERR_NONE)
    {
        response["result"] = "SUCCESS";
        response["details"] = "DSHal_SetAudioMixerLevels call success";
        DEBUG_PRINT(DEBUG_LOG, "DSHal_SetAudioMixerLevels call is SUCCESS");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMixerLevels -->Exit\n");
    }
    else
    {
		checkERROR(ret,&error);
        response["result"] = "FAILURE";
        response["details"] = "SetAudioMixerLevels call failed "+ error;
        DEBUG_PRINT(DEBUG_ERROR, "DSHal_SetAudioMixerLevels call is FAILURE");
        DEBUG_PRINT(DEBUG_TRACE, "DSHal_SetAudioMixerLevels -->Exit\n");
    }
	return;
}

/**************************************************************************
Function Name   : cleanup

Arguments       : NULL

Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool DSHalAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Exit\n");

    return TEST_SUCCESS;
}

/**************************************************************************
Function Name : DestroyObject

Arguments     : Input argument is DSHalAgent Object

Description   : This function will be used to destory the DSHalAgent object.
**************************************************************************/
extern "C" void DestroyObject(DSHalAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying DSHal Agent object\n");
        delete stubobj;
}
