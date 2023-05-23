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
function param_help {
echo "------------------------------------"
echo "TDK Apparmor Testcase Help Page"
echo "------------------------------------"
echo "TDK_Apparmor_Test_01_Service_Status  aptest01"
echo "TDK_Apparmor_Test_02_Enable_Status   aptest02"
echo "TDK_Apparmor_Test_03_Blocklist       aptest03"
echo "TDK_Apparmor_Test_04_Binary_Profile  aptest04"
echo "TDK_Apparmor_Test_05_Profile_Check   aptest05"
echo "TDK_Apparmor_Test_06_WriteExe_Check  aptest06"
echo "TDK_Apparmor_Test_07_Capability_check aptest07"
echo "TDK_Apparmor_Test_08_No_Profile_check aptest08"
echo "TDK_Apparmor_Test_09_Complain_Reboot aptest09"
echo "TDK_Apparmor_Test_10_Enforce_Reboot aptest10"
echo "TDK_Apparmor_Test_11_Disable_Reboot aptest11"
echo "TDK_Apparmor_Test_12_Complain_Enforce_Reboot aptest12"
echo "TDK_Apparmor_Test_13_Denied_Messages aptest13"
echo "TDK_Apparmor_Test_14_Default_Enforce_check aptest14"
echo "TDK_Apparmor_Test_15_Dac_Override_Remove_irMgrMain aptest15"
echo "TDK_Apparmor_Test_16_Dac_Override_Remove_default aptest16"
echo "TDK_Apparmor_Test_17_Dac_Override_lighttpd aptest17"
echo "TDK_Apparmor_Test_18_Dac_Override_audiocapturemgr aptest18"
echo "TDK_Apparmor_Test_19_Log_Check aptest19"

echo "Note: if 'all' is passed as test number, all tests get executed"
echo "e.g.: ./tdk_apparmor_tests.sh all"
}

source /opt/apparmor_profile.config
path=/etc/apparmor.d/


#service status check script to check status of the apparmor service
function service_status {
#echo "inside service status"

echo -e "Executing script : apparmor_service_status_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_01_Service_Status"
echo -e "Connected to $ip Box"
echo -e "Connected to Server!\n"

echo "Executing command : systemctl status apparmor" 
systemctl status apparmor | grep "Active" >/dev/details
echo "DETAILS"
echo "-------------------------"
if cat /dev/details | grep 'Active: active' >/dev/null
then
echo "SUCCESS:Apparmor service is active and running in DUT"
else
echo "FAILURE:Apparmor feature is inactive"
fi
}

#apparmor status check on the device enabled/disabled
function enable_armor {
echo -e "Executing script : apparmor_status_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_02_Enable_Status"
echo -e "Connected to $ip Box"
echo -e "Connected to Server!\n"

echo "Executing ExecuteCommand: aa-enabled" 
aa-enabled >/dev/details
echo "DETAILS"
echo "-------------------------"
if cat /dev/details |grep 'Yes'
then
echo "SUCCESS:Apparmor feature is enabled"
else
echo "FAILURE:Apparmor feature is not enabled"
fi

}

#check to see if the default apparmor blocklist file is present or not
function blocklist {
echo -e "Executing script : apparmor_blocklist_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_03_Blocklist"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

echo "To check Apparmor is enabled"
command=$(aa-enabled)

if [ $command == "Yes" ]
then
        echo "Apparmor is enabled"
        
        FILE=/opt/secure/Apparmor_blocklist
        echo "Filepath: $FILE"
	if [ -f "$FILE" ]; then
                echo "SUCCESS:$FILE exists."
        else
                echo "FAILURE:$FILE does not exist."
        fi
else
        echo "FAILURE:Apparmor is disabled"
fi
}

#check to see if binary profiles are present in /etc/apparmor.d/binaryprofiles directory
function bin_process {
echo -e "Executing script : apparmor_binary_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_04_Binary_Profile"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

echo "To check Apparmor is enabled"
command=$(aa-enabled)
echo "$command"
if [ $command == "Yes" ]
then
        echo "Apparmor is enabled"
                profile1=/etc/apparmor.d/binaryprofiles/usr.bin.tr69hostif
                profile2=/etc/apparmor.d/binaryprofiles/usr.lib.bluez5.bluetooth.bluetoothd
                echo "Check for Profiles: $profile1, $profile2"
                if ! [[ -f "$profile1" ]] && [[ -f "$profile2" ]]
                then
                        echo -e "SUCCESS:$profile1, $profile2 does not exist"
                else
                        echo -e "FAILURE: Binary Profiles exists"
                fi

else
        echo "FAILURE:Apparmor is disabled"
fi

}


