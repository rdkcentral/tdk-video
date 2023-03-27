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
#  Author : Afeeja Shaik
#------------------------------------------------------------------------------
# Execution Starter
#------------------------------------------------------------------------------

#To remove white spaces before and after "=" from the config file
sed -i 's/ *=/=/;s/ = */=/' media_validation_variables.conf

#To remove white spaces before and after "+" from the config file
sed -i 's/ *+/+/;s/+ */+/' media_validation_variables.conf
source media_validation_variables.conf

#Creating logs and reports folders
LOG_DIRECTORY="logs"
REPORTS_DIRECTORY="reports"
echo "Creating logs and reports folders if not created before"
if [ ! -d "$LOG_DIRECTORY" ]; then
  $(mkdir "$LOG_DIRECTORY")
fi
if [ ! -d "$REPORTS_DIRECTORY" ]; then
  $(mkdir "$REPORTS_DIRECTORY")
fi

#Fetching Configurations
PLAYER_ARRAY=()
OPERATION_ARRAY=()
STREAM_ARRAY=()
INTERVALS=()
NO_OF_PLAYERS=0
INTERVAL=""

#Fetch the configuration Based on Execution Type
if [ "$EXECUTION_TYPE" = "runalltests" ]; then
  IFS=$','
  read -r -a PLAYER_ARRAY <<<"$ALL_PLAYERS"
  read -r -a STREAM_GROUP_ARRAY <<<"$ALL_STREAMS"
  for EACH_GROUP in "${STREAM_GROUP_ARRAY[@]}";do
    read -r -a STREAM_ARRAY_TEMP <<<"${!EACH_GROUP}"
    for GROUP_STREAM in "${STREAM_ARRAY_TEMP[@]}";do
      read -r -a SINGLE_GROUP_ARRAY <<<"$GROUP_STREAM"
      STREAM_ARRAY+=("${SINGLE_GROUP_ARRAY[@]}")
    done
  done
  read -r -a OPERATION_ARRAY <<<"$ALL_OPERATIONS"
  unset IFS
elif [ "$EXECUTION_TYPE" = "group" ]; then
  IFS=$','
  read -r -a PLAYER_ARRAY <<<"$PLAYER_NAME"
  read -r -a OPERATION_ARRAY <<<"$OPERATIONS"
  unset IFS
else
  IFS=$','
  read -r -a PLAYER_ARRAY <<<"$PLAYER_NAME"
  read -r -a OPERATION_ARRAY <<<"$OPERATIONS"
  unset IFS
fi

