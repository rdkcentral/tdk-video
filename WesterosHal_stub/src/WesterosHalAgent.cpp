/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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

#include "WesterosHalAgent.h"
static WstGLCtx *ctx = 0;
static userData *data = 0;

/***************************************************************************
 *Function name : setEnvironmentVariablesFromFile
 *Description   : setEnvironmentVariablesFromFile will setup the environment
 *                from the platform specific TDK.env file
 *
 *****************************************************************************/
void setEnvironmentVariablesFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file)
    {
	DEBUG_PRINT(DEBUG_TRACE,"Error: Could not open file %s", filename.c_str());
        return;
    }
    std::string line;
    while (std::getline(file, line))
    {
        // Ignore empty lines
        if (line.empty())
	    continue;
        // Check if the line starts with "export "
        if (line.find("export ") == 0)
	{
            std::string var = line.substr(7);  // Remove "export " prefix
            size_t eq_pos = var.find('=');
            if (eq_pos != std::string::npos)
	    {
                std::string key = var.substr(0, eq_pos);
                std::string value = var.substr(eq_pos + 1);
                setenv(key.c_str(), value.c_str(), 1); // Overwrite if exists
		DEBUG_PRINT(DEBUG_TRACE,"Set: %s=%s\n", key.c_str(), value.c_str());
            }
        }
    }
    file.close();
}

/***************************************************************************
 *Function name : startWesteros
 *Description   : startWesteros will start the westeros renderer by reading
 *                the platform specific renderer commands from TDK.env file
 *
 *****************************************************************************/
bool startWesteros()
{
    ifstream infile(TDK_ENV_FILE);
    if (!infile) {
        DEBUG_PRINT(DEBUG_TRACE,"\nCould not open %s\n",TDK_ENV_FILE);
        return false;
    }

    std::string command = std::string("source ") + TDK_ENV_FILE;
    if (!infile) {
        printf("\nCould not open the file!\n");
        return false;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("\nFork failed!\n");
	return false;
    }

    if (pid == 0)
    {
        // In the child process
	// Execute the command
	// Use "/bin/sh -c" to run the command in a shell
	printf("\nInitializing westeros\n");
	execl("/bin/sh", "sh", "-c", command.c_str(), (char*) nullptr);
	return true;
    }

    // In the parent process
    // Wait for westeros to start
    sleep(2);
    return true;
}

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/
std::string WesterosHalAgent::testmodulepre_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal testmodule pre_requisites --> Entry\n");
    bool start_westeros_result = false;
    if (getenv ("START_WESTEROS") != NULL)
    {
        start_westeros_result = startWesteros();
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal testmodule pre_requisites --> Exit\n");
    if (start_westeros_result)
    {
	setEnvironmentVariablesFromFile(std::string(TDK_ENV_FILE));
        return "SUCCESS";
    }
    else
    {
        return "FAILURE";
    }
}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Description   : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool WesterosHalAgent::testmodulepost_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal testmodule post_requisites --> Entry\n");
    int system_command_result = 1;
    if (getenv ("START_WESTEROS") != NULL)
    {
        DEBUG_PRINT(DEBUG_TRACE, "Killing westeros \n");
        char kill_command[30];
        sprintf(kill_command,"kill -9 `pidof westeros`");
        system_command_result = system(kill_command);
        if ( system_command_result == 0 )
        {
             DEBUG_PRINT(DEBUG_TRACE, "Successfully killed westeros\n");
        }
        else
        {
             DEBUG_PRINT(DEBUG_TRACE, "Unable to kill westeros\n");
        }
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal testmodule post_requisites --> Exit\n");
    if (!system_command_result)
    {
        return TEST_SUCCESS;
    }
    else
    {
        return TEST_FAILURE;
    }
}

/**************************************************************************
Function Name   : CreateObject
Arguments       : NULL
Description     : This function is used to create a new object of the class "WesterosHalAgent".
**************************************************************************/
extern "C" WesterosHalAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new WesterosHalAgent(ptrtcpServer);
}

