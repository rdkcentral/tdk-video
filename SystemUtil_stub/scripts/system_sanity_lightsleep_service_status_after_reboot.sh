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
#Test Objective     : To validate the status of services running after reboot because services status were changed after lightsleep.
#Automation_approch : To check status of service after reboot with pre-lightsleep services.
#ExpectedOutput     : Validating status of services after reboot.

#sleep is given based on averaging service time taken to start
sleep 180

#logfile where pre and post reboot actions are captured
logfile_path=$(find / -name "system_sanity_lightsleep_service_status.log" 2>/dev/null | head -n 1)
logfile=$logfile_path
# Temporary file to store pre_lightsleep_services
temp_file_path=$(find / -name "pre_lightsleep_services.tmp" 2>/dev/null | head -n 1)
temp_file=$temp_file_path

#Timeout is given to wait for sometime in until loop if service is not active
timeout=40

# Redirecting all output to the log file
exec >> "$logfile"
FAILED_SERVICES=()

#Function to check if a pre reboot service is active
is_service_active() {
  systemctl is-active "$1" >/dev/null 2>&1
}

# Compare services before and after reboot
while IFS= read -r service; do
  if is_service_active "$service"; then
    continue
  fi
  SECONDS=0
  until is_service_active "$service"; do
    if [[ SECONDS -gt $timeout ]]; then
      FAILED_SERVICES+=("$service")
      break
    fi
    # Waiting for service to start after the device is up
    sleep 5
    ((SECONDS+=5))
  done
done < "$temp_file"

# Check if any services have failed after reboot
echo "Services status after reboot"
if [ ${#FAILED_SERVICES[@]} -eq 0 ]; then
  echo "SUCCESS: Services Status are same pre-lightsleep and post-reboot"
else
  # Restart the failed services
  for failed_service in "${FAILED_SERVICES[@]}"; do
    echo "Timeout exceeded ----| $failed_service |---- did not become active."
    systemctl restart "$failed_service"
    echo " Status after Restarting "$failed_service": "
    systemctl status "$failed_service" | grep "Active:"
  done
  echo "FAILURE: Pre-lightsleep Services and Post-reboot Services are different"
  echo "********Restarted Systemctl of failed services,Pls check the status and proceed********"
fi

#*********************Removing the script from startup job**********************
#Navigating to /etc/init.d/ directory -- Below commands should be executed from init.d folder only
cd /etc/init.d/
#Remove the script from startup job list
update-rc.d -f system_sanity_lightsleep_service_status_after_reboot.sh remove
#Delete the script from /etc/init.d directory
rm -rf /etc/init.d/system_sanity_lightsleep_service_status_after_reboot.sh
if [ -f "system_sanity_lightsleep_service_status_after_reboot.sh" ]; then
  echo "Failed to remove the script from init.d folder"
else
  echo "Successfully removed the script from init.d folder"
fi
#Remove the temp file created in execution
rm -rf "$temp_file"
if [ -f "$temp_file" ]; then
  echo "Failed to remove the temp file"
else
  echo "Successfully removed the temp file"
fi

