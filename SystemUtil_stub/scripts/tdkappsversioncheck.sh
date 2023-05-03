#!/bin/bash
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
echo -e "Executing script : version_validation"
echo -e "======================================="
echo -e "Test Execution Name is: version_validation"

# Read the config file
config_file="$(dirname "$0")/rdk6_version.config"
if [ ! -f $config_file ];then
echo "Please place the config file "$config_file" in the path and re-execute script"
exit 1
fi

source $config_file

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
if [ "$PARAMETER" = "kernel" ]; then
command=$(uname -r)
elif [ "$PARAMETER" == "YOCTO_VERSION" ]; then
command=$(grep "YOCTO_VERSION=" /version.txt | cut -d "=" -f2)
elif [ "$PARAMETER" == "Gstreamer" ]; then
command=$(gst-inspect-1.0 --version | sed -n 2p | cut -d " " -f2)
elif [ "$PARAMETER" == "wpa_supplicant" ]; then
command=$(wpa_supplicant -v | grep wpa_supplicant | cut -d ' ' -f2)
elif [ "$PARAMETER" == "Glibc" ]; then
command=$(ls -l /lib/libc.so.6 | cut -d '-' -f3)
elif [ "$PARAMETER" == "Openssl" ]; then
command=$(openssl version | cut -d " " -f2)
elif [ "$PARAMETER" == "dbus-daemon" ]; then
command=$(dbus-daemon --version | grep D-Bus | cut -d ' ' -f5)
elif [ "$PARAMETER" == "bluetoothctl" ]; then
command=$(bluetoothctl --version)
elif [ "$PARAMETER" == "systemd" ]; then
command=$( systemctl --version | grep "systemd" | cut -d " " -f2)
elif [ "$PARAMETER" == "apparmor" ]; then
command=$(apparmor_parser -V | grep "version" | cut -d " " -f4)
fi

echo "Version Specified in configuration file for RDK6:$RESULT & Response From DUT:$PARAMETER=$command"
VALUE=$(echo "$RESULT" | cut -d "=" -f2)
if [ "$VALUE" = "$command" ]; then
echo -e "Version Verification of $PARAMETER is Successful\n"
else
echo -e "Version Verification of $PARAMETER is Failed\n"
MISMATCHED+=( "$PARAMETER=$VALUE" )
fi
}
#Check if kernel version is matching with the Given value
Version_Validation "kernel"
#Check if YOCTO_VERSION version is matching with the specified value
Version_Validation "YOCTO_VERSION"
#Check if Gstreamer version is matching with the specified value
Version_Validation "Gstreamer"
#Check if wpa_supplicant version is matching with the specified value
Version_Validation "wpa_supplicant"
#Check if Glibc version is matching with the specified value
Version_Validation "Glibc"
#Check if Openssl version is matching with the specified value
Version_Validation "Openssl"
#Check if dbus-daemon version is matching with the specified value
Version_Validation "dbus-daemon"
#Check if bluetoothctl version is matching with the specified value
Version_Validation "bluetoothctl"
#Check if systemd version is matching with the specified value
Version_Validation "systemd"
#Check if bluetoothctl version is matching with the specified value
Version_Validation "apparmor"

#Printing Summary
if [ ${#MISMATCHED[@]} -gt 0 ];then
echo "FAILURE: FailedParameters:{${MISMATCHED[*]}}" | sed 's/[[:blank:]]/,/g'
else
echo "SUCCESS: All parameter versions are Matched"
fi
echo "Script completed ..."