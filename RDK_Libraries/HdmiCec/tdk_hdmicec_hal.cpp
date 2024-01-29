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

HDMI_CEC_STATUS HdmiCecOpen(int *handle)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecClose(int handle)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecSetLogicalAddress(int handle, int *logicalAddresses, int num)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecAddLogicalAddress(int handle, int logicalAddresses)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecRemoveLogicalAddress(int handle, int logicalAddresses)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecGetLogicalAddress(int handle, int devType,  int *logicalAddress)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecGetPhysicalAddress(int handle,unsigned int *physicalAddress)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecSetRxCallback(int handle, HdmiCecRxCallback_t cbfunc, void *data)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecSetTxCallback(int handle, HdmiCecTxCallback_t cbfunc, void *data)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecTx(int handle, const unsigned char *buf, int len, int *result)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

HDMI_CEC_STATUS HdmiCecTxAsync(int handle, const unsigned char *buf, int len)
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return HDMI_CEC_IO_SENT_FAILED;
}

