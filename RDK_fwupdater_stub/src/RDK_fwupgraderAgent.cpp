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

#include "RDK_fwupgradeAgent.h"
jmp_buf jumpBuffer;

/***************************************************************************
 *Function name : testmodulepre_requisites
 *Description   : testmodulepre_requisites will be used for setting the
 *                pre-requisites that are necessary for this component
 *
 *****************************************************************************/
std::string RDK_fwupgradeAgent::testmodulepre_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater testmodule pre_requisites --> Entry\n");
    log_init();
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater testmodule pre_requisites --> Exit\n");
    return "SUCCESS";
}

/***************************************************************************
 *Function name : testmodulepost_requisites
 *Description   : testmodulepost_requisites will be used for resetting the
 *                pre-requisites that are set
 *
 *****************************************************************************/
bool RDK_fwupgradeAgent::testmodulepost_requisites()
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater testmodule post_requisites --> Entry\n");
    log_exit();
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater testmodule post_requisites --> Exit\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name   : CreateObject
Arguments       : NULL
Description     : This function is used to create a new object of the class "RDK_fwupgradeAgent".
**************************************************************************/
extern "C" RDK_fwupgradeAgent* CreateObject(TcpSocketServer &ptrtcpServer)
{
        return new RDK_fwupgradeAgent(ptrtcpServer);
}

/***************************************************************************
 *Function name : initialize
 *Description   : Initialize Function will be used for registering the wrapper method
 *                with the agent so that wrapper functions will be used in the
 *                script
 *****************************************************************************/
bool RDK_fwupgradeAgent::initialize(IN const char* szVersion)
{
    DEBUG_PRINT (DEBUG_TRACE, "rdkfwupdater Initialization Entry\n");
    DEBUG_PRINT (DEBUG_TRACE, "rdkfwupdater Initialization Exit\n");
    return true;
}

/*********************************************************************************************
 *Function name : storeFileNames
 *Description   : storeFileNames will be used for populating a char array by reading the files
 *                which are stored in metaDataFileList_st.
 *********************************************************************************************/
static void storeFileNames(metaDataFileList_st *head, char *resultBuffer, size_t bufferSize)
{
    metaDataFileList_st *current = head;
    size_t usedLength = 0; // Track used buffer length
    while (current != NULL) {
        // Check if there is enough space in the buffer for the next file name
        size_t nameLength = strlen(current->fileName);
        if (usedLength + nameLength + 2 >= bufferSize) { // +2 for newline and null terminator
            DEBUG_PRINT(DEBUG_TRACE, "Buffer size exceeded!\n");
            break;
        }
        // Append the file name and a newline to the result buffer
        sprintf(resultBuffer + usedLength, "%s\n", current->fileName);
        usedLength += nameLength + 1; // Update the used length (+1 for the newline)
        current = current->next; // Move to the next node
    }
}

/*********************************************************************************************
 *Function name : createMetaDataList
 *Description   : createMetaDataList will be used for creating metaDataFileList_st by reading
 *                from char array.
 *********************************************************************************************/
metaDataFileList_st* createMetaDataList(const char* str)
{
    // Initialize an empty list
    metaDataFileList_st* head = nullptr;
    metaDataFileList_st* current = nullptr;
    // Use a stringstream to tokenize the input string based on spaces
    std::stringstream ss(str);
    std::string temp;
    while (ss >> temp) {
        // Create a new node for each element
        metaDataFileList_st* newNode = new metaDataFileList_st;
        // Copy the token (file name) into the fileName field of the node
        std::strncpy(newNode->fileName, temp.c_str(), sizeof(newNode->fileName) - 1);
        newNode->fileName[sizeof(newNode->fileName) - 1] = '\0';  // Ensure null-termination
        // Set the next pointer to nullptr initially
        newNode->next = nullptr;
        // If this is the first node, set it as the head
        if (head == nullptr) {
            head = newNode;
        } else {
            // Otherwise, link the new node to the last node in the list
            current->next = newNode;
        }
        // Move current to the new node
        current = newNode;
    }
    return head;
}