/***************************************************************************
 *Function name : initialize
 *Description   : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/
bool WesterosHalAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "WesterosHal Initialization Entry\n");
    DEBUG_PRINT(DEBUG_TRACE, "Westeros init ....\n");
    ctx = WstGLInit();
    data = (userData*)calloc( 1, sizeof(userData) );
    if (ctx){
        DEBUG_PRINT(DEBUG_TRACE, "WstGLInit call success\n");
	DEBUG_PRINT (DEBUG_TRACE, "WesterosHal Initialization Exit\n");
        return "SUCCESS";
    }
    else{
        DEBUG_PRINT(DEBUG_TRACE, "WstGLInit call failed\n");
	DEBUG_PRINT (DEBUG_TRACE, "WesterosHal Initialization Exit\n");
        return "FAILURE";
    }
}

/**********************************************************************************
 *Function name : displaySizeCallback
 *Description   : Callback function to be triggered when resolution size is changed
 **********************************************************************************/
void displaySizeCallback( void *userdata, int width, int height )
{
   DEBUG_PRINT(DEBUG_TRACE, "\n\n================ Entered displaySizeCallback ===============\n");
   DEBUG_PRINT(DEBUG_TRACE, "\nReceived resolution size width = %d,  height = %d\n",width, height);

   if ( (data->wDisplayWidth != width) || (data->wDisplayHeight != height) )
   {
      DEBUG_PRINT(DEBUG_TRACE, "\nDisplay size changed: %dx%d\n", width, height);
      data->wDisplayWidth= width;
      data->wDisplayHeight= height;
   }
   DEBUG_PRINT(DEBUG_TRACE, "\n\n================ Exited displaySizeCallback ===============\n");
}

/**********************************************************************************
 *Function name : GetCallBackData
 *Description   : Returns the userData to cross verify the callback
 **********************************************************************************/
void WesterosHalAgent::WesterosHal_GetCallBackData(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetCallBackData --->Entry\n");
    memset(&details[0], 0, sizeof(details));
    sprintf(details,"width=%d height=%d", data->wDisplayWidth, data->wDisplayHeight);
    response["details"] = details;
    response["result"]= "SUCCESS";
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetCallBackData --->Exit\n");
    return;
}


/***************************************************************************
 *Function name  : WesterosHal_CreateNativeWindow
 *Description    : This function is to create a native window
 *****************************************************************************/
void WesterosHalAgent::WesterosHal_CreateNativeWindow(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_CreateNativeWindow --->Entry\n");
    memset(&details[0], 0, sizeof(details));
    if((&req["width"]==NULL) || (&req["height"]==NULL))
    {
        response["result"] = "FAILURE";
        response["details"] = "Invalid Height or Width of Display";
        return;
    }

    int width = req["width"].asInt();
    int height = req["height"].asInt();

    DEBUG_PRINT(DEBUG_TRACE, "\nCreate NativeWindow for %dx%d \n", width, height);
    nativeWindow = WstGLCreateNativeWindow( ctx, 0, 0, width, height );

    if (nativeWindow)
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nCreation of Native Window is successfull\n");
	sprintf(details,"Creation of Native Window is successfull");
	response["details"] = details;
	response["result"]= "SUCCESS";
    }
    else
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nCreation of Native Window is failedn");
        sprintf(details,"Creation of Native Window is failed");
        response["details"] = details;
        response["result"]= "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_CreateNativeWindow --->Exit\n");
    return;
}

/***************************************************************************
 *Function name  : WesterosHal_DestroyNativeWindow
 *Description    : This function is to destroy the native window
 *****************************************************************************/
void WesterosHalAgent::WesterosHal_DestroyNativeWindow(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_DestroyNativeWindow --->Entry\n");
    memset(&details[0], 0, sizeof(details));

    try
    {
        WstGLDestroyNativeWindow( ctx, (void*)nativeWindow );
    }
    catch (...)
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nDestruction of Native Window is failedn");
        sprintf(details,"Destruction of Native Window is failed");
        response["details"] = details;
        response["result"]= "FAILURE";
	DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_DestroyNativeWindow --->Exit\n");
	return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "\nDestruction of Native Window is successfull\n");
    sprintf(details,"Destruction of Native Window is successfull");
    response["details"] = details;
    response["result"]= "SUCCESS";
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_DestroyNativeWindow --->Exit\n");
    return;
}