#check to see if profiles are present in /etc/apparmor.d/ directory 
function profile_check {
echo -e "Executing script : apparmor_profile_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_05_Profile_Check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

echo "To check Apparmor is enabled"
command=$(aa-enabled)
echo "$command"
if [ $command == "Yes" ]
then
	echo "Apparmor is enabled"
	path=/etc/apparmor.d/ 
	for profile in "${profiles[@]}"
	do
	    echo "Profile: $profile"
	done

	# Loop through the profiles and validate their status before running the test
	for profile in "${profiles[@]}"; do
 
	  if [ -f "$path/$profile" ]; then
	    echo "SUCCESS: $profile exists"
	  else
	    echo "FAILURE: $profile is missing"
	    exit 1
  	fi

	done
else
        echo "FAILURE: Apparmor is disabled"
fi
}



#function write exe check
function write_exe_check {
echo -e "Executing script : apparmor_write_exe_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_06_Write_Exe_Check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

   for profile in "${profiles[@]}"; do
	cat $path/$profile | grep -i 'w.*x' >/dev/details
        
	if [ $(cat /dev/details|wc -l) -gt 0 ]; then
        	echo "FAILURE: $profile contains write and execute permission"
		exit
        else
        	echo "SUCCESS: $profile has no write and execute permission together"
        fi

    done

}


#function to check capabilities
function capability_check {
echo -e "Executing script : apparmor_capability_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_07_Capability_Check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

	for profile in "${profiles[@]}"; do
		cat $path/$profile | grep -i 'DAC_OVERRIDE\|MAC_ADMIN' >/dev/details

		if [ $(cat /dev/details|wc -l) -gt 0 ]; then
                	echo "FAILURE: Admin/Override Capability Roles are present in $profile"
                	exit
        	else
		echo "SUCCESS: Admin/Override Capability roles are not present in $profile"
		fi
	done
}



#function to check no profiles
function no_profiles_check {
echo -e "Executing script : apparmor_no_profiles_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_08_No_Profiles_Check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

echo -e "This testcase serves negative scenario requires profiles to be deleted before run to validate negative case"

if [ -d "/etc/apparmor.d" ]; then
	if [ "$(ls -A /etc/apparmor.d)" ]; then
	echo "FAILURE: apparmor profiles are present in /etc/apparmor.d"
	else
		echo "No apparmor profiles found in /etc/apparmor.d"
		
		echo -e "To check if apparmor is still active after removing the profiles from /etc/apparmor.d/ path..."
		systemctl status apparmor.service | grep "Active" >/dev/details
		echo "DETAILS"
		echo "-------------------------"
		if cat /dev/details | grep 'Active: active' >/dev/null
		then
		echo "SUCCESS:Apparmor service is active and running in DUT"
		else
		echo "FAILURE:Apparmor service is inactive"
		fi
	fi
else
	echo "FAILURE: /etc/apparmor.d directory not found."
fi
}

function call_reboot_complain {
echo -e "Executing script : call_reboot_complain"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_09_Complain_Reboot"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

#logfile where pre and post reboot actions are captured
logfile="/opt/apparmor_profile_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile" 2>&1

for profile in "${profiles[@]}"
do
    echo "Profile: $profile"
done

# Loop through the profiles and validate their status before running the test
for profile in "${profiles[@]}"; do
  echo "$(date)-Before running tr181 command for $profile:"
  date

  # Check if the output contains the string "$profile (complain)"
  if ps -auxZ | grep "$profile" | grep -q "(complain)"; then
    echo "$profile is already in complain mode, So running with an AppArmor profile already"
    exit 1
  else
    echo "Verified that $profile is not in complain mode"
  fi

  # To print the status
  ps -auxZ | grep "$profile"
done

# Waiting for 10 seconds to allow the system to initialize (if the script is executed immediately after reboot then it
# might take some time for the command to fetch the status)
sleep 10

# Loop through the profiles and run the tr181 command to set them to "complain" mode
for profile in "${profiles[@]}"; do
  tr181 -s -v "$profile:complain" Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.NonRootSupport.ApparmorBlocklist
  echo "$(date)-After running tr181 command to set $profile to complain mode and it will be reflected after reboot"
  date

  # To print the status
  ps -auxZ | grep "$profile"
done

#call reboot function
call_reboot_function apparmor_profile_change_after_reboot.sh
}