/*********************************************************************************************
 *Function name : printMetaDataList
 *Description   : printMetaDataList will be used for printing metaDataFileList_st by navigating
 *                from head node to last node.
 *********************************************************************************************/
void printMetaDataList(metaDataFileList_st* head) 
{
    metaDataFileList_st* current = head;
    if (current == nullptr)
    {
	DEBUG_PRINT(DEBUG_TRACE, "List is empty");
    }
    while (current != nullptr)
    {
        DEBUG_PRINT(DEBUG_TRACE, " %s" ,current->fileName);
        current = current->next;
    }
}

/*********************************************************************************************
 *Function name : signalHandler
 *Description   : signalHandler will be used for handling SIGSEGV.
 *********************************************************************************************/
void signalHandler(int signal)
{
    switch(signal)
    {
       case SIGSEGV:
            DEBUG_PRINT(DEBUG_TRACE, "Observed Segmentation fault during test execution\n");
	    longjmp(jumpBuffer,1);
	    break;
       default :
	    DEBUG_PRINT(DEBUG_TRACE, "Received unknown signal\n");
    }
}

/*************************************************************************************************
 *Function name  : rdkfwupdater_mergeLists
 *Description    : This function merges two metaDataLists.
 *************************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_mergeLists(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_mergeLists --->Entry\n");
    metaDataFileList_st *merged_list = NULL;
    metaDataFileList_st *list1_head = NULL;
    metaDataFileList_st *list2_head = NULL;
    char resultBuffer[512] = {};

    if (req["null_list1"].asInt())
    {
	DEBUG_PRINT(DEBUG_TRACE, "List 1 set as NULL");
    }
    else
    {
	char list1[50] = {};
	strcpy(list1,req["list1"].asCString());
	DEBUG_PRINT(DEBUG_TRACE, "List 1 : %s ",list1);
	list1_head = createMetaDataList(list1);
	DEBUG_PRINT(DEBUG_TRACE, "List 1 obtained as below");
	printMetaDataList(list1_head);
    }

    if (req["null_list2"].asInt())
    {
	DEBUG_PRINT(DEBUG_TRACE, "List 2 set as NULL");
    }
    else
    {
	char list2[50] = {};
	strcpy(list2,req["list2"].asCString());
	DEBUG_PRINT(DEBUG_TRACE, "List 2 : %s ",list2);
	list2_head = createMetaDataList(list2);
        DEBUG_PRINT(DEBUG_TRACE, "List 2 obtained as below");
	printMetaDataList(list2_head);
    }

    merged_list = mergeLists(list1_head, list2_head);
    DEBUG_PRINT(DEBUG_TRACE, "Merged list obtained as below");
    printMetaDataList(merged_list);

    storeFileNames(merged_list, resultBuffer, sizeof(resultBuffer));

    response["result"]= "SUCCESS";
    response["details"] = resultBuffer;
    return;
}


/*************************************************************************************************
 *Function name  : rdkfwupdater_stripinvalidchar
 *Description    : This function truncates a string when a control or space character is detected.
 *************************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_stripinvalidchar(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_stripinvalidchar --->Entry\n");
    char string[50] = {};
    size_t returnValue = 0;
    strcpy(string,req["string"].asCString());

    size_t string_size = req["string_size"].asInt();

    returnValue = stripinvalidchar(string, string_size);

    DEBUG_PRINT(DEBUG_TRACE, "\nreturn Value = %zu\n", returnValue);
    DEBUG_PRINT(DEBUG_TRACE, "\nModified string = %s\n", string);

    if (returnValue)
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nstripinvalidchar was successfull\n");
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nstripinvalidchar failed");
        response["result"]= "FAILURE";
    }

    response["details"] = string;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_stripinvalidchar --->Exit\n");
    return;
}

/*************************************************************************************************
 *Function name  : rdkfwupdater_getJsonRpc
 *Description    : This function truncates a string when a control or space character is detected.
 *************************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_getJsonRpc(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_getJsonRpc --->Entry\n");

    DownloadData DwnLoc;
    DwnLoc.pvOut = NULL;
    DwnLoc.datasize = 0;
    DwnLoc.memsize = 0;

    char post_data4[] = "{\"jsonrpc\":\"2.0\",\"id\":\"42\",\"method\": \"org.rdk.NetworkManager.IsConnectedToInternet\", \"params\" : { \"ipversion\" : \"IPv4\"}}";
    if( MemDLAlloc( &DwnLoc, 1024 ) == 0 )
    {
        if (0 != getJsonRpc(post_data4, &DwnLoc))
        {
	    DEBUG_PRINT(DEBUG_TRACE, "DEBUG getJsonRpc failed");
        }
	else
	{
	    DEBUG_PRINT(DEBUG_TRACE, "DEBUG getJsonRpc success");
	}
        DEBUG_PRINT(DEBUG_TRACE, "DEBUG json response : %s", (char*)DwnLoc.pvOut);
	if( DwnLoc.pvOut != NULL )
        {
            free( DwnLoc.pvOut );
        }
    }   


    char string[BUFFER_SIZE_LONG] = {};
    int returnValue = 1;
    strcpy(string,req["string"].asCString());
    DownloadData data = {};
    data.pvOut = NULL;
    data.datasize = 0;
    data.memsize = 0;

    if (!req["null_param"].asInt())
    {
	 DEBUG_PRINT(DEBUG_TRACE, "Initializing DownloadData");
         void *ptr = malloc( DEFAULT_DATA_SIZE );
         data.pvOut = ptr;
         data.memsize = DEFAULT_DATA_SIZE;
    }

    DEBUG_PRINT(DEBUG_TRACE, "jsonrpc command  : %s", string);
    returnValue = getJsonRpc(string, &data );
    DEBUG_PRINT(DEBUG_TRACE, "returnValue = %d", returnValue);
    DEBUG_PRINT(DEBUG_TRACE, "json response : %s", (char*)data.pvOut);
    if (!returnValue)
    {
        DEBUG_PRINT(DEBUG_TRACE, "getJsonRpc was successfull\n");
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "getJsonRpc failed");
        response["result"]= "FAILURE";
    }

    response["details"] = (char*)data.pvOut;
    if (data.pvOut)
        free(data.pvOut);
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_getJsonRpc --->Exit\n");
    return;
}

/*****************************************************************************************
 *Function name  : rdkfwupdater_getJRPCTokenData
 *Description    : This function is to parse jsonrpc token
 *****************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_getJRPCTokenData(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_getJRPCTokenData --->Entry\n");
    char string[BUFFER_SIZE_LONG] = {};
    int returnValue = 1;
    strcpy(string,req["string"].asCString());
    DEBUG_PRINT(DEBUG_TRACE, "String obtained : %s", string);
    char token[BUFFER_SIZE_LONG] = {};
    unsigned int tokenSize = sizeof(token);

    if (req["null_token"].asInt())
    {
	DEBUG_PRINT(DEBUG_TRACE, "Querying with NULL token\n");
	returnValue = getJRPCTokenData(NULL, string, tokenSize);
    }
    else if (req["null_tokenSize"].asInt())
    {
	DEBUG_PRINT(DEBUG_TRACE, "Querying with NULL tokenSize\n");
	returnValue = getJRPCTokenData(token, string, 0);
    }
    else
    {
	DEBUG_PRINT(DEBUG_TRACE, "Executing getJRPCTokenData\n");
	returnValue = getJRPCTokenData(token, string, tokenSize);
    }

    if (!returnValue)
    {
        DEBUG_PRINT(DEBUG_TRACE, "\ngetJRPCTokenData was successfull\n");
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\ngetJRPCTokenData failed");
        response["result"]= "FAILURE";
    }

    response["details"] = token;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_getJRPCTokenData --->Exit\n");
    return;
}

/*****************************************************************************************
 *Function name  : rdkfwupdater_getMetaDataFile
 *Description    : This function is to obtain the files list in the directory
 *****************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_getMetaDataFile(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_getMetaDataFile --->Entry\n");
    metaDataFileList_st *returnValue = NULL;
    char directory[50] = {};
    strcpy(directory,req["directory"].asCString());
    char resultBuffer[512] = {};
    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = getMetaDataFile(NULL);
         }
         else
         {
             returnValue = getMetaDataFile(directory);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }
    if (returnValue != NULL)
    {
         storeFileNames(returnValue, resultBuffer, sizeof(resultBuffer));
         DEBUG_PRINT(DEBUG_TRACE, "File names : %s", resultBuffer);
         response["result"]= "SUCCESS";
         response["details"] = resultBuffer;
    }
    else
    {
	 response["result"]= "FAILURE";
	 response["details"] = "No packages in directory";
    }
    return;
}

/*****************************************************************************************
 *Function name  : rdkfwupdater_get_system_uptime
 *Description    : This function is to obtain the system uptime
 *****************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_get_system_uptime(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_get_system_uptime --->Entry\n");
    double data = 0;
    bool returnValue = false;
    char details[20] = {};
    if (req["null_param"].asInt())
    {
         returnValue = get_system_uptime(NULL);
    }
    else
    {
	 returnValue = get_system_uptime(&data);
         DEBUG_PRINT(DEBUG_TRACE, "uptime data = %f", data);
         sprintf(details, "%f", data);
    }
    DEBUG_PRINT(DEBUG_TRACE, "returnValue = %d", returnValue);

    if (returnValue)
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nget_system_uptime success");
	response["result"]= "SUCCESS";
        response["details"] = data;
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nget_system_uptime failed");
	response["result"]= "FAILURE";
	response["details"] = "failed to get uptime";
    }
    return;
}

/*****************************************************************************************
 *Function name  : rdkfwupdater_MemDLAlloc
 *Description    : This function is to allocate a memory block and fill in data structure
 *****************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_MemDLAlloc(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_MemDLAlloc --->Entry\n");
    DownloadData data = {};
    int returnValue = 1;
    
    size_t allocateSize = req["allocate_size"].asInt();

    if (req["null_param"].asInt())
    {
	 returnValue = MemDLAlloc(NULL, allocateSize);
    }
    else
    {
         returnValue = MemDLAlloc(&data, allocateSize);
    }

    size_t sizeAllocated = 0;
    if (data.pvOut)
    {
	 sizeAllocated = malloc_usable_size(data.pvOut);
	 /*
	  * Value returned by malloc_usable_size() may be greater than
	  * the requested size of the allocation because of various internal
	  * implementation details.
	  * https://man7.org/linux/man-pages/man3/malloc_usable_size.3.html
	  *
	  * Hence verify if malloc_usable_size() returns greater than or equal to allocated size
	  */
	 DEBUG_PRINT(DEBUG_TRACE, "\nmalloc_usable_size : %zu", sizeAllocated);
	 DEBUG_PRINT(DEBUG_TRACE, "\nmemsize : %zu", data.memsize);

	 if (sizeAllocated < data.memsize)
         {
             DEBUG_PRINT(DEBUG_TRACE, "\nSize allocated and memsize of DownloadData doesn't match\n");
	     free(data.pvOut);
	     response["details"] = "Size allocated and memsize of DownloadData doesn't match";
	     response["result"]= "FAILURE";
	     return;
	 }
    }

    if (!returnValue)
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nMemDLAlloc was successfull\n");
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nMemDLAlloc failed");
        response["result"]= "FAILURE";
    }

    response["details"] = data.memsize;
    free(data.pvOut);
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_MemDLAlloc --->Exit\n");
    return;
}

