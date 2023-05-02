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
#Test Scenario:To validate Status of Plugins before and after reboot
#logfile where pre and post reboot actions are captured
logfile="$(dirname "$0")/sanity_test_post_reboot_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile"


# Created an array to store the pre-reboot status of plugins
pre_reboot_status=()

echo "*******************************************************"
echo "Plugins status before reboot"
echo "*******************************************************"


# Loop through all the JSON files in the directory
for file in /etc/WPEFramework/plugins/*.json
do
# Check if the file exists
if [ -f "$file" ]; then
    # Extract the plugin name from the file name
    plugin=$(basename "$file" .json)

    # Check if the plugin is enabled in autostart
    autostart=$(grep -q "\"autostart\":\s*true" "$file" && echo "true" || echo "false")
    # If the plugin is enabled in autostart, execute the curl command
    if [ "$autostart" == "true" ]; then
      #Check the result using curl response
      result=$(curl --silent --header "Content-Type: application/json" --request POST --data "{\"jsonrpc\":\"2.0\",\"id\":
      \"3\",\"method\": \"Controller.1.status@$plugin\"}" http://127.0.0.1:9998/jsonrpc)
      if [[ "$result" == *"deactivated"* ]]; then
        status="disabled"
        pre_reboot_status+=("$plugin=$status")
      else
        status="enabled"
        pre_reboot_status+=("$plugin=$status")
      fi
    fi
fi
done


echo "*******************************************************"
echo "Pre-reboot status : ${pre_reboot_status[*]}"
echo "*******************************************************"

#**********************Configuring the script to autostart after reboot************************************
# Copy the "after_reboot.sh" script to "/etc/init.d directory to autostart the second script after reboot"
cp $(dirname "$0")/system_sanity_check_after_reboot.sh /etc/init.d
# Set the executable permission for second script: after_reboot.sh
chmod +x /etc/init.d/system_sanity_check_after_reboot.sh
# update the startup job list "after_reboot.sh"
update-rc.d system_sanity_check_after_reboot.sh defaults

echo "*******************************************************"
echo "$(date)-Rebooting the device..."
echo "*******************************************************"
reboot
