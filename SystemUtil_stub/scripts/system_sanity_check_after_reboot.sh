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
#Array for getting count of failed plugins
FAILEDPLUGINS=()
#logfile where pre and post reboot actions are captured
logfile="$(find / -name "sanity_test_post_reboot_status.log"| head -n 1)"
# Redirecting all output to the log file
exec >> "$logfile"

# Created an array to store the post-reboot status of plugins
post_reboot_status=()

echo "*******************************************************"
echo "Plugins status after reboot"
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
        post_reboot_status+=("$plugin=$status")
      else
        status="enabled"
        post_reboot_status+=("$plugin=$status")
      fi
    fi
fi
done

#After reboot Pre-reboot status array is not fetched so grepping the array from logfile
read -a pre_reboot_status_array <<< "$(sed -n 's/Pre-reboot status : //p' "$logfile")"

echo "*******************************************************"
echo "Post-reboot status : ${post_reboot_status[*]}"
echo "*******************************************************"

# Initialized four indexed arrays in the form of map
declare -a post_reboot_status_keys
declare -a post_reboot_status_values
declare -a pre_reboot_status_keys
declare -a pre_reboot_status_values

# Populated the two indexed arrays with key-value pairs
for i in "${!post_reboot_status[@]}"; do
    post_reboot_status_keys[$i]=$(echo "${post_reboot_status[$i]}" | cut -d'=' -f1)
    post_reboot_status_values[$i]=$(echo "${post_reboot_status[$i]}" | cut -d'=' -f2)
    pre_reboot_status_keys[$i]=$(echo "${pre_reboot_status_array[$i]}" | cut -d'=' -f1)
    pre_reboot_status_values[$i]=$(echo "${pre_reboot_status_array[$i]}" | cut -d'=' -f2)
done

# Compare the two indexed arrays using key-value pairs
for key in "${!post_reboot_status_keys[@]}"; do
    if [[ ${post_reboot_status_values[$key]} != ${pre_reboot_status_values[$key]} ]]; then
        echo "${post_reboot_status_keys[$key]} status changed from ${pre_reboot_status_values[$key]} to ${post_reboot_status_values[$key]}"
        FAILEDPLUGINS+=(${post_reboot_status_keys[$key]})
    fi
done

echo "*******************************************************"
#Printing Failed Parameters
if [[ ${#FAILEDPLUGINS[@]} -gt 0 ]];then
  echo "FAILURE: Pre-reboot status and post-reboot status are different"
else
  echo "SUCCESS: Pre-reboot status and post-reboot status are Same"
fi
echo "*******************************************************"


#***************************************************************Test-Script-02**************************************************
#Test Scenario:To validate whether if any corefiles are generated after sometime of reboot
# Set the time limit for the script to be executed after 2 minutes of reboot
wait_time=120
# Getting the current time
start_time=$(date +%s)
echo "**********************Checking if any corefiles are generated after sometime of reboot************************************"
# Find all core dump files in the directory
core_files=$(find /mnt/memory/corefiles -type f)

# Loop until wait_time is reached or core dump files are found
until [ -n "$core_files" ] || [ $(($(date +%s)-start_time)) -gt $wait_time ]; do
  echo "No core dump files found. Sleeping for 10 seconds..."
  sleep 10
  core_files=$(find /mnt/memory/corefiles -type f)
done

# Check if core dump files were found
if [ -n "$core_files" ]; then
  for core_file in $core_files; do
    pid=$(echo "$core_file" | sed 's/.*\.\([0-9]*\)_core.*/\1/')
    prog=$(echo "$core_file" | sed 's/.*_core\.\([^\.]*\).*/\1/')
    signal=$(echo "$core_file" | sed 's/.*signal_\([0-9]*\)$/\1/')
    echo "Core dump file found: $core_file"
    echo "  PID: $pid"
    echo "  prog: $prog"
    echo "  Signal: $signal"
  done
else
  echo "No core dump files found within the time limit of $wait_time seconds."
fi
#*********************Removing the script from startup job**********************
#Navigating to /etc/init.d/ directory -- Below commands should be executed from init.d folder only
cd /etc/init.d/
# Remove the script from startup job list
update-rc.d -f system_sanity_check_after_reboot.sh remove
#Delete the script from /etc/init.d directory
rm -rf /etc/init.d/system_sanity_check_after_reboot.sh
if [ -f "system_sanity_check_after_reboot.sh" ]; then
  echo "Failed to remove the script from init.d folder"
else
  echo "Successfully removed the script from init.d folder"
fi