/***************************************************************************
 *Function name  : WesterosHal_GetDisplayInfo
 *Description    : This function returns the display info of the native window
 *****************************************************************************/
void WesterosHalAgent::WesterosHal_GetDisplayInfo(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetDisplayInfo --->Entry\n");
    memset(&details[0], 0, sizeof(details));

    if( WstGLGetDisplayInfo( ctx, &displayInfo ) )
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nGetDisplayInfo successfull width=%d height=%d \n", displayInfo.width, displayInfo.height);
        sprintf(details,"width=%d height=%d", displayInfo.width, displayInfo.height);
        response["details"] = details;
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nGetDisplayInfo  is failed\n");
        sprintf(details,"GetDisplayInfo is failed");
        response["details"] = details;
        response["result"]= "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetDisplayInfo --->Exit\n");
    return;
}

/**********************************************************************************
 *Function name  : WesterosHal_GetDisplaySafeArea
 *Description    : This function returns the display safe area of the native window
 **********************************************************************************/
void WesterosHalAgent::WesterosHal_GetDisplaySafeArea(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetDisplaySafeArea --->Entry\n");
    memset(&details[0], 0, sizeof(details));
    int x,y,w,h;

    if( WstGLGetDisplaySafeArea( ctx, &x, &y, &w, &h ) )
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nGetDisplaySafeArea successfull x=%d y=%d w=%d h=%d \n", x, y, w, h);
        sprintf(details,"x=%d y=%d w=%d h=%d", x, y, w, h);
        response["details"] = details;
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nGetDisplaySafeArea is failed\n");
        sprintf(details,"GetDisplaySafeArea is failed");
        response["details"] = details;
        response["result"]= "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_GetDisplaySafeArea --->Exit\n");
    return;
}

/********************************************************************************************************
 *Function name  : WesterosHal_AddDisplaySizeListener
 *Description    : This function registers a callback function to actively listen for display size change
 ********************************************************************************************************/
void WesterosHalAgent::WesterosHal_AddDisplaySizeListener(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_AddDisplaySizeListener --->Entry\n");
    memset(&details[0], 0, sizeof(details));

    if( WstGLAddDisplaySizeListener( ctx, data, displaySizeCallback) )
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nAddDisplaySizeListener successfull \n");
        sprintf(details,"AddDisplaySizeListener successfull");
        response["details"] = details;
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nAddDisplaySizeListener failed\n");
        sprintf(details,"AddDisplaySizeListener failed");
        response["details"] = details;
        response["result"]= "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_AddDisplaySizeListener --->Exit\n");
    return;

}

/*****************************************************************
 *Function name  : WesterosHal_RemoveDisplaySizeListener
 *Description    : This function deregisters the callback function
 *****************************************************************/
void WesterosHalAgent::WesterosHal_RemoveDisplaySizeListener(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_RemoveDisplaySizeListener --->Entry\n");
    memset(&details[0], 0, sizeof(details));

    if( WstGLRemoveDisplaySizeListener( ctx, displaySizeCallback) )
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nRemoveDisplaySizeListener successfull \n");
        sprintf(details,"RemoveDisplaySizeListener successfull");
        response["details"] = details;
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nRemoveDisplaySizeListener failed\n");
        sprintf(details,"RemoveDisplaySizeListener failed");
        response["details"] = details;
        response["result"]= "FAILURE";
    }
    DEBUG_PRINT(DEBUG_TRACE, "WesterosHal_RemoveDisplaySizeListener --->Exit\n");
    return;

}

/**************************************************************************
Function Name   : cleanup
Arguments       : NULL
Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool WesterosHalAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    WstGLTerm( ctx );
    DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Exit\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name : DestroyObject
Arguments     : Input argument is WesterosHalAgent Object
Description   : This function will be used to destory the DSHalAgent object.
**************************************************************************/
extern "C" void DestroyObject(WesterosHalAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying WesterosHal Agent object\n");
        delete stubobj;
}
