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
#Note 1: To execute all testcases in shell script execute with full name and all argument.
#Eg    : sh system_sanity_check.sh all
#Note 2: To execute specific testcase/testcases in shell script execute with the full name followed by testcase number.
#Eg    : sh system_sanity_check.sh 1/sh system_sanity_check.sh 1 2
date
echo -e "Executing script : System_Sanity_Check"
echo -e "============================================="
echo -e "Test Execution Name is: System_Sanity_Check"
echo -e "============================================="
#Function to print the help message
print_help() {
  echo "----------------------------------------------------"
  echo " Basic Sanity Testcase Help"
  echo "----------------------------------------------------"   
  echo "Note: To execute all testcases available in this script: sh system_sanity_check.sh all"
  echo "Note: To execute specific testcase/testcases from this script: sh system_sanity_check.sh 1 | sh system_sanity_check.sh 1 2"
  echo "Test cases:"
  echo "  1: DUT Connectivity."
  echo "  2: SSH Connectivity of DUT."
  echo "  3: DUT Standby state."
  echo "  4: Essential Services required for framework running in DUT."
  echo "  5: Essential Processes required for framework running in DUT."
  echo "  6: Essential Logfiles required for framework running in DUT."
  echo "  7: Display the Storage of directories and it's Partitions in DUT."
  echo "  8: Validate versions of gstreamer libraries."
  echo "  9: Check if Services are failed or in activating state from systemd services."
  echo " 10: Validate the System Device Details available in the device with regex pattern."
  echo " 11: Validate RDK6 component versions with the Config file in the DUT."
  echo " 12: Display supported resolution modes, aspect ratios and status of the currently connected output"
  echo " 13: Unknown keypress check."
  echo " 14: Binary Files update checker."
  echo " 15: Continuously Monitor CPU Usage for given time in background."
  echo " 16: Validate Plugin support in DUT."
  echo " 17: Compare FREE memory responses of both tr181 command and /proc/meminfo."
  exit 0
}

#Read the config file
config_file="$(dirname "$0")/sanity_check.config"
if [ ! -f $config_file ];then
echo "Please place the config file $config_file in the path and re-execute script"
exit 1
fi

source $config_file


#Checking if Dut is connected to Ethernet or Wifi and have Internet Connectivity
testcase1 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 1 : DUT Connectivity "
echo "---------------------------------------------------------------------------------------"
ETHERNET_IP=$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')
if [ -z "$ETHERNET_IP" ];then
  echo "Ethernet is not connected"
else
  echo "Ethernet is connected"
  echo "ip address : $ETHERNET_IP"
    ping -c 3 8.8.8.8
    if [ "$?" -eq "0" ]; then
     echo "Ethernet interface is connected and has internet connectivity"
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
    fi
fi
}
#Validate SSH Connectivity of DUT through netcat command
testcase2 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 2 :  SSH Connectivity of DUT "
echo "---------------------------------------------------------------------------------------"
if [[ -n "$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')" ]]; then
  ip_address="$(ifconfig eth0 | awk '/inet addr/{print substr($2,6)}')"
elif [[ -n "$(ifconfig wlan0 | awk '/inet addr/{print substr($2,6)}')" ]]; then
  ip_address="$(ifconfig wlan0 | awk '/inet addr/{print substr($2,6)}')"
fi

IP_ADDRESS="$ip_address"

SECONDS=0
TIMEOUT=20
#Loop through a range of port numbers and checking for SSH port
port=1
until [ $port -gt 65535 ]; do
  if [[ $SECONDS -gt $TIMEOUT ]]; then
    echo "IP_ADDRESS:${IP_ADDRESS}, Timeout: No SSH port found"
    return 1
  fi
  output=$(echo >/dev/null | nc -w 1 $IP_ADDRESS $port 2>&1)
  if [[ -n $output ]] && echo "$output" | grep -q "SSH"; then
    echo "SSH is running on IP_ADDRESS:${IP_ADDRESS} PORT_NUMBER: $port"
    return 1
  fi
  ((port++))
done

}

#Checking Device Standby state of DUT
testcase3 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 3 : DUT Standby state "
echo "---------------------------------------------------------------------------------------"
if [[ $(systemctl status systemd-suspend.service | grep Active) == *"active (standby)"* ]]; then
  echo "DUT is currently in standby mode."
else
  echo "DUT is not in standby mode."
fi
}
#Validate Status of the Essential Services required for framework running in DUT
testcase4 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 4 : Essential Services required for framework running in DUT "
echo "---------------------------------------------------------------------------------------"

#Check if the paths are configured
if [ -z "$services" ]; then
  echo "The services are missing in the configuration file, Pls configure and re-run."
  return 1
fi

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

echo "Running_services:{"${running_services[*]}"}" | sed 's/[[:blank:]]/,/g'