function call_reboot_enforce {
echo -e "Executing script : call_reboot_enforce"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_10_Enforce_Reboot"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

#logfile where pre and post reboot actions are captured
logfile="/opt/apparmor_profile_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile" 2>&1

for profile in "${profiles[@]}"
do
    echo "Profile: $profile"
done

# Loop through the profiles and validate their status before running the test
for profile in "${profiles[@]}"; do
  echo "$(date)-Before running tr181 command for $profile:"
  date

  # Check if the output contains the string "$profile (enforce)"
  if ps -auxZ | grep "$profile" | grep -q "(enforce)"; then
    echo "$profile is already in enforce mode, So running with an AppArmor profile already"
    exit 1
  else
    echo "Verified that $profile is not in enforce mode"
  fi

  # To print the status
  ps -auxZ | grep "$profile"
done

# Waiting for 10 seconds to allow the system to initialize (if the script is executed immediately after reboot then it
# might take some time for the command to fetch the status)
sleep 10

# Loop through the profiles and run the tr181 command to set them to "enforce" mode
for profile in "${profiles[@]}"; do
  tr181 -s -v "$profile:enforce" Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.NonRootSupport.ApparmorBlocklist
  echo "$(date)-After running tr181 command to set $profile to enforce mode and it will be reflected after reboot"
  date

  # To print the status
  ps -auxZ | grep "$profile"
done

#call reboot function
call_reboot_function apparmor_profile_change_after_reboot.sh
}

function call_reboot_disable {
echo -e "Executing script : call_reboot_disable"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_11_Disable_Reboot"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

#logfile where pre and post reboot actions are captured
logfile="/opt/apparmor_profile_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile" 2>&1

for profile in "${profiles[@]}"
do
    echo "Profile: $profile"
done

# Loop through the profiles and validate their status before running the test
for profile in "${profiles[@]}"; do
  echo "$(date)-Before running tr181 command for $profile:"
  date

  # Check if the output contains the string "$profile (null)"
  if ps -auxZ | grep "$profile" | grep -q "(null)"; then
    echo "$profile is already in disable mode, So running with an AppArmor profile already"
    exit 1
  else
    echo "Verified that $profile is not in disable mode"
  fi

  # To print the status
  ps -auxZ | grep "$profile"
done

# Waiting for 10 seconds to allow the system to initialize (if the script is executed immediately after reboot then it
# might take some time for the command to fetch the status)
sleep 10

# Loop through the profiles and run the tr181 command to set them to "disable" mode
for profile in "${profiles[@]}"; do
  tr181 -s -v "$profile:disable" Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.NonRootSupport.ApparmorBlocklist
  echo "$(date)-After running tr181 command to set $profile to disable mode and it will be reflected after reboot"
  date

  # To print the status
  ps -auxZ | grep "$profile"
done

#call reboot function
call_reboot_function apparmor_profile_change_after_reboot.sh
}

function call_reboot_complain_enforce {
echo -e "Executing script : call_reboot_complain_enforce"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_12_Complain_Enforce_Reboot"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

#logfile where pre and post reboot actions are captured
logfile="/opt/apparmor_profile_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile" 2>&1

# Check the AppArmor status of the profiles before changing to enforce mode
echo "$(date) - profiles status before changing to enforce mode:"

# Check if profiles are in complain mode
for profile in "${profiles[@]}"
do
  if ps -auxZ | grep "$profile" | grep -q "(complain)"; then
    echo "SUCCESS: $profile is running in complain mode"
  else
    echo "FAILURE: $profile not running in complain mode. Set $profile to complain mode."
	exit 1
  fi
  
  # To print the status
  ps -auxZ | grep "$profile"
done

# Waiting for 10 seconds to allow the system to initialize (if the script is executed immediately after reboot then it
# might take some time for the command to fetch the status)
sleep 10


# Change the profile to enforce mode
for profile in "${profiles[@]}"
do
  echo "$(date) - Changing $profile profile to enforce mode"
  tr181 -s -v "$profile:enforce" Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.NonRootSupport.ApparmorBlocklist
  echo "$(date)-After running tr181 command to set $profile to enforce mode and it will be reflected after reboot"
  date
  
  # To print the status
  ps -auxZ | grep "$profile"
done

#call reboot function
call_reboot_function validate_complain_to_enforce.sh
}

