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
#Q1: How to execute all test cases in the shell script?
#A1: To execute all test cases in the shell script, need to run the script with the full name and the "all" argument. 
#Eg: sh system_sanity_check_before_reboot.sh all
#Q2: How to execute specific test case(s) in the shell script?
#A2: To execute specific test case(s) in the shell script, need to run the script with the full name followed by the test case number(s).
#Eg: sh system_sanity_check_before_reboot.sh 1 | system_sanity_check_before_reboot.sh 1 2
#Objective: To run testcases where reboot is involved as a teststep. 

#Deleting if any previous logfiles are present
find / -name "system_sanity_test_reboot_testcase.log" -type f -exec rm -f {} + 2>/dev/null

#Read the config file
config_file="$(dirname "$0")/sanity_check.config"
if [ ! -f "$config_file" ];then
echo "Please place the config file "$config_file" in the path and re-execute script"
exit 1
fi
source "$config_file"

#logfile where pre reboot actions are captured
#Check if LOGFILE_PATH is configured
if [ -z "$LOGFILE_PATH" ]; then
    echo "Error: LOGFILE_PATH not set, Pls configure and re-run"
    exit 1
fi
logfile="/$LOGFILE_PATH/system_sanity_test_reboot_testcase.log"
#clear the logfile if any old logs are present
> "$logfile"
#Redirecting all output to the log file
exec >> "$logfile"


config_file_path=$(cd "$(dirname "$0")" && pwd)
echo "config file path : $config_file_path"

date
echo -e "Executing script : System_Sanity_Check_before_reboot.sh"
echo -e "======================================================="

#Function to print the help message
print_help() {
  echo " "
  echo "Basic Sanity REBOOT TESTCASES Help"
  echo " "
  echo "Note: To execute all testcases available in this script: sh system_sanity_check_before_reboot.sh all"
  echo "Note: To execute specific testcase/testcases from this script: sh system_sanity_check_before_reboot.sh 1 | sh system_sanity_check_before_reboot.sh 1 2"
  echo "Test cases:"
  echo "  1: Autostart:true Plugins status before and after reboot."
  echo "  2: Core file check after reboot"
  echo "  3: Storage check before and after reboot"
  exit 0
}

echo "*******************************************************"

#Check for post reboot script
post_reboot_script="$(dirname "$0")/system_sanity_check_after_reboot.sh"
if [ ! -f "$post_reboot_script" ];then
echo "Please place the post reboot script "$post_reboot_script" in the path and re-execute script"
exit 1
fi

#**********************Configuring the script to autostart after reboot************************************
# Copy the "after_reboot.sh" script to "/etc/init.d directory to autostart the second script after reboot"
cp $(dirname "$0")/system_sanity_check_after_reboot.sh /etc/init.d
# Set the executable permission for second script: after_reboot.sh
chmod +x /etc/init.d/system_sanity_check_after_reboot.sh
# update the startup job list "after_reboot.sh"
update-rc.d system_sanity_check_after_reboot.sh defaults
echo "*******************************************************"

echo "*********************************************************************************************************"
#Save the test case arguments to an array and redirect to logfile
test_case_numbers=("$@")
echo "Test case arguments: ${test_case_numbers[*]}"  >> "$logfile"
echo "*********************************************************************************************************"

testcase1() {
#Test Case Name     : Autostart:true Plugins status before reboot
#Test Objective     : To validate Status of Plugins before and after reboot.
#Automation_approach: List out the plugins for which autostart:true in /plugin directory and validate Status of Plugins before and after reboot.
#Expected Output    : SUCCESS if plugin status are same before and after reboot.


echo "*******************************************************"
echo "Test Case Name : Autostart:true Plugins status before reboot"

#Created an array to store the pre-reboot status of plugins
pre_reboot_status=()

echo "*******************************************************"
echo "Plugins status before reboot"
echo "*******************************************************"


#Loop through all the JSON files in the directory
for file in /etc/WPEFramework/plugins/*.json
do
#Check if the file exists
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
}

testcase2 () {
#Test Case Name     : Core files check before reboot
#Test Objective     : To check if any core files are generated after sometime in DUT.
#Automation_approach: Report if any corefiles are generated.
#Expected Output    : Display if corefiles are generated within time limit specified.
echo "*******************************************************"
echo "Test Case Name : Core files check before reboot"
echo "No operations are performed before reboot"
echo "*******************************************************"
}

testcase3 () {
#Test Case Name     : Storage fill check before reboot
#Test Objective     : To Fill the storage of specified path to maximum and check for functionalities of logfile and apps.
#Automation_approach: Filling a specified path to its maximum storage capacity and test log file and apps functionalities.
#Expected Output    : Success if logfiles and apps are working after storage is full.
echo "*******************************************************"
echo "Test Case Name : Storage fill check before reboot"
echo "*******************************************************"
#Check if max_usage_percentage,destination_path,download_url,log_file are provided in logfile
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

#Get the usage percentage (without percentage sign)
usage_percentage_before_copying=$(df -hP / | awk 'NR==2 {gsub(/%/, "", $5); print $5}')

echo "*********************************************************************************************************"
echo "Before Copying Files: "$destination_path" | Usage Percentage: $usage_percentage_before_copying"
echo "*********************************************************************************************************"

if curl -I "$download_url" -o /dev/null -w '%{http_code}\n' -s | grep -q '^200$' && [ "$usage_percentage_before_copying" -lt "$max_usage_percentage" ]; then
  echo "Link is working and accessible. Starting download..."
  curl -L "$download_url" -o "$destination_path/$filename"
else
  echo "Link is not accessible or returned an error, or the usage percentage exceeds the limit."
  return 1
fi


while true; do
  #Check the current disk usage percentage of the root filesystem
  usage_percentage=$(df -hP / | awk 'NR==2 {gsub(/%/, "", $5); print $5}')
  
  #Download the file and copy it to a new destination with a timestamp in the filename
  if ((usage_percentage < max_usage_percentage)); then
    new_destination_filename="tdk_file.$(date +%s)"
    cp "$destination_path/$filename" "$destination_path/$new_destination_filename"
  elif ((usage_percentage == max_usage_percentage)); then
    echo "Usage percentage increased after downloading and copying the file."
    break
  else
    echo "Download or Copying Failed"
    echo "FAILURE: Usage percentage did not increase after downloading and copying the file."
    return 1
  fi

done

#Get the usage percentage
usage_percentage_after_copying=$(df -hP / | awk 'NR==2 {gsub(/%/, "", $5); print $5}')
echo "*********************************************************************************************************"
echo "After Copying Files: "$destination_path" | Usage Percentage: $usage_percentage_after_copying"
echo "*********************************************************************************************************"

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
#Check proccess of apps whether they are running fine
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

}

#Check if "all" is provided as an argument
if [ "$1" = "all" ]; then
  #Iterating dynamically through test case number
  max_test_cases=0
  i=1
  until [ "$(type -t "testcase$i")" != "function" ]; do
    test_case_function="testcase$i"
    max_test_cases=$((i - 1))
     echo " "
    "$test_case_function"
     echo " "
    i=$((i + 1))
  done
elif [ $# -gt 0 ]; then
  #Looping through each provided test case number
  for test_case_number in "$@"; do
    test_case_function="testcase$test_case_number"
    if [ "$(type -t "$test_case_function")" = "function" ]; then
       echo " "
      "$test_case_function"
       echo " "
    else
      echo "Test case number: $test_case_number doesn't exist"
      print_help
    fi
  done
else
  print_help
fi

echo "*******************************************************"
echo "$(date)-Rebooting the device..."
echo "*******************************************************"
reboot


