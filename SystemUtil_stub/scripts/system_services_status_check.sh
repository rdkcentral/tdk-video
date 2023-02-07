#!/bin/sh
##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
date

echo -e "Executing script : System_Services_Status_Check"
echo -e "======================================="
echo -e "Test Execution Name is: System_Services_Status_Check"

echo "Executing ExecuteCommand..."
echo "Processing..."
echo "List of the Systemd Services in Device"
Failed_Services()
{
RESPONSE=$(systemctl -a --state=failed,activating | grep 'failed\|activating')
echo "$RESPONSE"
}
Failed_Services

if [[ $RESPONSE ]];then 
	echo "SUCCESS:A few systemd services are in failed or activating state"
else
	echo "FAILURE:No systemd services are in failed or activating state"
fi

