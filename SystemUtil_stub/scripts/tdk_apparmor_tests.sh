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
echo 
echo "Note: if 'all' is passed as test number, all tests get executed"
echo "e.g.: ./tdk_apparmor_tests.sh all"
}

#file declaration

apparmor_file=/opt/apparmor_profile.config


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
                echo "Profiles: $profile1, $profile2"
	        if [[ -f "$profile1" ]] && [[ -f "$profile2" ]]
                then
                        echo -e "SUCCESS:$profile1, $profile2 exists"
                else "FAILURE:Profiles does not exists"
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
	source /opt/apparmor_profile.config
        echo "Apparmor is enabled"
	if [[ -f "$profile1" ]] && [[ -f "$profile2" ]] && [[ -f "$profile3" ]]
                then
                echo "SUCCESS: $profile1, $profile2, $profile3 exists"
                else 
		echo "FAILURE: One of Profiles does not exist"
         fi

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

source /opt/apparmor_profile.config
apparmor_file=/opt/apparmor_profile.config
        cat $profile1 | grep -i 'w.*x' >/dev/details
        if [ $(cat /dev/details|wc -l) -gt 0 ]; then
                echo "FAILURE:$profile contains write and execute permission"
		exit
                else
                echo "SUCCESS:$profile has no write and execute permission together"
        fi
	cat $profile2 | grep -i 'w.*x' >/dev/details
        if [ $(cat /dev/details|wc -l) -gt 0 ]; then
                echo "FAILURE:$profile contains write and execute permission"
                exit
		else
                echo "SUCCESS:$profile has no write and execute permission together"
        fi
	cat $profile3 | grep -i 'w.*x' >/dev/details
        if [ $(cat /dev/details|wc -l) -gt 0 ]; then
                echo "FAILURE:$profile contains write and execute permission"
                exit
		else
                echo "SUCCESS:$profile has no write and execute permission together"
        fi
}

#function to check capabilities
function capability_check {
echo -e "Executing script : apparmor_capability_check"
echo -e "======================================="
echo -e "Test Execution Name is: TDK_Apparmor_Test_07_Capability_Check"
echo -e "Connected to $ip Box for validating profiles"
echo -e "Connected to Server!\n"
source /opt/apparmor_profile.config

	res=$(cat $profile1 | grep -i 'CAP_*')
	res2=$(cat $profile2 | grep -i 'CAP_*')
	res3=$(cat $profile3 | grep -i 'CAP_*')
	if [ $res ]; then
                echo "FAILURE: Capability Roles are present in $profile1"
                exit
        elif [ $res2 ]; then
                echo "FAILURE: Capability Roles are present in $profile2"
                exit
        elif [ $res3 ]; then
                echo "FAILURE: Capabililty Roles are present in $profile3"
                exit
        else
	echo "SUCCESS: Capability Roles are not present in all profiles"
	fi
}

function call_action_default {
enable_armor
service_status
profile_check
bin_process
blocklist
write_exe_check
capability_check
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