NO_OF_PLAYERS=${#PLAYER_ARRAY[@]}
NO_OF_OPERATIONS=${#OPERATION_ARRAY[@]}

#Fetch the Player URL based on Player Name
GET_PLAYER_URL() {
  ARG=$1
  RETURN_VALUE=""
  if [[ "$ARG" =~ "shaka" ]]; then
    RETURN_VALUE="$lightning_apps_loc$lightning_shaka_test_app_url"
  elif [[ "$ARG" =~ "aamp" ]]; then
    RETURN_VALUE="$lightning_apps_loc$lightning_uve_test_app_url"
  else
    RETURN_VALUE="$lightning_apps_loc$lightning_video_test_app_url"
  fi
  echo "$RETURN_VALUE"
}

#Fetch player URLs
PLAYER_URLS=()
for PLAYER in "${PLAYER_ARRAY[@]}"; do
    PLAYER_NAME_URL=$(GET_PLAYER_URL "$PLAYER")
    PLAYER_URLS+=("$PLAYER_NAME_URL")
done

#Formatting Stream URL
FORMAT_STREAM_URL(){
  ARGUMENT=$1
  STREAM_REF=${!ARGUMENT}
  if [[ -n "$STREAM_REF" ]] && [[ "$STREAM_REF" != " " ]]; then
    STREAM_URL_FINAL="$test_streams_base_path""${STREAM_REF}"
  else
    STREAM_URL_FINAL=""
  fi
  echo "$STREAM_URL_FINAL"
}

#Fetch Interval based on the operation
FETCH_INTERVAL(){
  ARGUMENT=$1
  RESULT_INTERVAL=""
  if [ "$ARGUMENT" == "playpause" ]; then
    RESULT_INTERVAL="$pause_interval $play_interval"
  elif [ "$ARGUMENT" == "play" ]; then
    RESULT_INTERVAL=$close_interval
  elif [ "$ARGUMENT" == "muteunmute" ]; then
    RESULT_INTERVAL=$mute_unmute_interval
  elif [[ "$ARGUMENT" =~ "pause" ]]; then
    RESULT_INTERVAL=$pause_interval
  elif [ "$ARGUMENT" == "close" ]; then
    RESULT_INTERVAL=$close_interval
  elif [ "$ARGUMENT" == "checkloop" ]; then
    RESULT_INTERVAL=$checkloop_interval
  elif [ "$ARGUMENT" == "playtillend" ]; then
    RESULT_INTERVAL=$play_till_end_interval
  elif [ "$ARGUMENT" == "seekff" ]; then
    RESULT_INTERVAL="$seekfwd_interval $fastfwd_interval $play_now_interval"
  elif [ "$ARGUMENT" == "fastfwd" ]; then
    RESULT_INTERVAL="$seekfwd_interval $play_now_interval $close_interval"
  elif [ "$ARGUMENT" == "fastfwd2x"  ] || [ "$ARGUMENT" == "fastfwd3x" ] || [ "$ARGUMENT" == "fastfwd4x" ]; then
    RESULT_INTERVAL="$seekfwd_interval $fastfwd_interval $play_now_interval"
  elif [ "$ARGUMENT" == "rewind" ]; then
    RESULT_INTERVAL="$rewind_interval $close_interval"
  elif [ "$ARGUMENT" == "seekfwd" ]; then
    RESULT_INTERVAL="$fastfwd_interval $close_interval"
  elif [ "$ARGUMENT" == "seekbwd" ]; then
    RESULT_INTERVAL="$seekbwd_interval $close_interval"
  elif [ "$ARGUMENT" == "seekpos" ]; then
    RESULT_INTERVAL="$seekfwd_position $seekbwd_position"
  elif [ "$ARGUMENT" == "trickplay" ]; then
    RESULT_INTERVAL="$operation_max_interval"
  elif [ "$ARGUMENT" == "changetext" ]; then
    RESULT_INTERVAL="$change_text_interval $close_interval"
  fi
  echo "$RESULT_INTERVAL"
}

#fetching Intervals based on operation
if [ "$EXECUTION_TYPE" = "runalltests" ]; then
  for SINGLE_OPERATION in "${OPERATION_ARRAY[@]}";do
    INTERVAL_OPERATION=$(FETCH_INTERVAL "$SINGLE_OPERATION")
    INTERVALS+=("$INTERVAL_OPERATION")
   done
fi

#Additional arguments for parameters like check interval
GET_ADDITIONAL_REQUIREMENTS(){
  ADDITIONAL_REQUIREMENTS=""
  OPERATION_ARG=$1
  if [ "$OPERATION_ARG" == "seekff" ]; then
    ADDITIONAL_REQUIREMENTS=$seekfwd_check_interval
  elif [[ "$OPERATION_ARG" =~ "fastfwd" ]]; then
    ADDITIONAL_REQUIREMENTS="$seekfwd_check_interval"
  elif [ "$OPERATION_ARG" == "rewind" ]; then
    ADDITIONAL_REQUIREMENTS="$rewind_check_interval"
  elif [ "$OPERATION_ARG" == "seekfwd" ] || [ "$OPERATION_ARG" == "seekbwd" ]; then
    ADDITIONAL_REQUIREMENTS="$seekfwd_check_interval,$seek_interval"
  elif [ "$OPERATION_ARG" == "seekpos" ]; then
    ADDITIONAL_REQUIREMENTS="$seekpos_check_interval"
  elif [ "$OPERATION_ARG" == "trickplay" ]; then
    ADDITIONAL_REQUIREMENTS="$seekfwd_check_interval"
  elif [ "$OPERATION_ARG" == "changetext" ]; then
    ADDITIONAL_REQUIREMENTS="$changetext_check_interval"
  fi
  echo "$ADDITIONAL_REQUIREMENTS"
}

#Format URL, SSH into the Device and Execute the Script
STREAM_URL=""
STREAM_URLS=()
DATE_STAMP=$(date -u +'%Y_%m_%d_%I_%M_%S')

CSV_NAME="reports/"$EXECUTION_TYPE"_execution_report_"$DATE_STAMP".csv"
echo "S.NO","Stream","Operation","Player","Result" >> "$CSV_NAME"
CSV_SERIAL_NUMBER=0

#Function to generate CSV
generateCSV(){
  CSV_SERIAL_NUMBER=$((CSV_SERIAL_NUMBER+1))
  IFS=$' '
  FINAL_RESULT="$4"
  if [ -z "$FINAL_RESULT" ]; then
    echo "$CSV_SERIAL_NUMBER","$1","$2","$3","FAILURE" >> "$CSV_NAME"
  else
    read -r -a FINAL_RESULT_ARRAY <<< "$FINAL_RESULT"
    unset IFS
    RESULT="${FINAL_RESULT_ARRAY[${#FINAL_RESULT_ARRAY[@]}-1]}"
    if [ "$RESULT" != "SUCCESS" ]; then
      echo "$CSV_SERIAL_NUMBER","$1","$2","$3","FAILURE" >> "$CSV_NAME"
    else
      echo "$CSV_SERIAL_NUMBER","$1","$2","$3","$RESULT" >> "$CSV_NAME"
    fi
  fi
}

#SSH into the device and execute the script
#This function is responsible for CSV and log generation
SSH_EXECUTE(){
  PLAYER_NAME=$1
  OPERATION=$2
  STREAM_EXECUTION=$3
  PLAYER_URL=$4
  FORMATTED_STREAM=$5
  INTERVAL_OPERATION=$6
  ADDITIONAL_REQUIREMENTS=$7
  LOG_NAME="logs/log_"$EXECUTION_TYPE"_"${PLAYER_NAME}"_"$OPERATION"_"${STREAM}"_"$DATE_STAMP".log"
  ssh "$user_name"@"$ip_address" 'bash -s' -- < ./MVS_Single_Execution_Script.sh "'$PLAYER_URL'" "$OPERATION" "$FORMATTED_STREAM" "'$INTERVAL_OPERATION'" "$PLAYER_NAME" "$ADDITIONAL_REQUIREMENTS" >> "$LOG_NAME"
  echo "$(<"$LOG_NAME")"
  FINAL_RESULT=$(tail -n 1 "$LOG_NAME")
  generateCSV "$STREAM_EXECUTION" "$OPERATION" "$PLAYER_NAME" "$FINAL_RESULT"
}

LOG_NAME=""
#Categorize the execution based on Execution type
if [ "$EXECUTION_TYPE" = "runalltests" ]; then
  for (( k = 0; k < NO_OF_PLAYERS; k++ )); do
    PLAYER_NAME=${PLAYER_ARRAY[k]}
    PLAYER_URL=${PLAYER_URLS[k]}
    for (( i = 0; i < NO_OF_OPERATIONS; i++ )); do
      OPERATION=${OPERATION_ARRAY[i]}
      INTERVAL_OPERATION=${INTERVALS[$i]}
      ADDITIONAL_REQUIREMENTS=$(GET_ADDITIONAL_REQUIREMENTS "$OPERATION")
      for STREAM in "${STREAM_ARRAY[@]}";do
        FORMATTED_STREAM=$(FORMAT_STREAM_URL "$STREAM")
        if [ -z "$FORMATTED_STREAM" ]; then
          echo "Error: Stream is not configured for:$STREAM"
          continue
        fi
        SSH_EXECUTE "$PLAYER_NAME" "$OPERATION" "$STREAM" "$PLAYER_URL" "$FORMATTED_STREAM" "$INTERVAL_OPERATION" "$ADDITIONAL_REQUIREMENTS"
      done
    done
  done

elif [ "$EXECUTION_TYPE" = "group" ]; then
  for (( k = 0; k < NO_OF_PLAYERS; k++ )); do
    PLAYER_NAME=${PLAYER_ARRAY[k]}
    PLAYER_URL=${PLAYER_URLS[k]}
    STREAM_URLS=()
    for (( i = 0; i < NO_OF_OPERATIONS; i++ )); do
      OPERATION=${OPERATION_ARRAY[i]}
      ADDITIONAL_REQUIREMENTS=$(GET_ADDITIONAL_REQUIREMENTS "$OPERATION")
      INTERVAL_OPERATION=$(FETCH_INTERVAL "$OPERATION")
      GROUP_STREAM_NAMES=$"STREAM$((i+1))"
      IFS=$','
      read -r -a GROUP_STREAMS <<<"${!GROUP_STREAM_NAMES}"
      for STREAM_NAME in "${GROUP_STREAMS[@]}";do
        read -r -a SINGLE_STREAM <<<"${!STREAM_NAME}"
        for STREAM in "${SINGLE_STREAM[@]}";do
          FORMATTED_STREAM=$(FORMAT_STREAM_URL "$STREAM")
          if [ -z "$FORMATTED_STREAM" ]; then
            echo "Error: Stream is not configured for: $STREAM"
            continue
          fi
          SSH_EXECUTE "$PLAYER_NAME" "$OPERATION" "$STREAM" "$PLAYER_URL" "$FORMATTED_STREAM" "$INTERVAL_OPERATION" "$ADDITIONAL_REQUIREMENTS"
        done
      done
      unset IFS
    done
  done
else
  for (( k = 0; k < NO_OF_PLAYERS; k++ )); do
    PLAYER_NAME=${PLAYER_ARRAY[k]}
    PLAYER_URL=${PLAYER_URLS[k]}
    for (( i = 0; i < NO_OF_OPERATIONS; i++ )); do
      OPERATION=${OPERATION_ARRAY[i]}
      ADDITIONAL_REQUIREMENTS=$(GET_ADDITIONAL_REQUIREMENTS "$OPERATION")
      INTERVAL_OPERATION=$(FETCH_INTERVAL "$OPERATION")
      STREAM_PARAM_NAME="STREAM$((i+1))"
      STREAM_VALUE="${!STREAM_PARAM_NAME}"
      IFS=$','
      read -r -a STREAMS_LIST <<<"$STREAM_VALUE"
      unset IFS
      STREAM_URLS=()
      for STREAM_NAME in "${STREAMS_LIST[@]}";do
        FORMATTED_STREAM=$(FORMAT_STREAM_URL "$STREAM_NAME")
        if [ -z "$FORMATTED_STREAM" ]; then
          echo "Error: Stream is not configured for: $STREAM_NAME"
          continue
        fi
        SSH_EXECUTE "$PLAYER_NAME" "$OPERATION" "$STREAM_NAME" "$PLAYER_URL" "$FORMATTED_STREAM" "$INTERVAL_OPERATION" "$ADDITIONAL_REQUIREMENTS"
      done
    done
  done
fi

