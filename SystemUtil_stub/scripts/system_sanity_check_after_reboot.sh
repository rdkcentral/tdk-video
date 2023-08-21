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
#logfile where post reboot actions are captured
logfile="$(find / -name "system_sanity_test_reboot_testcase.log" -type f 2>/dev/null | head -n1 )"
# Redirecting all output to the log file
exec >> "$logfile"

#Extract the pwd of config file from the log file
config_file_path=$(grep "config file path" "$logfile" | awk -F ': ' '{print $2}')
#Read the config file
config_file="$config_file_path/sanity_check.config"
if [ ! -f $config_file ];then
  echo "Please place the config file $config_file in the before reboot Parent script path and re-execute script"
  exit 1
fi
source $config_file


testcase1 () {
#Test Case Name     : Autostart:true Plugins status after reboot
#Test Objective     : To validate Status of Plugins before and after reboot.
#Automation_approach: List out the plugins for which autostart:true in /plugin directory and validate Status of Plugins before and after reboot.
#Expected Output    : SUCCESS if plugin status are same before and after reboot.

echo "*******************************************************"
echo "Test Case Name : Autostart:true Plugins status after reboot"
echo "*******************************************************"

#Array for getting count of failed plugins
FAILEDPLUGINS=()
#Created an array to store the post-reboot status of plugins
post_reboot_status=()

echo "*******************************************************"
echo "Plugins status after reboot"
echo "*******************************************************"

#Loop through all the JSON files in the directory
for file in /etc/WPEFramework/plugins/*.json
do
#Check if the file exists
if [ -f "$file" ]; then
    #Extract the plugin name from the file name
    plugin=$(basename "$file" .json)

    #Check if the plugin is enabled in autostart
    autostart=$(grep -q "\"autostart\":\s*true" "$file" && echo "true" || echo "false")
    #If the plugin is enabled in autostart, execute the curl command
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
read -r -a pre_reboot_status_array <<< "$(sed -n 's/Pre-reboot status : //p' "$logfile")"

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
}

testcase2 () {
#Test Case Name     : Core files check after reboot
#Test Objective     : To check if any core files are generated after sometime in DUT.
#Automation_approach: Report if any corefiles are generated.
#Expected Output    : Display if corefiles are generated within timelimit specified.

echo "*******************************************************"
echo "Test Case Name : Core file check after reboot"
echo "**********************Checking if any corefiles are generated after sometime of reboot************************************"
# Find all core dump files in the directory
core_files=$(find /mnt/memory/corefiles -type f)

#Setting the time limit for the script to be executed after 2 minutes of reboot
SECONDS=0
timeout=120
#Loop until timeout is reached or core dump files are found
until [ -n "$core_files" ] || [[ SECONDS -gt timeout ]]; do
  echo "No core dump files found. Sleeping for 10 seconds..."
  sleep 10
  core_files=$(find /mnt/memory/corefiles -type f)
done

#Check if core dump files were found
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
  echo "FAILURE: Core dump files found within the time limit of $timeout seconds."
else
  echo "SUCCESS: No core dump files found within the time limit of $timeout seconds."
fi
}
testcase3 () {
#***************************************************************Test-Script-03**************************************************
# Test Case Name     : Storage fill check after reboot
# Test Objective     : To Fill the storage of specified path to maximun and check for functionalities of logfile and apps.
# Automation_approach: Filling a specified path to its maximum storage capacity and test log file and apps functionalities.
# Expected Output    : Success if app functionalities and log file writing are working properly.

echo "*******************************************************"
echo "Test Case Name : Storage fill check after reboot"
echo "*******************************************************"
#Check if max_usage_percentage,destination_path,download_url,log_file are provided to logfile
missing_parameters=()

[ -z "$max_usage_percentage" ] && missing_parameters+=("MAX USAGE PERCENTAGE")
[ -z "$destination_path" ] && missing_parameters+=("DESTINATION PATH")
[ -z "$download_url" ] && missing_parameters+=("DOWNLOAD URL")
[ -z "$log_file" ] && missing_parameters+=("LOG FILE")

if [ ${#missing_parameters[@]} -gt 0 ]; then
  echo "Error: ${missing_parameters[*]} not found in config file.Pls configure and re-run."
  return 1
fi

#Extract the filename from the download URL
filename=$(basename "$download_url")
#Get the usage percentage of the root filesystem (without percentage sign)
usage_percentage_after_reboot=$(df -hP / | awk 'NR==2 {gsub(/%/, "", $5); print $5}')
echo "*********************************************************************************************************"
echo "After Reboot: "$destination_path" | Usage Percentage After Reboot: $usage_percentage_after_reboot"
echo "*********************************************************************************************************"

#Check if the usage percentage has increased after downloading and copying the file
if [ "$max_usage_percentage" -eq "$usage_percentage_after_reboot" ]; then
    echo "Usage percentage increased after downloading and copying the file."
else
  echo "FAILURE: Usage percentage did not increase after downloading and copying the file."
  return 1
fi

timestamp="$(date +"%Y-%m-%d %H:%M:%S")"
timestamp_for_validation+=("$timestamp")

#Writing random logs to wpeframework.log to validate its functionality after reboot
echo "$timestamp_for_validation Writing data to wpeframework for validating $log_file after storage fill" >> "$log_file"

echo "*********************************************************************************************************"
#Check if the written data is present in the log file
if grep -qi "$timestamp_for_validation Writing data to wpeframework for validating $log_file after storage fill" "$log_file"; then
  echo "SUCCESS: Logs are writing into $log_file even after the memory is equal to its maximum capacity"
else
  echo "FAILURE: Logs are not writing into $log_file after the memory is equal to its maximum capacity"
fi
echo "*********************************************************************************************************"

echo "*********************************************************************************************************"
#Check process of listed apps whether they are running as expected
failed_processes=()
excluded_processes='sh <defunct>|sh|sleep|systemd|kworker|ps|grep'
output_apps=$(ps -e -o comm | grep -v -E "$excluded_processes")

#Check if any of the filtered processes are not running
failed_processes=""
for app in "${failed_apps_array[@]}"; do
  if ! pgrep -f "$app" &> /dev/null; then
    failed_processes+=" $app"
  fi
done

if [ -z "$failed_processes" ]; then
  echo "SUCCESS: All listed apps processes are running as expected"
else
  echo "Failed processes:$failed_processes"
fi
echo "*********************************************************************************************************"


#Deleting the downloaded file
if [ -f "$destination_path/$filename" ] || [ -n "$(find "$destination_path" -maxdepth 1 -name "$new_destination_filename.*" -type f)" ]; then
  rm -f "$destination_path/$filename"
  rm -f "$destination_path/$new_destination_filename".*
  echo "Downloaded and copied files removed successfully"
else
  echo "Failed to delete the downloaded and copied files"
fi
#Get the usage percentage of the destination path (without percentage sign)
usage_percentage_after_removing_files=$(df -hP "$destination_path" | awk 'NR==2 {gsub(/%/, "", $5); print $5}')
echo "*********************************************************************************************************"
echo "After deleting files: Destination Path: $destination_path | Usage Percentage: $usage_percentage_after_removing_files"
echo "*********************************************************************************************************"

}

run_test_cases() {
  if [ -f "$logfile" ]; then
    #Initialized a flag to determine whether to run all test cases or not
    run_all_test_cases=false

    test_case_numbers=()
    #Check if the log file contains "Test case arguments: all"
    if grep -qi "Test case arguments: all" "$logfile"; then
      run_all_test_cases=true
    else
      while read -r line; do
        number=$(echo "$line" | cut -d ' ' -f 4)
        test_case_numbers+=("$number")
      done < <(grep -oE 'Test case arguments: [0-9]+' "$logfile")
    fi
    if [ "$run_all_test_cases" = true ]; then
      #Find all test case functions dynamically and execute them
      for test_case_function in $(compgen -A function | grep "^testcase[0-9]\+$"); do
        echo " "
        "$test_case_function"
        echo " "
      done
    elif [ ${#test_case_numbers[@]} -gt 0 ]; then
      #Loop through each test case number and execute the corresponding test case
      for test_case_number in "${test_case_numbers[@]}"; do
        test_case_function="testcase$test_case_number"
        if [ "$(type -t "$test_case_function")" = "function" ]; then
          echo " "
          "$test_case_function"
          echo " "
        else
          echo "Test case number: $test_case_number doesn't exist"
        fi
      done
    else
      echo "No test case numbers found in the log file."
    fi
  else
    echo "Log file not found: $logfile"
  fi
}

run_test_cases

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