function call_reboot_denied_messages {
echo -e "Executing script : call_reboot_denied_messages"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_13_Denied_Messages"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

#logfile where pre and post reboot actions are captured
logfile="/opt/apparmor_profile_status.log"
#clear the logfile
> "$logfile"
# Redirecting all output to the log file
exec >> "$logfile" 2>&1

#check whether apparmor is enabled 
if aa-enabled | grep -q "Yes"; then
  echo "AppArmor is enabled"
else
  echo "AppArmor is not enabled"
  exit 1
fi

# Waiting for 10 seconds to allow the system to initialize (if the script is executed immediately after reboot then it
# might take some time for the command to fetch the status)
sleep 10


# Set profiles to disable mode
for profile in "${profiles[@]}"
do
  echo "$(date) - Changing $profile profile to disable mode"
  tr181 -s -v "$profile:disable" Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.NonRootSupport.ApparmorBlocklist
  echo "$(date)-After running tr181 command to set $profile to disable mode and it will be reflected after reboot"
  date
done

#call reboot function
call_reboot_function validate_logs_for_denied_message.sh
}

function call_reboot_function {
#**********************Configuring the script to autostart after reboot************************************
# Copy the validate script to "/etc/init.d directory to autostart the second script after reboot"
echo "inside call_reboot_function: $1"
fname=$1

cp $(dirname "$0")/"$fname" /etc/init.d
# Set the executable permission for second script
chmod +x /etc/init.d/"$fname"
# update the startup job list validate script"
update-rc.d "$fname" defaults

#********************Reboot******************
echo "$(date)- Rebooting the device..."
reboot
}

#check to see if enforce is default mode in the device
function call_check_default_enforce_mode {
echo -e "Executing script : call_check_default_enforce_mode"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_14_Default_Enforce_check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

for profile in "${profiles[@]}"
do
    echo "Profile: $profile"
done

# Loop through the profiles and validate their status before running the test
for profile in "${profiles[@]}"; do
  # To print the status
  # ps -auxZ | grep "$profile"
   ps -auxZ | grep "$profile" | awk 'NR==1{print $2}' >/dev/details
  
  # Check if the output for enforce"
  if ps -auxZ | grep "$profile" | grep -q "(enforce)"; then
    echo "SUCCESS: $profile is in enforce mode"
  else
    echo "FAILURE: $profile is not in enforce mode. Current mode: $(cat /dev/details)"
  fi

done
}

#function to check irmgr capabilities
function remove_dac_override_irmgr {
echo -e "Executing script : remove_dac_override_irmgr"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_15_Dac_Override_Remove_irMgrMain"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

for profile in "${profiles[@]}"; do
	if [ "$profile" == "irMgrMain" ]; then
		cat $path/$profile | grep -i 'DAC_OVERRIDE' >/dev/details
		if [ $(cat /dev/details|wc -l) -gt 0 ]; then
			echo "FAILURE: Override Roles are present in $profile"
			exit
		else
			echo "SUCCESS: Override roles are not present in $profile"
		fi
	fi
done
}

#function to remove dac override roles check
function remove_dac_override_default {
echo -e "Executing script : remove_dac_override_default"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_16_Dac_Override_Remove_default"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

for profile in "${profiles[@]}"; do
	if [ "$profile" == "default" ]; then
	cat $path/$profile | grep -i 'DAC_OVERRIDE' >/dev/details
		if [ $(cat /dev/details|wc -l) -gt 0 ]; then
			echo "FAILURE: Override Roles are present in $profile"
			exit
		else
			echo "SUCCESS: Override roles are not present in $profile"
		fi
	fi
done
}

