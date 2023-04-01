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
echo -e "Executing script : System_Sanity_Check"
echo -e "======================================="
echo -e "Test Execution Name is: System_Sanity_Check"

# Read the config file
config_file="$(dirname "$0")/sanity_check.config"
if [ ! -f $config_file ];then
echo "Please place the config file $config_file in the path and re-execute script"
exit 1
fi

source $config_file


# Checking if Dut is connected to Ethernet or Wifi and have Internet Connectivity
ETHERNET_IP=$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')
if [ -z "$ETHERNET_IP" ];then
  echo "Ethernet is not connected"
else
  echo "Ethernet is connected"
  echo "ip address : $ETHERNET_IP"
    ping -c 3 8.8.8.8
    if [ "$?" -eq "0" ]; then
            echo "Ethernet interface is connected and has internet connectivity"
    else
            echo "Ethernet interface is connected but doesn't have internet connectivity"
    fi
fi

WIFI_IP=$(ifconfig wlan0 | awk '/inet addr/{print substr($2,6)}')
if [[ -z "$WIFI_IP" ]];then
  echo "wifi is not connected"
else
  echo "wifi is connected"
  echo "ip address : $WIFI_IP"
    ping -c 3 8.8.8.8
    if [ "$?" = "0"  ]; then
            echo "WIFI interface is connected and has internet connectivity"
    else
            echo "WIFI interface is connected but doesn't have internet connectivity"
    fi
fi

echo "---------------------------------------------------------------------------------------"

#Validate SSH Connectivity of DUT through netcat command

if [[ -n "$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')" ]]; then
  ip_address="$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')"
elif [[ -n "$(ifconfig wlan0 | awk '/inet addr/{print substr($2,6)}')" ]]; then
  ip_address="$(ifconfig wlan0 | awk '/inet addr/{print substr($2,6)}')"
fi

IP_ADDRESS="$ip_address"
#SSH port number
PORT_NUMBER=22

if echo | timeout 5 nc -w 3 "${IP_ADDRESS}" "${PORT_NUMBER}" > /dev/null 2>&1; then
    echo  "SSH is running on IP_ADDRESS:${IP_ADDRESS} PORT_NUMBER:${PORT_NUMBER}"
else
    echo  "SSH is not running on IP_ADDRESS:${IP_ADDRESS} PORT_NUMBER:${PORT_NUMBER}"
fi

echo "---------------------------------------------------------------------------------------"

#Checking Device Standby state of DUT

if [[ $(systemctl status systemd-suspend.service | grep Active) == *"active (standby)"* ]]; then
  echo "DUT is currently in standby mode."
else
  echo "DUT is not in standby mode."
fi

echo "---------------------------------------------------------------------------------------"

#Validate Status of the Essential Services required for framework running in DUT

running_services=()
failed_services=()

for service in "${services[@]}"
do
    if systemctl is-active --quiet "$service"; then
        running_services+=("$service")
    else
        failed_services+=("$service")
    fi
done

echo "Running_services:{"${running_services[@]}"}" | sed 's/[[:blank:]]/,/g'

#Printing Failed Services
if [[ ${#failed_services[@]} -gt 0 ]];then
  echo "FAILURE:FAILED_SERVICES:{"${failed_services[*]}"}" | sed 's/[[:blank:]]/,/g'
else
  echo "SUCCESS:All ESSENTIAL SERVICES ARE RUNNING"
fi

echo "---------------------------------------------------------------------------------------"

# Checking if Essential Processes are running in DUT

running_process=()
failed_process=()

for process in "${processes[@]}"
do
    if pgrep "$process" > /dev/null; then
        running_process+=("$process")
    else
        failed_process+=("$process")
    fi
done

echo "Running_processes:{"${running_process[@]}"}" | sed 's/[[:blank:]]/,/g'

#Printing Failed processes
if [[ ${#failed_process[@]} -gt 0 ]];then
  echo "FAILURE:FAILED_PROCESSES:{"${failed_process[*]}"}" | sed 's/[[:blank:]]/,/g'
else
  echo "SUCCESS:All ESSENTIAL PROCESSES ARE RUNNING"
fi

echo "---------------------------------------------------------------------------------------"

# Checking if Required Logfiles are Present in DUT

Files_Present=()
Files_Missing=()

# Checking if each log file exists
for log_file in "${LOG_FILES[@]}"
do
  if [ -f "$log_file" ]; then
    Files_Present+=("$log_file")
  else
    Files_Missing+=("$log_file")
  fi
done

echo "Required_LOG_FILES:{"${Files_Present[@]}"}" | sed 's/[[:blank:]]/,/g'

#Printing Missing LOG_FILES
if [[ ${#Files_Missing[@]} -gt 0 ]];then
  echo "FAILURE:MISSSING_LOGFILES:{"${Files_Missing[@]}"}" | sed 's/[[:blank:]]/,/g'
else
  echo "SUCCESS:All REQUIRED LOG_FILES are Present"
fi

echo "---------------------------------------------------------------------------------------"