#Printing Failed Services
if [[ ${#failed_services[@]} -gt 0 ]];then
  failedservices=$(IFS=,; echo "${failed_services[*]}")
  echo "FAILURE: FAILED_SERVICES:{"${failedservices}"}" 
else
  echo "SUCCESS: All ESSENTIAL SERVICES ARE RUNNING"
fi

}
#Checking if Essential Processes are running in DUT
testcase5 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 5 : Essential Processes required for framework running in DUT "
echo "---------------------------------------------------------------------------------------"

#Check if the paths are configured
if [ -z "$processes" ]; then
  echo "The processes are missing in the configuration file, Pls configure and re-run."
  return 1
fi

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

echo "Running_processes:{"${running_process[*]}"}" | sed 's/[[:blank:]]/,/g'

#Printing Failed processes
if [[ ${#failed_process[@]} -gt 0 ]];then
  failedprocesses=$(IFS=,; echo "${failed_process[*]}")
  echo "FAILURE: FAILED_PROCESSES:{"${failedprocesses}"}" 
else
  echo "SUCCESS: All ESSENTIAL PROCESSES ARE RUNNING"
fi
}

#Checking if Required Logfiles are Present in DUT
testcase6 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 6 : Essential Logfiles required for framework running in DUT "
echo "---------------------------------------------------------------------------------------"

#Check if the paths are configured
if [ -z "$LOG_FILES" ]; then
  echo "The LOG_FILES are missing in the configuration file, Pls configure and re-run."
  return 1
fi

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

echo "Required_LOG_FILES:{'"${Files_Present[*]}"'}" | sed 's/[[:blank:]]/,/g'

#Printing Missing LOG_FILES
if [[ ${#Files_Missing[@]} -gt 0 ]];then
  failedlogfiles=$(IFS=,; echo "${Files_Missing[*]}")
  echo "FAILURE: MISSSING_LOGFILES:{"${failedlogfiles}"}"
else
  echo "SUCCESS: All REQUIRED LOG_FILES are Present"
fi

}

#Test Objective     : To display the Storage of directories and it's Partitions in DUT.
#Automation_approch : To display Storage of directories and Partitions.
#Expected Output    : Display the available and used memory of each directories and it's associated partitions.
testcase7 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 7 : Display the Storage of directories and it's Partitions in DUT "
echo "---------------------------------------------------------------------------------------"
#Specify the starting directory for the search
starting_directory="/"

#Find all main directories and validate each one and excluding proc since it is a virtual directory
for directory in $(find "$starting_directory"* -maxdepth 0 -type d | grep -v "/proc"); do
  if [ -d "$directory" ]; then
    echo "Validating Directory: $directory & Associated Partition: $partition "
    #Checking the size of the directory
    directory_used=$(du -sh "$directory" | awk '{print $1}')
    directory_available=$(df -hP "$directory" | awk 'NR==2 {print $4}')
    #Checking the partition details of the directory
    partition=$(df -hP "$directory" | awk 'NR==2 {print $1}')
    partition_used=$(df -hP "$directory" | awk 'NR==2 {print $3}')
    partition_available=$(df -hP "$directory" | awk 'NR==2 {print $4}')
    echo " "
    echo "Directory Space used: $directory_used | Directory Space available: $directory_available"
    echo "Partition Space used: $partition_used | Partition Space available: $partition_available "
  else
    echo "FAILURE: Directory does not exist: $directory"
  fi
  echo "------------------------------------------------------------------"
done
}

#Test Objective      : To check if gstreamer libraries match with package version installed in DUT.
#Automation_approach : To validate versions of gstreamer libraries.
#ExpectedOutput      : validate versions of gstreamer libraries.
testcase8 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 8 : Validate versions of gstreamer libraries "
echo "---------------------------------------------------------------------------------------"
version=$(gst-launch-1.0 --version | grep -o 'GStreamer [0-9.]*' | cut -d ' ' -f 2)
echo "Checking version of GStreamer libraries..."

matching=()
not_matching=()

for lib in $(find / -name "*lib*gst*.so" -type f 2>/dev/null  ); do
  #Check only if version is not empty
  if [[ -n $(strings "$lib" | grep "$version") ]]; then
    #Check library file version
    if strings "$lib" | grep "$version" ; then
        matching+=("$lib")
    else
        not_matching+=("$lib")
    fi
  fi
done 2>&1 >/dev/null

echo "Total libraries checked: Matching: ${#matching[@]} :: Non Matching: ${#not_matching[@]}"
if [[ ${#not_matching[@]} -gt 0 ]]; then
    echo "*******************************************************"
    echo "Non-matching libraries:"
    printf '%s\n' "${not_matching[@]}"
    echo "*******************************************************"
    echo "FAILURE: Libraries are not matching GStreamer package version installed in DUT: ${#not_matching[@]}"
else
    echo "SUCCESS: Libraries are matching with GStreamer package version installed in DUT"
fi
}

#Test Objective       : To check whether any systemd services are in failed or activating state.
#Automation_approach  : Check if Services are failed or in activating state from systemd services.
#ExpectedOutput       : Services which are failed or activating should be listed from systemd services.
testcase9 () {
echo "----------------------------------------------------------------------------------------------------------"
echo  "Testcase 9 : Check if Services are failed or in activating state from systemd services"
echo "----------------------------------------------------------------------------------------------------------"

echo "List of the Systemd Services in Device"
Failed_Services()
{
RESPONSE=$(systemctl -a --state=failed,activating | grep 'failed\|activating')
echo "$RESPONSE"
}
Failed_Services

if [[ $RESPONSE ]];then
	echo "FAILURE: A few systemd services are in failed or activating state"
else
	echo "SUCCESS: No systemd services are in failed or activating state"
fi
}

#Test Objective       : To Validate the System Device Details available in the device with regex pattern.
#Automation_approach  : To read parameters from file /tmp/.deviceDetails.cache and check using regex for each parameter.
#ExpectedOutput       : Device Parameters should be matched with the Regex Pattern.
testcase10 () {
echo "----------------------------------------------------------------------------------------------------------"
echo  "Testcase 10 : Validate the System Device Details available in the device with regex pattern"
echo "----------------------------------------------------------------------------------------------------------"

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

#Array to store Failed and Empty parameters
FAILEDPARAMETER=()
EMPTYPARAMETER=()
#Check if box_IP is in expected format
BOX_IP=$(cat /tmp/.deviceDetails.cache | grep boxIP | cut -d "=" -f2)
regex='^(([0-9]{1,3}\.){3}[0-9]{1,3})|(([0-9a-fA-F]*[:-])+([0-9a-fA-F])*)$'
if [[ -z $BOX_IP ]]; then
  echo -e "box_IP is EMPTY \nValue=${BOX_IP}\n"
  EMPTYPARAMETER=("box_IP=$BOX_IP")
elif [[ "$BOX_IP" =~ $regex  ]]; then
  echo -e "Verification of box_IP is SUCCESS \nValue=${BOX_IP}\n"
else
  echo -e "Verification of box_IP is FAILURE \nValue=${BOX_IP}\n"
  BOX_IP_DEVICE_DETAILS=$(cat /tmp/.deviceDetails.cache | grep boxIP)
  FAILEDPARAMETER=("$BOX_IP_DEVICE_DETAILS")
fi

VALIDATE_VALUE()
{
  PARAMETER=$1
  #Check if $PARAMETER is in expected format
  echo "Executing ExecuteCommand...."
  RESPONSE=$(cat /tmp/.deviceDetails.cache)
  RESULT=$(echo "$RESPONSE" | grep "$PARAMETER")
  regex=""
  if [ "$PARAMETER" == "bluetooth_mac" ]; then
    if $IS_BLUETOOTH_SUPPORTED
    then
      regex='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]){2}$'
    else
      regex='^$'
    fi
  elif [ $PARAMETER == "build_type" ]; then
    regex='(VBN|DEV)'
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
  if [[ -z $VALUE ]]; then
    echo -e "$PARAMETER is empty \nValue=${VALUE}\n"
    EMPTYPARAMETER+=($PARAMETER=$VALUE)
  elif [[ $VALUE =~ $regex ]]; then
    echo -e "Verification of $PARAMETER is SUCCESS \nValue=${VALUE}\n"
  else
    if [[ $PARAMETER == "wifi_mac" && ! $IS_WIFI_SUPPORTED && ! -z $VALUE  ]]; then
      echo "wifi_mac should be empty for this device(WIFI NOT SUPPORTED)"
    elif [[ $PARAMETER == "bluetooth_mac" && ! $IS_BLUETOOTH_SUPPORTED && ! -z $VALUE ]]; then
      echo "bluetooth_mac should be empty for this device(Bluetooth NOT SUPPORTED)"
    fi
    echo -e "Verification of $PARAMETER is FAILED \nValue=${VALUE}\n"
    FAILEDPARAMETER+=( "$PARAMETER=$VALUE" )
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

#Printing Summary
if [ ${#FAILEDPARAMETER[@]} -gt 0 ]; then
    failedParameters=$(IFS=,; echo "${FAILEDPARAMETER[*]}")
    echo "FAILURE: FailedParameters:{${failedParameters}}"
fi
if [ ${#EMPTYPARAMETER[@]} -gt 0 ]; then
    emptyParameters=$(IFS=,; echo "${EMPTYPARAMETER[*]}")
    echo "FAILURE: EmptyParameters:{${emptyParameters}}"
fi

if [ ${#FAILEDPARAMETER[@]} -eq 0 ] && [ ${#EMPTYPARAMETER[@]} -eq 0 ]; then
    echo "SUCCESS: All parameters are verified"
fi
}

#Test Objective      : To Validate RDK6 component versions with the Config file in the DUT.
#Automation_approach : To Check if Versions are matching in the DUT with RDK6 release components.
#ExpectedOutput      : Versions should match with the configured file in DUT.
testcase11 () {
echo "---------------------------------------------------------------------------------------------------"
echo  "Testcase 11 : Validate RDK6 component versions with the Config file in the DUT "
echo "---------------------------------------------------------------------------------------------------"

MISMATCHED=()
RESPONSE=$(cat "$config_file")
Version_Validation() {
    PARAMETER=$1
#Check if $PARAMETER is in expected format
echo "Version Validation of $PARAMETER"
RESULT=$(echo "$RESPONSE" | grep "$PARAMETER")
echo "Executing ExecuteCommand...."
command=""
#Compares the version from DUT with $parameter mentioned in configuration file
if [ "$PARAMETER" = "kernel_version" ]; then
command=$(uname -r | cut -d "-" -f1)
elif [ "$PARAMETER" == "Gstreamer_version" ]; then
command=$(gst-inspect-1.0 --version | sed -n 2p | cut -d " " -f2)
elif [ "$PARAMETER" == "wpa_supplicant_version" ]; then
command=$(wpa_supplicant -v | grep wpa_supplicant | cut -d ' ' -f2 | grep -o '[0-9.]*')
elif [ "$PARAMETER" == "Glibc_version" ]; then
command=$(ls -l /lib/libc.so.6 | cut -d '-' -f3)
elif [ "$PARAMETER" == "Openssl_version" ]; then
command=$(openssl version | cut -d " " -f2 | grep -o '[0-9.]*')
elif [ "$PARAMETER" == "dbus_daemon_version" ]; then
command=$(dbus-daemon --version | grep D-Bus | cut -d ' ' -f5)
elif [ "$PARAMETER" == "bluetoothctl_version" ]; then
command=$(bluetoothctl --version | cut -d ":" -f2 | sed 's/^[[:space:]]*//')
elif [ "$PARAMETER" == "systemd_version" ]; then
command=$(systemctl --version | grep "systemd" | cut -d " " -f2)
elif [ "$PARAMETER" == "apparmor_version" ]; then
command=$(apparmor_parser -V | grep "version" | cut -d " " -f4)
elif [ "$PARAMETER" == "YOCTO_VERSION" ]; then
command=$(grep VERSION_ID /etc/os-release | cut -d '=' -f2)
elif [ "$PARAMETER" == "Lightning_version" ]; then
command=$(grep "Lightning v" /home/root/lxresui/js/lightning.min.js | cut -d' ' -f4 | grep -o '[0-9.]*' )
elif [ "$PARAMETER" == "Browser_WPEWebkit_version" ]; then
command=$(grep -o 'WPE_WEBKIT_VERSION=[0-9.]*' /version.txt | grep -o '[0-9.]*')
fi


command=$(echo "$command" | tr -d '[:space:]')
echo "Version Specified in configuration file for RDK6:${RESULT// } & Response From DUT:${PARAMETER// }=$command"
VALUE=$(echo "$RESULT" | cut -d "=" -f2 | tr -d '[:space:]')
if [ "$VALUE" == "$command" ]; then
echo -e "Version Verification of $PARAMETER is Successful\n"
else
echo -e "Version Verification of $PARAMETER is Failed\n"
MISMATCHED+=( "$PARAMETER=$command" )
fi
}
#Check if kernel version is matching with the Given value
Version_Validation "kernel_version"
#Check if Gstreamer version is matching with the specified value
Version_Validation "Gstreamer_version"
#Check if wpa_supplicant version is matching with the specified value
Version_Validation "wpa_supplicant_version"
#Check if Glibc version is matching with the specified value
Version_Validation "Glibc_version"
#Check if Openssl version is matching with the specified value
Version_Validation "Openssl_version"
#Check if dbus-daemon version is matching with the specified value
Version_Validation "dbus_daemon_version"
#Check if bluetoothctl version is matching with the specified value
Version_Validation "bluetoothctl_version"
#Check if systemd version is matching with the specified value
Version_Validation "systemd_version"
#Check if apparmor version is matching with the specified value
Version_Validation "apparmor_version"
#Check if YOCTO_VERSION is matching with the specified value
Version_Validation "YOCTO_VERSION"
#Check if Lightning version is matching with the specified value
Version_Validation "Lightning_version"
#Check if Browser_WPEWebkit version is matching with the specified value
Version_Validation "Browser_WPEWebkit_version"

#Printing Summary
if [ ${#MISMATCHED[@]} -gt 0 ]; then
    failedParameters=$(IFS=,; echo "${MISMATCHED[*]}")
    echo "FAILURE: FailedParameters:{${failedParameters}}"
else
    echo "SUCCESS: All parameters are verified"
fi
}

#Test Objective      : To display supported resolution modes, aspect ratios and status of the currently connected output.
#Automation_approach : To check whether card0-HDMI-A-1 is present(It is a virtual file present only when hdmi interface is detected).
#ExpectedOutput      : Supported resolution modes, aspect ratios and status of the currently connected output.
testcase12 (){
echo "-----------------------------------------------------------------------------------------------------------"
echo "Testcase 12: Display supported resolution modes, aspect ratios and status of the currently connected output"
echo "-----------------------------------------------------------------------------------------------------------"
hdmi_device_path=$(find /sys/class/drm -name "*HDMI*")
# Check if HDMI device is present and HDMI is connected
if [[ -z $hdmi_device_path ]] ||  [[ $(cat "$hdmi_device_path/status") != "connected" ]]; then
    echo "FAILURE: HDMI is not connected"
    return 1
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
    return 1
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
}


#Test Objective     : To connect to bluetooth device and detect unknown keypress events in the logfile if any.
#Automation_approach: Pair and Connect to bluetooth device and find error messages if any.
#Expected Output    : Detect unknown keypress events in the logfile and report if any.
#pre_requisite      : The script expects the bluetooth device(emulator) to be discoverable.
#Note               : Pls accept the pairing popup on bluetooth device(emulator) when prompted
testcase13 () {
echo "---------------------------------------------------------------------------------------"
echo "Testcase 13: Unknown keypress check"
echo "---------------------------------------------------------------------------------------"

echo "pre_requisite      : The script expects the bluetooth device(emulator) to be discoverable"
echo "Note               : Pls accept the pairing popup on bluetooth device(emulator) when prompted"

#Checking if device name and timeout_seconds are configured
missing_parameters=()

[ -z "$device_name" ] && missing_parameters+=("DEVICE NAME")
[ -z "$timeout_seconds" ] && missing_parameters+=("TIMEOUT_SECONDS")

if [ ${#missing_parameters[@]} -gt 0 ]; then
  missing_params_list=$(IFS=','; echo "${missing_parameters[*]}")
  echo "Error: ${missing_params_list[*]} not found in config file. Pls configure and re-run."
  return 1
fi

#Removing paired devices from rdk-device if any
rdk_device=($(bluetoothctl <<< paired-devices | awk '{print $2}' |  grep -oE '([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}'))
#Looping through the MAC addresses and removing the paired devices if any
for mac_address in $rdk_device; do
  {
    echo "remove $mac_address"
    sleep 5
  } | bluetoothctl
done

#Scan for available devices
scan_output=""
scan_output=$( {
  echo "scan on"
  sleep 20
  echo "quit"
} | bluetoothctl )

#Finding the MAC address of the specified device
#Check if any devices were found
if [[ -z "$scan_output" ]]; then
  echo "No devices found.Device are not discoverable"
else
  echo "Devices found: "
  echo "$scan_output"
fi

#Fetch the MAC address of the device based on its name
device_mac=$(echo "$scan_output" | grep -i "Device.*"$device_name"" | grep -oE '([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}')

#Check if the bluetooth MAC address was found
if [[ -z "$device_mac" ]]; then
    echo "FAILURE: "$device_name" Bluetooth MAC address was not found. Pls make the device discoverable and try again"
    return 1
fi
echo "-----------------------------------------------------------"
echo "Emulator mac retrieved: Device name: $device_name:$device_mac"
echo "-----------------------------------------------------------"
#Pair to the device
#Wait until the device is paired
echo "Pairing to the device"
echo "-------------------------------------------------------------"
echo "Please confirm the pairing request on the screen"
echo "------------------------------------------------------------"
{
  echo "pair $device_mac"
  sleep 20
  echo "yes"
  sleep 5
} | bluetoothctl

SECONDS=0
TIMEOUT_PAIR_CONNECT=10
until [[ "$(bluetoothctl <<< "info $device_mac")" =~ "Paired: yes" ]] ; do
  if [[ SECONDS -gt TIMEOUT_PAIR_CONNECT ]]; then
    echo "FAILURE: Failed to pair with the device: "$device_name": "$device_mac", Pls rerun and accept the pairing popup"
    return 1
  fi
  sleep 5
done

echo "----------------------------------------------------------------------"
echo "Device is paired Successfully to Device name: $device_name:$device_mac"
echo "----------------------------------------------------------------------"

#Connect to the device
#Wait until the device is connected
echo "Connecting to the device"
{
echo "connect $device_mac"
sleep 5
} | bluetoothctl

SECONDS=0
until  [[ "$(bluetoothctl <<< "info $device_mac")" =~ "Connected: yes" ]]  ; do
  if [[ SECONDS -gt TIMEOUT_PAIR_CONNECT ]]; then
    echo "FAILURE: Failed to connect with the device: $device_name: $device_mac, Pls rerun and accept the pairing popup"
    return 1
  fi
  sleep 5
done

echo "--------------------------------------------------------------------------"
echo "Device is connected Successfully to Device name: $device_name:$device_mac"
echo "--------------------------------------------------------------------------"

echo "Monitoring Wpeframework.log for unknown keypress events"
#Running tail command with a timeout to monitor keypress events
output=$(timeout "$timeout_seconds" tail -n 0 -f "$log_file" | grep -nE "unknown key code|common key code not found")
if [ -z "$output" ]; then
    echo "No unknown keypress event logs found"
else
    echo "$output"
fi

}

#Test Objective     : To Compare binary files with image build date and report if any old binary files are found.
#Automation_approach: List down all the files in the given locations and filter the files based on image build date.
#Expected Output    : Lists Success when no old binaries are found and Failure when old binaries are found.
testcase14 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 14 : Binary Files update checker "
echo "---------------------------------------------------------------------------------------"

#Check if the paths are configured
if [ -z "$SEARCH_PATHS" ]; then
  echo "The search paths are missing in the configuration file, Pls configure and re-run."
  return 1
fi

#Splitting the SEARCH_PATHS variable into an array using the ',' delimiter
IFS="," read -r -a search_paths <<< "$SEARCH_PATHS"

#Find version.txt file and retrieve its date
version_date=$(grep "BUILD_TIME" /version.txt | awk -F'"' '{print $2}' | awk -F' ' '{print $1}')
if [[ -z "$version_date" ]]; then
  echo "version.txt file not found or has an invalid format."
  return 1
fi

#Get the image date in the format YYYY-MM-DD for comparison
image_date=$(date -d "$version_date" +"%Y-%m-%d")
formatted_date=$(date -d "$version_date" +"%b %d")
echo "---------------------------------------------"
echo "Image build date: $formatted_date"
echo "---------------------------------------------"

#Execute find command on search paths
files=$(find "${search_paths[@]}" -type f \( ! -path "*/proc/*" -a ! -path "*/run/*" -a ! -path "*/tmp/*" -a ! -path "*/logs/*" -a ! -path "*/sys/*" -a ! -path "*/certs/*" \) 2>/dev/null)

#Array to store outdated binary files
outdated_files_found=()

#Execute find command on the specified search path
for file in $files; do
  #Filtering to ignore certain file extensions since they are not updated on regular basis
  if [[ ! "$file" =~ \.(log|txt|sh|jpeg|jpg|png|config|csv)$ ]]; then
    #Command lists out the last updated date of file
    file_date=$(date -r "$file" +"%Y-%m-%d" 2>/dev/null)
    #If the date is retrieved and less than the build date
    if [[ -n "$file_date" ]] && [[ "$file_date" < "$image_date" ]]; then
      #Convert the file_date to the desired format
      formatted_file_date=$(date -d "$file_date" +"%b %d %Y")
      echo "File: $file - Last Updated date: $formatted_file_date"
      outdated_files_found+=($"file")
    fi
  fi
done
echo "-----------------------------------------------------------------------------------------------------------------"
#Check if any outdated files are found
if [ ${#outdated_files_found[@]} -gt 0 ]; then
  echo "FAILURE: Old binary files are found and listed"
else
  echo "SUCCESS: No Old binary files found"
fi
echo "-----------------------------------------------------------------------------------------------------------------"
}

#Test Objective     : To monitor CPU usage for given interval of time.
#Automation_approach: Monitor the cpu usage in the background and list highest 3 cpu usages at the end of the script and kill the script after timeout.
#Expected Output    : List the 3 processes with highest cpu usage.
testcase15() {

#Function to check if a value is an integer
check_parameter() {
  [[ "$1" =~ ^[0-9]+$ ]]
}
#Setting a flag to track parameter validation errors
validation_failed=false

#Function to check and validate a parameter
check_and_validate_parameter() {
    if [ -z "$2" ]; then
      echo "Pls configure $1 value and re-run."
      validation_failed=true
    elif ! check_parameter "$2"; then
      echo "Error: $1 must be a number. Pls configure integer value and re-run."
      validation_failed=true
    fi
}

check_and_validate_parameter "Total Duration" "$total_duration"
check_and_validate_parameter "High CPU Threshold" "$high_cpu_threshold"
check_and_validate_parameter "Check interval" "$check_interval"

#Check if any parameter validation failed
if [ "$validation_failed" = true ]; then
  echo "Validation failed. Exiting."
  echo " "
  return 1
fi

#logfile path to store logs in DUT
LOGFILE_PATH=opt
logfile="/$LOGFILE_PATH/cpu_monitor.log"
#clear the logfile if any old logs are present
> "$logfile"
#Redirecting all output to the log file
exec >> "$logfile" 2>&1
sleep 10
echo "---------------------------------------------------------------------------------------------------------------------------------"
echo "Total duration of the script specified in config file: $total_duration seconds"
echo "---------------------------------------------------------------------------------------------------------------------------------"
#Function to check Highest CPU usage
print_processes() {
echo "---------------------------------------------------------------------------------------------------------------------------------"
echo "3 Processes with Highest CPU USAGES"
echo "PID   USER        CPU %    COMMAND"
echo "---------------------------------"
top_processes=$(top -bn1 | tail -n +8 | head -n 4 | grep -v top | awk '{printf "%-6s %-10s %-8s %s\n", $1, $2, $9, $12}')
echo "$top_processes"
echo "----------------------------------------------------------------------------------------------------------------------------------"
}

#Array for storing failure count
is_high_cpu_usage_detected=()
#Function to check CPU usage
check_cpu_usage() {
    timestamp=$(date +"%d-%m-%Y %H:%M:%S")
    #Get the current CPU usage percentage
    cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}')
    cpu_usage=${cpu_usage%.*}
    echo "[$timestamp] CPU usage: ${cpu_usage}%"
    #Check if CPU usage is greater than the threshold
    if [ $((cpu_usage)) -ge $((high_cpu_threshold)) ]; then
        echo "High CPU usage detected"
        is_high_cpu_usage_detected+=(1)
    else
        echo "CPU usage is within given threshold value."
    fi
}

SECONDS=0
while true; do
    check_cpu_usage
    print_processes
    if [ $((SECONDS)) -gt $((total_duration)) ]; then
      if [ ${#is_high_cpu_usage_detected[@]} -gt 0 ]; then
        echo "FAILURE: Cpu Usage is greater than Threshold within the given timeout."
      else
        echo "SUCCESS: Cpu Usages did not meet Threshold within given time"
      fi
      if [ -n "$testcase15_pid" ] && ps -p "$testcase15_pid" > /dev/null; then
        echo "Killing background process of testcase 15 with PID $testcase15_pid"
        kill "$testcase15_pid"
      else
       echo "testcase 15 background process is completed."
      fi
      break
    else
      sleep "$check_interval"
    fi
done
}

#Function to start testcase 15 in background
start_testcase_15() {
  echo "---------------------------------------------------------------------------------------"
  echo  "Testcase 15 : Continuously Monitor CPU Usage for given time in background "
  echo "---------------------------------------------------------------------------------------"
  echo "testcase 15 is running in background, Pls check the "/opt/cpu_monitor.log""
  testcase15 &
  #Storing the PID of the background process
  testcase15_pid=$!
  echo "PID: $testcase15_pid"
}

#Test Objective     : To check the status of plugins and report if it's not supported in DUT.
#Automation_approach: Check the status of given list of plugins and validate support in the DUT.
#Expected Output    : List supported and not supported plugins in DUT.
#Note: To validate multiple directories provide directory names seperated by comma in config file to variable directories_to_test. 
testcase16 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 16 : Validate Plugin Support in DUT "
echo "---------------------------------------------------------------------------------------"
#Checking if DIRECTORIES, API_URL_IP and API_URL_PORT are configured
missing_parameters=()
DIRECTORIES=()

[ -z "$directories_to_test" ] && missing_parameters+=("DIRECTORIES")
[ -z "$api_url_ip" ] && missing_parameters+=("API_URL_IP FOR CURL COMMAND")
[ -z "$api_url_port" ] && missing_parameters+=("API_URL_PORT FOR CURL COMMAND")

if [ ${#missing_parameters[@]} -gt 0 ]; then
  missing_params_list=$(IFS=','; echo "${missing_parameters[*]}")
  echo "Error: ${missing_params_list[*]} not found in config file. Pls configure and re-run."
  return 1
fi

IFS=$','
#directories to be tested
read -r -a DIRECTORIES <<< "$directories_to_test"
unset IFS
#flag to validate final summary
FINAL_RESULT=true
#Loop through all directories and check plugin status
for directory in "${DIRECTORIES[@]}"; do
  #Arrays for supported and not supported
  supported_plugins=()
  not_supported_plugins=()
  #Loop through all JSON files in the directory and check plugin status
  for json_file in "$directory"/*.json; do
      #Extract the plugin name from the JSON file
      plugin=$(basename "$json_file" .json)
      #Check the plugin status
      response=$(curl --silent --header "Content-Type: application/json" --request POST --data "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\": \"Controller.1.status@$plugin\"}" "http://$api_url_ip:$api_url_port/jsonrpc")
      #Check if the response contains the specific error code and message
      if [[ "$response" == *'"error":{"code":22,"message":"ERROR_UNKNOWN_KEY"}'* ]]; then
          not_supported_plugins+=("$plugin")
          FINAL_RESULT=false
      else
          supported_plugins+=("$plugin")
      fi
  done
  echo "                                        |Validation Summary|                              "
  echo "-----------------------------------------------------------------------------------------------------------------"
  echo "Validated Directory: $directory"
  num_validated_plugins=$(( ${#supported_plugins[@]} + ${#not_supported_plugins[@]} ))
  echo "Number of Validated Plugins: $num_validated_plugins"
  echo "-----------------------------------------------------------------------------------------------------------------"
  if [[ -n "$supported_plugins" ]]; then
    echo "                                        |Supported Plugins|                              "
    for supported in "${supported_plugins[@]}"; do
      echo "| $supported |"
    done
  fi
  echo "-----------------------------------------------------------------------------------------------------------------"
  if [[ -n "$not_supported_plugins" ]]; then
    echo "                                        |Not Supported Plugins|                              "
    for not_supported in "${not_supported_plugins[@]}"; do
      echo "| $not_supported |"
    done
  fi
  echo "-----------------------------------------------------------------------------------------------------------------"
done

if [[ "$FINAL_RESULT" = true ]]; then
    echo "SUCCESS: All plugins are supported in DUT."
else
    echo "FAILURE: Few Plugins are not supported in DUT"
fi
echo "-----------------------------------------------------------------------------------------------------------------"
}

#Test Objective       - To compare FREE memory responses of both tr181 command and /proc/meminfo.
#Automation Approach  - Compare the responses and report if it is exceeding the threshold limit.
#Expected Output      - If Responses from both tr181 command and /proc/meminfo are same then success or else failure.
testcase17 () {
echo "---------------------------------------------------------------------------------------"
echo  "Testcase 17 : Compare FREE memory responses of both tr181 command and /proc/meminfo "
echo "---------------------------------------------------------------------------------------"
#Fetching the responses of FREE memory
tr181_memory=$(tr181 Device.DeviceInfo.MemoryStatus.Free 2>&1)
meminfo_memory=$(grep MemFree /proc/meminfo | awk '{print $2}')

#Threshold limit to validate free memory responses
FREE_MEMORY_THRESHOLD=5

#Check if tr181_memory or meminfo_memory is empty and positive numbers
if [ -z "$tr181_memory" ] && ! [[ $tr181_memory =~ ^[0-9]+$ ]]; then
  echo "Error: tr181_memory command returned empty or invalid output."
  exit 1
fi

if [ -z "$meminfo_memory" ] && ! [[ $meminfo_memory =~ ^[0-9]+$ ]]; then
  echo "Error: meminfo_memory command returned empty or invalid output."
  exit 1
fi

#Converting tr181_memory and meminfo_memory to MB since they are in KB
tr181_memory_MB=$((tr181_memory / 1024))
meminfo_memory_MB=$((meminfo_memory / 1024))

echo "Memory value from tr181 command: $tr181_memory_MB MB"
echo "Memory value from /proc/meminfo command: $meminfo_memory_MB MB"

#Calculate the difference
difference=$(awk -v mem1="$tr181_memory_MB" -v mem2="$meminfo_memory_MB" 'BEGIN {print int(mem1 - mem2) < 0 ? mem2 - mem1 : mem1 - mem2}')

#Comparing the difference with the threshold
if ((difference <= FREE_MEMORY_THRESHOLD)); then
    echo "SUCCESS: Memory values are within specified threshold limit."
else
    echo "FAILURE: Memory values exceeds the expected threshold limit."
fi
}

if [ "$1" = "all" ]; then
  #Iterating dynamically through test case number
  max_test_cases=0
  i=1
  until [ "$(type -t "testcase$i")" != "function" ]; do
    test_case_function="testcase$i"
    max_test_cases=$((i - 1))
    echo "Running testcase : $i"
    if [ "$i" = "15" ]; then
       start_testcase_15
       disown
    else
      #sourcing for every function since for running with all there is error in fetching values
      source "$config_file"     
      "$test_case_function"
    fi
    echo " "
    i=$((i + 1))
  done
elif [ $# -gt 0 ]; then
  #Looping through each provided test case number
  for test_case_number in "$@"; do
    test_case_function="testcase$test_case_number"
    echo "Running testcase : $@"
    if [ "$(type -t "$test_case_function")" = "function" ]; then
      if [ "$test_case_number" = "15" ]; then
        start_testcase_15
        disown
      else
        #sourcing for every function since for running with multiple arguments there is error in fetching values
        source "$config_file"   
        "$test_case_function"
      fi
      echo " "
    else
      echo "Test case number: $test_case_number doesn't exist"
      print_help
    fi
  done
else
  print_help
fi