#function to check audiocapturemgr capabilities
function dac_override_audiocapture {
echo -e "Executing script : dac_override_audiocapture"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_17_Dac_Override_Audiocapturemgr"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

for profile in "${profiles[@]}"; do
	if [ "$profile" == "audiocapturemgr" ]; then
		cat $path/$profile | grep -i 'DAC_OVERRIDE' >/dev/details
		if [ $(cat /dev/details|wc -l) -gt 0 ]; then
			echo "SUCCESS: Override Roles are present in $profile"
			exit
		else
			echo "FAILURE: Override roles are not present in $profile"
		fi
	fi
done
}

function dac_override_lighttpd {
echo -e "Executing script : dac_override_lighttpd"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_18_Dac_Override_lighttpd"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"

for profile in "${profiles[@]}"; do
	if [ "$profile" == "lighttpd" ]; then
		cat $path/$profile | grep -i 'DAC_OVERRIDE' >/dev/details
		if [ $(cat /dev/details|wc -l) -gt 0 ]; then
			echo "SUCCESS: Override Roles are present in $profile"
			exit
		else
			echo "FAILURE: Override roles are not present in $profile"
		fi
	fi
done
}

#check_logging : TODO
function check_logging {
echo "TBA"
}

function call_action_default {
enable_armor
service_status
profile_check
bin_process
blocklist
write_exe_check
capability_check
no_profiles_check
call_reboot_complain
call_reboot_enforce
call_reboot_disable
call_reboot_complain_enforce
call_reboot_denied_messages
call_check_default_enforce_mode
remove_dac_override_irmgr
remove_dac_override_default
dac_override_audiocapture
dac_override_lighttpd
}

if [ $# -lt 1 ]
then
        echo "Usage : "
	echo "$0 << [aptest01] [aptest02] ...>>"
	echo 
        param_help
	exit
fi

if [ $# -eq 0 ]; then
    call_action_default
fi
   
for test in "$@"; do
	echo "Testcase--->>> ${test}"
	case "${test}" in 
	 aptest01*)
	   echo "calling service_status"
	   service_status
   	   ;;
         aptest02*)
	   echo "calling enable_armor"
	   enable_armor
	  ;;
         aptest04*)
	   echo "calling bin_process"
	   bin_process
	   ;;
	 aptest03*)
	   echo "calling blocklist"
	   blocklist
	  ;;
          aptest05*)
	   echo "calling profile_check"
	   profile_check
	   ;;
	  aptest06*)
	   echo "calling write execute check"
	   write_exe_check	
	   ;;
	  aptest07*)
	   echo "calling capability check"
	   capability_check
	   ;;
	  aptest08*)
	   echo "calling no profiles check"
	   no_profiles_check
	   ;;
	   aptest09*)
	   echo "calling complain reboot check"
	   call_reboot_complain
	   ;;
	   aptest10*)
	   echo "calling enforce reboot check"
	   call_reboot_enforce
	   ;;
	   aptest11*)
	   echo "calling disable reboot check"
	   call_reboot_disable 
	   ;;
	   aptest12*)
	   echo "calling change from complain to enforce reboot check"
	   call_reboot_complain_enforce
	   ;;
	   aptest13*)
	   echo "calling denied messages after reboot check"
	   call_reboot_denied_messages
	   ;;
	   aptest14*)
	   echo "calling default enforce mode check"
	   call_check_default_enforce_mode
	   ;;
	   aptest15*)
	   echo "calling check irmgrmain dac override"
	   remove_dac_override_irmgr
	   ;;
	   aptest16*)
	   echo "calling check default dac override"
	   remove_dac_override_default
	   ;;
	   aptest17*)
	   echo "calling check audiocapture dac override"
	   dac_override_audiocapture
	   ;;
	   aptest18*)
	   echo "calling check lighttpd dac override"
	   dac_override_lighttpd
	   ;;
	   aptest19*)
	   echo "calling check log for denied messages"
	   check_logging
	   ;;
	  all*)
	   echo "Call all tests"
	   call_action_default
	   ;;
	  *)
	   echo "FAILURE:Error calling script"
	   param_help
	   exit 2
	  ;;
	esac
   done	

