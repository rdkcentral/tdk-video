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

# Load the AppArmor profiles from the configuration file
source /opt/apparmor_profile.config

# Redirect output logs to logfile
exec >> /opt/apparmor_profile_status.log 2>&1

# Waiting for 30 seconds to allow the system to initialize after reboot
sleep 30

# Loop through each profile and check the AppArmor status after reboot
for profile in "${profiles[@]}"
do
    echo "$(date) - $profile status after reboot:"
    # Print the status
    ps -auxZ | grep "$profile"
    # Validate the status after running the test whether it's in Disable mode
    output=$(ps -auxZ | grep "$profile")
    # Check if the output contains the string "$profile (unconfined)"
    if ps -auxZ | grep "$profile" | grep -q "unconfined" ; then
      echo "SUCCESS: $profile profile is in disable mode"
    else
      echo "FAILURE: $profile profile not set in disable mode"
	  exit 1
    fi
done


echo "Checking log for the Denied message in /opt/logs/startup_stdout_log.txt"
denied_count=$(grep -c "DENIED" /opt/logs/startup_stdout_log.txt)
echo "Number of Denied messages found: $denied_count"

# Verify that Denied message count for the process is one
if [ $denied_count -eq 1 ]; then
    echo "Success: Denied message count is one"
elif [ $denied_count -eq 0 ]; then
    echo "No Denied message found in the log"
else
    echo "Error: Multiple Denied messages found in the log"
fi

#*********************Removing the script from startup job**********************
#Navigating to /etc/init.d/ directory -- Below commands should be executed from init.d folder only
cd /etc/init.d/
# Remove the script from startup job list
update-rc.d -f validate_logs_for_denied_message.sh remove
#Delete the script from /etc/init.d directory
rm -rf /etc/init.d/validate_logs_for_denied_message.sh
if [ -f "validate_logs_for_denied_message.sh" ]; then
  echo "Failed to remove the script from init.d folder"
else
  echo "Successfully removed the script from init.d folder"
fi
