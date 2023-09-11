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
#Test Objective     : To validate the status of services running before and after lightsleep.
#Automation_approch : To check status of service before and after lightsleep.If status vary reboot the dut and check the status.
#ExpectedOutput     : Validating status of services before and after lightsleep.

#Deleting if any previous logfiles are present
find / -name "system_sanity_lightsleep_service_status.log" -type f -exec rm -f {} + 2>/dev/null

#Read the config file
config_file="$(dirname "$0")/sanity_check.config"
if [ ! -f "$config_file" ];then
echo "Please place the config file "$config_file" in the path and re-execute script"
exit 1
fi
source "$config_file"

#logfile where pre and post lightsleep actions are captured
#Check if LOGFILE_PATH is configured
if [ -z "$LOGFILE_PATH" ]; then
    echo "Error: LOGFILE_PATH not set, Pls configure and re-run"
    exit 1
fi
logfile="/$LOGFILE_PATH/system_sanity_lightsleep_service_status.log"
#clear the logfile
> "$logfile"
#Redirecting output to the log file
exec >> "$logfile"

#Function to validate mode
validate_mode() {
    output="$1"
    expected_mode="$2"
    if [ "$output" = "$expected_mode" ]; then
        echo "$expected_mode MODE"
    else
        echo "FAILURE: Error: Not in expected $expected_mode Mode"
        rm -rf "$temp_file" && echo "Successfully removed the temp file" || echo "Failed to remove the temp file"
        exit 1
    fi
}

#Array to store failed and active services
FAILED_SERVICES=()
ACTIVE_SERVICES=()

#Temporary file to store pre_lightsleep_services
temp_file="$(dirname "$0")/pre_lightsleep_services.tmp"

#Services running before lightsleep
systemctl --type=service --state=running --no-legend | awk '{print $1}' | grep -v "dropbear" > "$temp_file"
pre_lightsleep_services=($(cat "$temp_file"))

echo "*******************************************************"
count_before_lightsleep=${#pre_lightsleep_services[@]}
echo "Checking services before lightsleep"
echo -e "Number of services: $count_before_lightsleep \n"
echo "-------------------------------------------------"
for before_lightsleep in "${pre_lightsleep_services[@]}"; do
  echo "| $before_lightsleep |"
done
echo "-------------------------------------------------"

echo "*******************************************************"
echo "Setting the Device into lightsleep mode: "
SetPowerState LIGHTSLEEP
echo -e "\n"

echo "Verifying the Powerstate mode: "
output=$(QueryPowerState)
validate_mode "$output" "LIGHTSLEEP"
echo -e "\n"

echo "Reverting the lightsleep mode to normal: "
SetPowerState ON
echo -e "\n"

echo "Verifying the Powerstate mode: "
output=$(QueryPowerState)
validate_mode "$output" "ON"
echo -e "\n"
echo "*******************************************************"

#Function to check if a pre lightsleep service is active
is_service_active() {
  systemctl is-active "$1" >/dev/null 2>&1
}
#Compare services before and after lightsleep
while IFS= read -r service; do
    if ! is_service_active "$service"; then
      FAILED_SERVICES+=("$service")
    else
      ACTIVE_SERVICES+=("$service")
    fi
done < $temp_file

echo "*******************************************************"
count_after_lightsleep=${#ACTIVE_SERVICES[@]}
echo "Checking services status after lightsleep"
echo -e "Number of services active: $count_after_lightsleep \n"
echo "-------------------------------------------------"
for after_lightsleep in "${ACTIVE_SERVICES[@]}"; do
  echo "| $after_lightsleep |"
done
echo "-------------------------------------------------"
echo "*******************************************************"

# Checking if services are failed after lightsleep
if [ ${#FAILED_SERVICES[@]} -eq 0 ]; then
  echo "SUCCESS: Status of Services are same"
  #Remove the temp file created in execution
  rm -rf "$temp_file" && echo "Successfully removed the temp file" || echo "Failed to remove the temp file"
else
  echo "FAILURE: Pre-lightsleep status and Post-lightsleep status are different"
  echo "The following services have failed after lightsleep:"
  count_failed_services=${#FAILED_SERVICES[@]}
  echo -e "Number of services failed: $count_failed_services \n"
  echo "-------------------------------------------------"
  for failed_services in "${FAILED_SERVICES[@]}"
  do
     echo "| $failed_services |"
  done
  echo "-------------------------------------------------"

  #**********************Configuring the script to autostart after reboot becase service status are not same after lighsleep************************************
  # Copy the "after_reboot.sh" script to "/etc/init.d directory to autostart the second script after reboot"
  cp "$(dirname "$0")"/system_sanity_lightsleep_service_status_after_reboot.sh /etc/init.d
  # Set the executable permission for second script: after_reboot.sh
  chmod +x /etc/init.d/system_sanity_lightsleep_service_status_after_reboot.sh
  # update the startup job list "after_reboot.sh"
  update-rc.d system_sanity_lightsleep_service_status_after_reboot.sh defaults
  echo "$(date)-Rebooting the device since the services running are not same pre and post lightsleep..."
  reboot
fi