/***************************************************************************
 *Function name  : rdkfwupdater_makeHttpHttps
 *Description    : This function is to change http into https in a give string
 *****************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_makeHttpHttps(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_makeHttpHttps --->Entry\n");

    char string[50] = {};
    size_t returnValue = 0;
    strcpy(string,req["string"].asCString());

    size_t string_size = req["string_size"].asInt();

    returnValue = makeHttpHttps(string, string_size);

    DEBUG_PRINT(DEBUG_TRACE, "\nreturn Value = %zu\n", returnValue);
    DEBUG_PRINT(DEBUG_TRACE, "\nModified string = %s\n", string);

    if (returnValue)
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nmakeHttpHttps was successfull\n");
        response["result"]= "SUCCESS";
    }
    else
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nmakeHttpHttps failed");
        response["result"]= "FAILURE";
    }

    response["details"] = string;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_makeHttpHttps --->Exit\n");
    return;
}

/***************************************************************************
 *Function name  : rdkfwupdater_RunCommand
 *Description    : This function is to run a command and get output
 *****************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_RunCommand(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_RunCommand --->Entry\n");

    SYSCMD system_cmd = eWpeFrameworkSecurityUtility;

    char command[30] ={};
    size_t returnValue = 0;
    strcpy(command,req["command"].asCString());

    if (command[0] == '\0')
    {
	system_cmd = static_cast<SYSCMD>(-1);
    }

    char *argument = NULL;
    if (strcmp(req["argument"].asCString(), "NULL") == 0)
    {
        argument = NULL;
    }
    else
    {
        argument = strdup(req["argument"].asCString());
    }

    bool result_empty = false;
    result_empty = req["result_empty"].asInt();

    bool result_size_empty = false;
    result_size_empty = req["result_size_empty"].asInt();

    char *result;
    size_t result_size = 256;

    result = (char *)calloc(result_size,sizeof(char));

    if (req["result_size"].asInt())
        result_size = req["result_size"].asInt();

    if (result_size_empty)
	result_size = 0;

    if (result_empty)
    {
	free(result);
	result  = NULL;
    }
	
    if (!strcmp(command,"eWpeFrameworkSecurityUtility"))
    {	    
        DEBUG_PRINT(DEBUG_TRACE, "\nRunning  eWpeFrameworkSecurityUtility\n");
	system_cmd = eWpeFrameworkSecurityUtility;
    }
    else if (!strcmp(command,"eMfrUtil"))
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nRunning  eMfrUtil\n");
        system_cmd = eMfrUtil;
    }
    else if (!strcmp(command,"eMD5Sum"))
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nRunning eMD5Sum\n");
	system_cmd = eMD5Sum;
    }
    else if (!strcmp(command,"eGetModelNum"))
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nRunning eGetModelNum\n");
        system_cmd = eGetModelNum;
    }

    returnValue = RunCommand( system_cmd, argument, result, result_size );
    DEBUG_PRINT(DEBUG_TRACE, "\nreturn Value = %zu\n", returnValue);
    DEBUG_PRINT(DEBUG_TRACE, "\nresult = %s\n",result);

    if (returnValue)
    {
        DEBUG_PRINT(DEBUG_TRACE, "\nRunCommand was successfull\n");
	response["result"]= "SUCCESS";
    }
    else
    {
	DEBUG_PRINT(DEBUG_TRACE, "\nRunCommand failed");
        response["result"]= "FAILURE";
    }

    response["details"] = result;
    free(result);
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_RunCommand --->Exit\n");
    return;
}

/*********************************************************************
 *Function name  : rdkfwupdater_GetServerURL
 *Description    : This function scans a file for a URL
 *********************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetServerURL(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetServerURL --->Entry\n");
    size_t returnValue = 0;
    char filename[40] = { 0 };

    strcpy(filename,req["filename"].asCString());
    size_t buffer_size = req["buffer_size"].asInt();
    char serverurl[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetServerUrlFile(NULL, buffer_size, filename);
         }
         else if (req["filename_null_check"].asInt())
         {
             returnValue = GetServerUrlFile(serverurl, buffer_size, NULL);
         }
         else
         {
             returnValue = GetServerUrlFile(serverurl, buffer_size, filename);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Server URL: %s, Return Value: %d Filename: %s", serverurl, returnValue, filename);
    if (returnValue)
    {
       response["result"]= "SUCCESS";
    }
    else
    {
       response["result"]= "FAILURE";
    }

    response["details"] = serverurl;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetServerURL --->Exit\n");
    return;
}

/**********************************************************************
 *Function name  : rdkfwupdater_GetTimezone
 *Description    : This function returns the timezone for the device.
 **********************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetTimezone(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetTimezone --->Entry\n");
    char cpuarch_string[10] = { 0 };
    size_t returnValue = 0;

    strcpy(cpuarch_string,req["cpuarch"].asCString());
    size_t buffer_size = req["buffer_size"].asInt();

    char strTimezone[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetTimezone(NULL, cpuarch_string, buffer_size);
         }
		 else if (req["cpuarch_null_param_check"].asInt())
		 {
                    returnValue = GetTimezone(strTimezone, NULL, buffer_size);
		 }
         else
         {
             returnValue = GetTimezone(strTimezone, cpuarch_string, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "TimeZone: %s, Return Value: %d cpuarch: %s", strTimezone, returnValue, cpuarch_string);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strTimezone;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetTimezone --->Exit\n");
    return;
}

/**********************************************************************************************
 *Function name  : rdkfwupdater_GetAdditionalFwVerInfo
 *Description    : This function returns the PDRI filename plus Remote Info for the device.
 **********************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetAdditionalFwVerInfo(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetAdditionalFwVerInfo --->Entry\n");
    size_t returnValue = 0;	
    size_t buffer_size = req["buffer_size"].asInt();
    char strFwInfo[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetAdditionalFwVerInfo(NULL, buffer_size);
         }
         else
         {
             returnValue = GetAdditionalFwVerInfo(strFwInfo, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "FW info: %s, Return Value: %d Buffer size: %d", strFwInfo, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }    
    response["details"] = strFwInfo;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetAdditionalFwVerInfo --->Exit\n");
    return;
}

/****************************************************************************
 *Function name  : rdkfwupdater_GetPDRIFileName
 *Description    : This function returns the PDRI for the device.
 ****************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetPDRIFileName(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetPDRIFileName --->Entry\n");
    size_t returnValue = 0;
    size_t buffer_size = req["buffer_size"].asInt();
    char strPDRIFilename[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetPDRIFileName(NULL, buffer_size);
         }
         else
         {
             returnValue = GetPDRIFileName(strPDRIFilename, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "PDRI Filename: %s, Return Value: %d Buffer size: %d", strPDRIFilename, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strPDRIFilename;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetPDRIFileName --->Exit\n");
    return;
}

/************************************************************************************************************
 *Function name  : rdkfwupdater_GetUTCTime
 *Description    : This function returns a formatted UTC device time. Example: Tue Jul 12 21:56:06 UTC 2022
 ************************************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetUTCTime(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetUTCTime --->Entry\n");
    size_t returnValue = 0;
    size_t buffer_size = req["buffer_size"].asInt();
    char strUTCTime[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetUTCTime(NULL, buffer_size);
         }
         else
         {
             returnValue = GetUTCTime(strUTCTime, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "UTC Time: %s, Return Value: %d Buffer size: %d", strUTCTime, returnValue, buffer_size);

    if (returnValue)
    { 
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strUTCTime;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetUTCTime --->Exit\n");
    return;
}

/**************************************************************************
 *Function name  : rdkfwupdater_GetCapabilities
 *Description    : This function returns the device capabilities.
 **************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetCapabilities(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetCapabilities --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strCapabilities[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetCapabilities(NULL, buffer_size);
         }
         else
         {
             returnValue = GetCapabilities(strCapabilities, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Device Capabilities: %s, Return Value: %d Buffer size: %d", strCapabilities, returnValue, buffer_size);

    if (returnValue)
    {	 
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }
    response["details"] = strCapabilities;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetCapabilities --->Exit\n");
    return;
}

/************************************************************************
 *Function name  : rdkfwupdater_GetPartnerId
 *Description    : This function returns the partner ID of the device.
 ************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetPartnerId(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetPartnerId --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strPartnerId[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetPartnerId(NULL, buffer_size);
         }
         else
         {
             returnValue = GetPartnerId(strPartnerId, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Device PartnerID: %s, Return Value: %d Buffer size: %d", strPartnerId, returnValue, buffer_size);

    if (returnValue)
    { 
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strPartnerId;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetPartnerId --->Exit\n");
    return;
}

/**************************************************************************
 *Function name  : rdkfwupdater_GetSerialNum
 *Description    : This function returns the serial number of the device.
 **************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetSerialNum(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetSerialNum --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strSerialNum[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetSerialNum(NULL, buffer_size);
         }
         else
         {
             returnValue = GetSerialNum(strSerialNum, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Serial Number: %s, Return Value: %d Buffer size: %d", strSerialNum, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strSerialNum;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetSerialNum --->Exit\n");
    return;
}

/**************************************************************************
 *Function name  : rdkfwupdater_GetAccountID
 *Description    : This function returns the account ID of the device..
 **************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetAccountID(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetAccountID --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strAccountID[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetAccountID(NULL, buffer_size);
         }
         else
         {
             returnValue = GetAccountID(strAccountID, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Account ID: %s, Return Value: %d Buffer size: %d", strAccountID, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strAccountID;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetAccountID --->Exit\n");
    return;
}

/**************************************************************************
 *Function name  : rdkfwupdater_GetOsClass
 *Description    : This function returns the OsClass of the device.
 **************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetOsClass(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetOsClass --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strOsClass[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetOsClass(NULL, buffer_size);
         }
         else
         {
             returnValue = GetOsClass(strOsClass, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "OsClass: %s, Return Value: %d Buffer size: %d", strOsClass, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strOsClass;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetOsClass --->Exit\n");
    return;
}

/**************************************************************************
 *Function name  : rdkfwupdater_GetModelNum
 *Description    : This function returns the model number of the device.
 **************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetModelNum(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetModelNum --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strModelNum[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetModelNum(NULL, buffer_size);
         }
         else
         {
             returnValue = GetModelNum(strModelNum, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Model no.: %s, Return Value: %d Buffer size: %d", strModelNum, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strModelNum;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetModelNum --->Exit\n");
    return;
}

/*********************************************************************************************
 *Function name  : rdkfwupdater_GetBuildType
 *Description    : This function returns the gets the build type of the device in lowercase.
 *********************************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetBuildType(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetBuildType --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();

    char strBuildType[buffer_size] = {0};
    BUILDTYPE eBuildType = static_cast<BUILDTYPE>(0);

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
            returnValue = GetBuildType(NULL, buffer_size, &eBuildType);
         }
	 else if (req["BuildType_null_param_check"].asInt())
	 {
            returnValue = GetBuildType(strBuildType, buffer_size, NULL);
	 }
         else
         {
            returnValue = GetBuildType(strBuildType, buffer_size, &eBuildType);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "BuildType: %s, Return Value: %d eBuildType: %d", strBuildType, returnValue, eBuildType);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strBuildType;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetBuildType --->Exit\n");
    return;
}

/****************************************************************************
 *Function name  : rdkfwupdater_GetFirmwareVersion
 *Description    : This function returns the firmware version of the device.
 ****************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetFirmwareVersion(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetFirmwareVersion --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strFWVersion[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetFirmwareVersion(NULL, buffer_size);
         }
         else
         {
             returnValue = GetFirmwareVersion(strFWVersion, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Firmware Ver.: %s, Return Value: %d Buffer size: %d", strFWVersion, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strFWVersion;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetFirmwareVersion --->Exit\n");
    return;
}

/****************************************************************************
 *Function name  : rdkfwupdater_GetEstbMac
 *Description    : This function returns the eSTB MAC address of the device.
 ****************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetEstbMac(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetEstbMac --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strEstbMac[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetEstbMac(NULL, buffer_size);
         }
         else
         {
             returnValue = GetEstbMac(strEstbMac, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "STB MAC ADDR.: %s, Return Value: %d Buffer size: %d", strEstbMac, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strEstbMac;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetEstbMac --->Exit\n");
    return;
}

/***********************************************************************
 *Function name  : rdkfwupdater_GetRemoteInfo
 *Description    : This function returns the remote info of the device
 ***********************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetRemoteInfo(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetRemoteInfo  --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strRemoteInfo[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetRemoteInfo(NULL, buffer_size);
         }
         else
         {
             returnValue = GetRemoteInfo(strRemoteInfo, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Remote Info: %s, Return Value: %d Buffer size: %d", strRemoteInfo, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strRemoteInfo;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetRemoteInfo --->Exit\n");
    return;
}

/*******************************************************************************
 *Function name  : rdkfwupdater_GetRemoteVers
 *Description    : This function returns the peripheral versions of the device
 *******************************************************************************/
