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
#include "downloadUtil.h"
#include "rdkv_cdl_log_wrapper.h"

void *doCurlInit(void)
{
    CURL *curl = NULL;
    printf("\nDummy implementation\n");
    return (void *)curl;
}
void doStopDownload(void *curl)
{
    printf("\nDummy implementation\n");
}
int doInteruptDwnl(void *in_curl, unsigned int max_dwnl_speed)
{
    printf("\nDummy implementation\n");
    return 1;
}
unsigned int doGetDwnlBytes(void *in_curl)
{
    unsigned int bytes = 0;
    printf("\nDummy implementation\n");
    return bytes;
}
int doCurlPutRequest(void *in_curl, FileDwnl_t *pfile_dwnl, char *jsonrpc_auth_token, int *out_httpCode)
{
    printf("\nDummy implementation\n");
    return -1;
}
int getJsonRpcData(void *in_curl, FileDwnl_t *pfile_dwnl, char *jsonrpc_auth_token, int *out_httpCode )
{
    printf("\nDummy implementation\n");
    return -1;
}
int doHttpFileDownload(void *in_curl, FileDwnl_t *pfile_dwnl, MtlsAuth_t *auth, unsigned int max_dwnl_speed, char *dnl_start_pos, int *out_httpCode )
{
    printf("\nDummy implementation\n");
    return -1;
}

int doAuthHttpFileDownload(void *in_curl, FileDwnl_t *pfile_dwnl, int *out_httpCode )
{
    printf("\nDummy implementation\n");
    return -1;
}
