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

#include "hdmi_cec_driver.h"
#include <stdio.h>

int HdmiCecOpen(int *handle)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecClose(int handle)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecSetLogicalAddress(int handle, int *logicalAddresses, int num)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecAddLogicalAddress(int handle, int logicalAddresses)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecRemoveLogicalAddress(int handle, int logicalAddresses)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecGetLogicalAddress(int handle, int *logicalAddress)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

void HdmiCecGetPhysicalAddress(int handle,unsigned int *physicalAddress)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
}

int HdmiCecSetRxCallback(int handle, HdmiCecRxCallback_t cbfunc, void *data)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecSetTxCallback(int handle, HdmiCecTxCallback_t cbfunc, void *data)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecTx(int handle, const unsigned char *buf, int len, int *result)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

int HdmiCecTxAsync(int handle, const unsigned char *buf, int len)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 1;
}