void RDK_fwupgradeAgent::rdkfwupdater_GetRemoteVers(IN const Json::Value& req, OUT Json::Value& response)
{
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetRemoteVers  --->Entry\n");
    size_t returnValue = 0;

    size_t buffer_size = req["buffer_size"].asInt();
    char strRemoteVers[buffer_size] = {0};

    signal(SIGSEGV,signalHandler);
    if (setjmp(jumpBuffer) == 0)
    {
         if (req["null_param"].asInt())
         {
             returnValue = GetRemoteVers(NULL, buffer_size);
         }
         else
         {
             returnValue = GetRemoteVers(strRemoteVers, buffer_size);
         }
    }
    else
    {
        DEBUG_PRINT(DEBUG_LOG, "Observed crash during test execution\n");
        response["result"]= "FAILURE";
        response["details"] = "Observed crash during test execution";
        return;
    }

    DEBUG_PRINT(DEBUG_TRACE, "Remote Version Info: %s, Return Value: %d Buffer size: %d", strRemoteVers, returnValue, buffer_size);

    if (returnValue)
    {
        response["result"]= "SUCCESS";
    }
    else
    {
        response["result"]= "FAILURE";
    }

    response["details"] = strRemoteVers;
    DEBUG_PRINT(DEBUG_TRACE, "rdkfwupdater_GetRemoteVers --->Exit\n");
    return;
}

/**************************************************************************
Function Name   : cleanup
Arguments       : NULL
Description     : This function will be used to the close things cleanly.
 **************************************************************************/
bool RDK_fwupgradeAgent::cleanup(IN const char* szVersion)
{
    DEBUG_PRINT(DEBUG_TRACE, "cleaning up\n");
    DEBUG_PRINT(DEBUG_TRACE,"\ncleanup ---->Exit\n");
    return TEST_SUCCESS;
}

/**************************************************************************
Function Name : DestroyObject
Arguments     : Input argument is RDK_fwupgradeAgent Object
Description   : This function will be used to destory the RDK_fwupgradeAgent object.
**************************************************************************/
extern "C" void DestroyObject(RDK_fwupgradeAgent *stubobj)
{
        DEBUG_PRINT(DEBUG_LOG, "Destroying rdkfwupdater Agent object\n");
        delete stubobj;
}
