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
#Test Objective     : To display the Storage of directories and it's Partitions in DUT.
#Automation_approch : To display Storage of directories and Partitions.
#Expected Output    : Display the available and used memory of each directories and it's associated partitions.

# Specify the starting directory for the search
starting_directory="/"

# Find all main directories and validate each one and excluding proc since it is a virtual directory
for directory in $(find "$starting_directory"* -maxdepth 0 -type d | grep -v "/proc"); do
  if [ -d "$directory" ]; then
    echo "Validating Directory: $directory & Associated Partition: $partition "
    # Checking the size of the directory
    directory_used=$(du -sh "$directory" | awk '{print $1}')
    directory_available=$(df -hP "$directory" | awk 'NR==2 {print $4}')
    # Checking the partition details of the directory
    partition=$(df -hP "$directory" | awk 'NR==2 {print $1}')
    partition_used=$(df -hP "$directory" | awk 'NR==2 {print $3}')
    partition_available=$(df -hP "$directory" | awk 'NR==2 {print $4}')
    echo " "
    echo "Directory Space used: $directory_used | Directory Space available: $directory_available"
    echo "Partition Space used: $partition_used | Partition Space available: $partition_available "
  else
    echo "Directory does not exist: $directory"
  fi
  echo "------------------------------------------------------------------"
done
#Test Objective     : To check if gstreamer libraries match with package version installed in DUT.
#Automation_approch : To validate versions of gstreamer libraries.
#ExpectedOutput     : Versions should match with the package version installed in DUT.
version=$(gst-launch-1.0 --version | grep -o 'GStreamer [0-9.]*' | cut -d ' ' -f 2)
echo "Checking version of GStreamer libraries..."

matching=()
not_matching=()

for lib in $(find / -name "*lib*gst*.so" -type f 2>/dev/null  ); do
    # Check library file version
    if strings "$lib" | grep "$version" ; then
        matching+=("$lib")
    else
        not_matching+=("$lib")
    fi
done 2>&1 >/dev/null

echo "Summary:"
echo "Total libraries checked: ${#matching[@]} + ${#not_matching[@]}"
echo "*******************************************************"
echo "Libraries matching GStreamer package version installed in DUT: ${#matching[@]}"
if [[ ${#matching[@]} -gt 0 ]]; then
    echo "*******************************************************"
    echo "Matching libraries:"
    printf '%s\n' "${matching[@]}"
	echo "*******************************************************"
fi
echo "*******************************************************"
echo "Libraries not matching GStreamer package version installed in DUT: ${#not_matching[@]}"
if [[ ${#not_matching[@]} -gt 0 ]]; then
    echo "*******************************************************"
    echo "Non-matching libraries:"
    printf '%s\n' "${not_matching[@]}"
    echo "*******************************************************"
fi
echo "---------------------------------------------------------------------------------------"
#Test Objective      : To display supported resolution modes, aspect ratios and status of the currently connected output.
#Automation_approach : To check whether card0-HDMI-A-1 is present(It is a virtual file present only when hdmi interface is detected).
#ExpectedOutput      : Supported resolution modes, aspect ratios and status of the currently connected output.

hdmi_device_path=$(find /sys/class/drm -name "*HDMI*")
# Check if HDMI device is present and HDMI is connected
if [[ -z $hdmi_device_path ]] ||  [[ $(cat "$hdmi_device_path/status") != "connected" ]]; then
    echo "HDMI is not connected"
    exit 1
fi

# Function to calculate the aspect ratio (resolution width/resolution height)
calculate_aspect_ratio() {
  dividend=$1
  divisor=$2
  remainder=$((dividend % divisor))
  if [ "$remainder" -eq 0 ]; then
    echo "$divisor"
  else
    calculate_aspect_ratio "$divisor" "$remainder"
  fi
}
echo "Checking supported resolution modes, aspect ratios and status of the currently connected output"
echo "Supported Resolutions:"
#Read the supported resolutions from the file
supported_resolutions=$(cat "$hdmi_device_path/modes")

#Loop through each supported resolution
for resolution in $supported_resolutions; do
  # Split the resolution into width and height
  IFS="x" read -r resolution_width resolution_height <<< "$resolution"

  # Remove any additional characters like 'i' from the height
  resolution_height=${resolution_height%%[a-zA-Z]*}

  # Calculate the aspect ratio if width and height are numeric
  if [[ $resolution_width =~ ^[0-9]+$ && $resolution_height =~ ^[0-9]+$ ]]; then
    divisor=$(calculate_aspect_ratio "$resolution_width" "$resolution_height")
    aspect_ratio_width=$((resolution_width / divisor))
    aspect_ratio_height=$((resolution_height / divisor))
    echo "Resolution: $resolution"
    echo "Aspect Ratio: $aspect_ratio_width:$aspect_ratio_height"
  fi
done
echo "-------------------------------------------------------------"

# Extract the current resolution using fbset command
current_resolution=$(fbset -s | grep -o '[0-9]\{3,4\}x[0-9]\{3,4\}')
if [[ -z $current_resolution ]]; then
    echo "Failed to extract resolution."
    exit 1
fi
# Check if the connected device name is available
device_name=$(cat "$hdmi_device_path/device/device/graphics/fb0/name" 2>/dev/null)
if [[ -n $device_name ]]; then
  echo "Connected Device Name: $device_name"
fi
echo "Current Resolution: $current_resolution"
# Split the resolution into width and height
IFS="x" read -r resolution_width resolution_height <<< "$current_resolution"
# Calculate the aspect ratio
divisor=$(calculate_aspect_ratio "$resolution_width" "$resolution_height")
aspect_ratio_width=$((resolution_width / divisor))
aspect_ratio_height=$((resolution_height / divisor))
# Print the aspect ratio
echo "Current Aspect Ratio: $aspect_ratio_width:$aspect_ratio_height"
echo "---------------------------------------------------------------------------------------"




