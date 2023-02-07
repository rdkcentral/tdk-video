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
echo -e "Executing script : System_Device_Details_Check"
echo -e "======================================="
echo -e "Test Execution Name is: System_Device_Details_Check"

#Check if wifi is supported
IS_WIFI_SUPPORTED=$(cat /etc/device.properties | grep -q WIFI_SUPPORT=true)
if $IS_WIFI_SUPPORTED
then
  echo -e "Device supports wifi\n"
else
  echo -e "Device does not have wifi Support\n"
fi

#Check if Bluetooth is supported
IS_BLUETOOTH_SUPPORTED=$(cat /etc/device.properties | grep -q BLUETOOTH_ENABLED=true)
if $IS_BLUETOOTH_SUPPORTED
then
  echo -e "Device supports Bluetooth\n"
else
  echo -e "Device does not have Bluetooth Support\n"
fi

FIELDPARAMETER=()

#Check if box_IP is in expected format
BOX_IP=$(cat /tmp/.deviceDetails.cache |grep boxIP| cut -d "=" -f2)
FORMAT=$(ifconfig| grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'| grep -v '[127.0|172.17].0.1')
if [[ $BOX_IP == $FORMAT  ]]; then
  echo -e "Verification of box_IP is SUCCESS \nValue =${BOX_IP}\n"
else
  echo -e "Verification of box_IP is FAILURE \nValue =${BOX_IP}\n"
  FIELDPARAMETER+=$(cat /tmp/.deviceDetails.cache |grep boxIP)
fi

VALIDATE_VALUE()
{
  PARAMETER=$1
  #Check if $PARAMETER is in expected format
  echo "Executing ExecuteCommand...."
  RESPONSE=$(cat /tmp/.deviceDetails.cache)
  RESULT=$(echo "$RESPONSE" | grep "$PARAMETER")
  regex=""
  if [ $PARAMETER == "bluetooth_mac" ]; then
    if $IS_BLUETOOTH_SUPPORTED
    then
      regex='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]){2}$'
    else
      regex='^$'
    fi
  elif [ $PARAMETER == "boxip" ]; then
    regex='^([0-9a-f]*[:-])+([0-9a-f])*$'
  elif [ $PARAMETER == "build_type" ]; then
    regex='VBN'
  elif [ $PARAMETER == "estb_mac" ]; then
    regex='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]){2}$'
  elif [ $PARAMETER == "eth_mac" ]; then
    regex='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]){2}$'
  elif [ $PARAMETER == "imageVersion" ]; then
    regex=$(cat /tmp/.deviceDetails.cache | grep imageVersion | cut -d "=" -f2)
  elif [ $PARAMETER == "model_number" ]; then
    regex='[A-Z0-9]*'
  elif [ $PARAMETER == "rf4ce_mac" ]; then
    regex='^([0-9A-Fa-f]{2}[:-]){7}([0-9A-Fa-f]){2}$'
  elif [ $PARAMETER == "wifi_mac" ]; then
    if $IS_WIFI_SUPPORTED
    then
      regex='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]){2}$'
    else
      regex='^$'
    fi
  fi

  VALUE=$(cat /tmp/.deviceDetails.cache | grep $PARAMETER  | cut -d "=" -f2)
  if [[ $VALUE =~ $regex ]]; then
    echo -e "Verification of $PARAMETER is SUCCESS \nValue =${VALUE}\n"
  else
    if [[ $PARAMETER == "wifi_mac" && ! $IS_WIFI_SUPPORTED && ! -z $VALUE  ]]; then
      echo "wifi_mac should be empty for this device(WIFI NOT SUPPORTED)"
    elif [[ $PARAMETER == "bluetooth_mac" && ! $IS_BLUETOOTH_SUPPORTED && ! -z $VALUE ]]; then
      echo "bluetooth_mac should be empty for this device(Bluetooth NOT SUPPORTED)"
    fi
    echo -e "Verification of $PARAMETER is FAILED Value =${VALUE}\n"
    FIELDPARAMETER+=( "$PARAMETER=$VALUE" )
  fi
}


#Check if bluetooth_mac is in expected format
VALIDATE_VALUE "bluetooth_mac"
#Check if build_type is in expected format
VALIDATE_VALUE "build_type"
#Check if estb_mac is in expected format
VALIDATE_VALUE "estb_mac"
#Check if eth_mac is in expected format
VALIDATE_VALUE "eth_mac"
#Check if imageVersion is in expected format
VALIDATE_VALUE "imageVersion"
#Check if model_number is in expected format
VALIDATE_VALUE "model_number"
#Check if rf4ce_mac is in expected format
VALIDATE_VALUE "rf4ce_mac"
#Check if wifi_mac is in expected format
VALIDATE_VALUE "wifi_mac"

#Printing Failed Parameters
if [[ ${#FIELDPARAMETER[@]} -gt 0 ]];then
  echo "FAILURE:FailedParameters:{"${FIELDPARAMETER[*]}"}" | sed 's/[[:blank:]]/,/g'
else
  echo "SUCCESS:All parameters verified"
fi







