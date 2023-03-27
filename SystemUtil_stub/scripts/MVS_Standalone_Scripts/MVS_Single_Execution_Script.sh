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
# Steps for Single Execution
#------------------------------------------------------------------------------

PLAYER_URL=$1
OPERATION=$2
STREAM_URL=$3
INTERVAL=$4
PLAYER=$5
LOG_FILE="/opt/logs/wpeframework.log"
SKIP_PROGRESS_LIST=()
STREAM_DURATION_NUMBER=0
FINAL_RESULT="SUCCESS"

#Below are the operations supported by lightning app players as per the wiki https://etwiki.sys.comcast.net/display/RDKV/TDKVA
SHAKA_SUPPORTED=("pause" "play" "playnow" "switch" "playtillend" "close" "seekfwd" "seekbwd" "seekpos" "fastfwd" "fastfwd2x" "fastfwd3x" "fastfwd4x" "fastfwd16x" "fastfwd32x" "fastfwd64x" "rewind" "rewind2x" "rewind3x" "rewind4x" "rewind16x" "rewind32x" "rewind64x" "Mute" "Unmute" "checkloop" "init" "changeaudio" "changetext" "Integer" "autoplay" "loop" "seekInterval" "headers" "enablecc")
SDK_SUPPORTED=("pause" "play" "playnow" "switch" "playtillend" "close" "seekfwd" "seekbwd" "seekpos" "fastfwd" "fastfwd2x" "fastfwd3x" "fastfwd4x" "fastfwd16x" "fastfwd32x" "fastfwd64x" "rewind" "rewind2x" "rewind3x" "rewind4x" "rewind16x" "rewind32x" "rewind64x" "Mute" "Unmute" "checkloop" "init" "Integer" "autoplay" "loop" "seekInterval")
AAMP_SUPPORTED=("pause" "play" "playnow" "switch" "playtillend" "close" "seekfwd" "seekbwd" "seekpos" "fastfwd" "fastfwd2x" "fastfwd3x" "fastfwd4x" "fastfwd16x" "fastfwd32x" "fastfwd64x" "rewind" "rewind2x" "rewind3x" "rewind4x" "rewind16x" "rewind32x" "rewind64x" "Mute" "Unmute" "checkloop" "init" "changeaudio" "changetrack" "Integer" "autoplay" "loop" "seekInterval" "headers" "enablecc")
CURRENT_OPERATIONS=()

#Steps for Stream Validation
MONITOR_STREAM_OPERATION(){
  LOCAL_OPERATION=$1
  EVENT_MESSAGE=$2
  LOCAL_DATE_TIME=$3
  END_EXECUTION_STRING="Execution Ended!!!"
  PROGRESSING_STRING="Video Progressing"
  LOG_PATTERN='\[open\]\|Observed Event\:\|Video Progressing\|Playback Rate to\|TEST STATUS\|TEST RESULT\|Execution Ended\|\[TESTCASE RESULT\]'

  #Validation 1: Beginning of the operation
  echo "Step 5: Validating beginning of the Stream Operation"
  BEGIN_OPERATION_STRING="Automatic Operation Execution Begins"
  SECONDS=0
  until awk -v pattern="$LOCAL_DATE_TIME" '$0~pattern,0' $LOG_FILE | grep -i -q "$BEGIN_OPERATION_STRING"
  do
    if [[ SECONDS -gt 30 ]]; then
      PRINT_END_STATEMENT "Error: Operation execution did not start as per logs"
    fi
    sleep 3
  done
  OPERATION_START=$(awk -v pattern="$LOCAL_DATE_TIME" '$0~pattern,0' $LOG_FILE | grep -i "$BEGIN_OPERATION_STRING")
  IFS=$' '
  read -r -a OPERATION_DATE_TIME_ARRAY <<< "$OPERATION_START"
  OPERATION_DATE_TIME="${OPERATION_DATE_TIME_ARRAY[0]} ${OPERATION_DATE_TIME_ARRAY[1]} ${OPERATION_DATE_TIME_ARRAY[2]}"
  unset IFS

  echo "Waiting for the Video Play to Complete"
  sleep $(( INTERVAL + 40 ))

  #Validation2: To check whether the stream Play is completed
  echo "Step 6: Validating End of the execution"
  COMPLETE_LOG=()
  if [[ "$LOCAL_OPERATION" == "playtillend" ]]; then
    echo "Please wait while \"Play Till End\" Operation completes"
    SECONDS=0
    IFS=$'\n'
    COMPLETE_LOG=($(<$LOG_FILE))
    until grep -i -q "$END_EXECUTION_STRING" $LOG_FILE
    do
      IFS=$'\n'
      COMPLETE_LOG=($(<$LOG_FILE))
      if [[ SECONDS -gt $((STREAM_DURATION_NUMBER+20)) ]]; then
        PRINT_END_STATEMENT "Stream \"Play End\" event did not occur"
      fi
      sleep 20
    done
    echo "Stream Play is Completed."
  else
    SECONDS=0
    until awk -v pattern="$OPERATION_DATE_TIME" '$0~pattern,0' $LOG_FILE | grep -i -q "$END_EXECUTION_STRING"
      do
      if [[ SECONDS -gt $((STREAM_DURATION_NUMBER+10)) ]]; then
        PRINT_END_STATEMENT "Stream \"Play End\" event did not occur"
      fi
      echo "Stream play is not completed yet. Waiting for few more moments"
      sleep 10
      done
    echo "Stream Play is Completed."
    IFS=$'\n'
    COMPLETE_LOG=($(awk -v pattern="$OPERATION_DATE_TIME" '$0~pattern,0' $LOG_FILE))
    unset IFS
  fi

  #Validation 3: Event Validation
  echo "Step 7: Validating Observed Event Details"
  if [[ "$EVENT_MESSAGE" =~ "," ]]; then
    IFS=$','
    read -r -a EVENT_MESSAGE_ARRAY <<< "$EVENT_MESSAGE"
    unset IFS
    IFS=$'\n'
    LOG_EVENTS=($(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep "Observed Event" | grep -v -e "TimeUpdate" -e "AdStart" -e "AdEnd" -e "LoadedData" -e "CanPlay" -e "CanPlayThrough"))
    unset IFS
    RESULT_EVENT=0
    for SUB_EVENT in "${EVENT_MESSAGE_ARRAY[@]}";do
      SECONDS=0
      PLAYING_EVENT_STRING="Observed Event: $SUB_EVENT"
      EVENT_ENTRY_RESULT=$(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep -i -c "$PLAYING_EVENT_STRING")
      if [[ EVENT_ENTRY_RESULT -eq 0 ]]; then
        echo "Error: Failed to receive the expected event: $SUB_EVENT"
      fi
    done
  else
    PLAYING_EVENT_STRING="Observed Event: $EVENT_MESSAGE"
    SECONDS=0
    EVENT_ENTRY_RESULT=$(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep -i -c "$PLAYING_EVENT_STRING")
    if [[ EVENT_ENTRY_RESULT -eq 0 ]]; then
      echo "Error: Failed to receive the expected event: $EVENT_MESSAGE"
    else
      echo "---------------------------------------------------------------------------------------------------------------------------------------"
      echo "All the events occurred as expected"
      echo "---------------------------------------------------------------------------------------------------------------------------------------"
    fi
  fi

  #Validation4: Video Progress Status
  PROGRESSING_STRING="Video Progressing"
  PROGRESS_VALIDATE=$(echo "${SKIP_PROGRESS_LIST[@]}" | grep -ow "$LOCAL_OPERATION" | wc -w)
  if [[ PROGRESS_VALIDATE -eq 0 ]]; then
    echo "Step 8: Validating Stream Progress Duration for Hang detection"
    sleep 15
    IFS=$'\n'
    PROGRESS_ENTRIES=($( printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep "$PROGRESSING_STRING" ))
    unset IFS
    LENGTH=0
    LENGTH=${#PROGRESS_ENTRIES[@]}
    TEST_STATUS=${COMPLETE_LOG[SUCCESS_STATUS]}
    PREVIOUS_DURATION=0
    CURRENT_DURATION=0
    echo "                                                        "
    if [[ LENGTH -eq 0 ]]; then
      echo "Video Progressing Status is not found"
      PRINT_END_STATEMENT "Error:Stream Validation is FAILED."
    fi
    for (( i = 0; i < LENGTH; i++ ))
    do
      LINE_DURATION=${PROGRESS_ENTRIES[$i]}
      IFS=$' '
      read -r -a LINE_ARRAY <<< "$LINE_DURATION"
      DURATION=${LINE_ARRAY[${#LINE_ARRAY[@]} - 1]}
      if [[ $i == 0 ]]; then
        PREVIOUS_DURATION=$DURATION
        continue
      else
        CURRENT_DURATION=$DURATION
        if [ 1 -eq "$(awk 'BEGIN{print '"$CURRENT_DURATION"' < '"$PREVIOUS_DURATION"'?1:0}')" ]; then
          PRINT_END_STATEMENT -e "Error:Stream Validation is FAILED and Hang is detected based on Progress of the Video."
        fi
        PREVIOUS_DURATION=${CURRENT_DURATION}
      fi
    done
  fi

  #Printing logs
  IFS=$'\n'
  echo "                                                      "
  echo "****************************************************************Execution Summary******************************************************"
  echo "                                                      "
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  echo "STREAM DETAILS"
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  STREAM_DETAILS_LOG=($(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep "_loader" ))
  echo "${STREAM_DETAILS_LOG[*]}"
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  echo "                                                                     "
  echo "Validating logs for Hang detection"
  PRINT_LOGS=($(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep "$LOG_PATTERN" | grep -v -e "TimeUpdate"))
  echo "${PRINT_LOGS[*]}"
  unset IFS

  #Validating Final Result
  echo "Final Step: Validating End Result of the Execution"
  TEST_STATUS="TEST RESULT"
  TEST_CASE_RESULT=$(printf -- '%s\n' "${COMPLETE_LOG[@]}" | grep "$TEST_STATUS")
  echo "                                                      "
  if [[ "$TEST_CASE_RESULT" =~ "TEST RESULT: SUCCESS" ]] && [ "$FINAL_RESULT" != "FAILURE" ]; then
    echo "-------------------------------------------------------------------------------------------------------------------------------------"
    echo "Stream Validation is Successful for the Operation: \"$OPERATION\" on Player: \"$PLAYER\""
    echo "-------------------------------------------------------------------------------------------------------------------------------------"
  else
    FINAL_RESULT="FAILURE"
    echo "$TEST_CASE_RESULT"
    FAILURE_REASON_STRING="Error Occured While Executing the Testcases :"
    FAILURE_REASON=$(awk -v pattern="$OPERATION_DATE_TIME" '$0~pattern,0' $LOG_FILE| grep -i "$FAILURE_REASON_STRING")
    echo "-------------------------------------------------------------------------------------------------------------------------------------"
    echo -e "Stream Validation is Failed due to: \n $FAILURE_REASON"
    echo "-------------------------------------------------------------------------------------------------------------------------------------"
  fi
}

#Printing Log
DATE_DEVICE=$(date)
echo "****************************************************************Executing MVS Standalone Script******************************************************"
echo "Execution Date Time: $DATE_DEVICE"
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "Configuration Details"
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "Player URL: $PLAYER_URL"
echo "STREAM URL: $STREAM_URL"
echo "Operation: $OPERATION"
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "Check Pre conditions..."

#To print log in case of error
PRINT_END_STATEMENT(){
  FINAL_RESULT="FAILURE"
  echo "$1"
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  echo "*********************************************************** End of Execution *************************************************************"
  echo "Validation of the script is: $FINAL_RESULT"
  exit 1
}

#To check whether operation is supported in the given player or not
CHECK_IF_OPERATION_SUPPORTED(){
  ELEMENT=("$1")
  IS_OPERATION_SUPPORTED=0
    if [ "$PLAYER" == "shaka" ]; then
      IS_OPERATION_SUPPORTED=$(echo "${SHAKA_SUPPORTED[@]}" | grep -ow "$ELEMENT" | wc -w)
    elif [ "$PLAYER" == "sdk" ]; then
      IS_OPERATION_SUPPORTED=$(echo "${SDK_SUPPORTED[@]}" | grep -ow "$ELEMENT" | wc -w)
    elif [ "$PLAYER" == "aamp" ]; then
      IS_OPERATION_SUPPORTED=$(echo "${AAMP_SUPPORTED[@]}" | grep -ow "$ELEMENT" | wc -w)
    fi
  echo $((IS_OPERATION_SUPPORTED))
}

#Formatting the actual URL
IS_OPERATION_SUPPORTED=0
if [[ "$OPERATION" =~ "muteunmute" ]]; then
  CURRENT_OPERATIONS=("mute" "Unmute")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  URL="$PLAYER_URL&operations=mute($INTERVAL),Unmute($INTERVAL)&logging=RESTAPI&url=$STREAM_URL&autotest=true"
elif [[ "$OPERATION" =~ "seekff"  ]]; then
  CURRENT_OPERATIONS=("seekfwd" "fastfwd" "playnow")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  IFS=$' '
  read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
  unset IFS
  CHECK_INTERVAL=$6
  URL="$PLAYER_URL&logging=RESTAPI&operations=seekfwd(${INTERVAL_ARRAY[0]}),fastfwd(${INTERVAL_ARRAY[1]}),playnow(${INTERVAL_ARRAY[2]})&autotest=true&options=checkInterval($CHECK_INTERVAL),loop&url=$STREAM_URL"
  INTERVAL=$((INTERVAL_ARRAY[0]))
elif [[ "$OPERATION" == "play" || "$OPERATION" == "close" ]]; then
  CURRENT_OPERATIONS=("close")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  URL="$PLAYER_URL&operations=close($INTERVAL)&logging=RESTAPI&url=$STREAM_URL&autotest=true"
elif [[ "$OPERATION" =~ "fastfwd" ]]; then
  CHECK_INTERVAL=$6
  IFS=$' '
  read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
  INTERVAL=$((INTERVAL_ARRAY[0]))
  unset IFS
  if [ "$OPERATION" == "fastfwd" ]; then
    CURRENT_OPERATIONS=("seekfwd" "fastfwd" "playnow")
    for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
      IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
      if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
         PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
      fi
    done
    URL="$PLAYER_URL&operations=seekfwd(${INTERVAL_ARRAY[0]}),fastfwd(${INTERVAL_ARRAY[1]}),playnow(${INTERVAL_ARRAY[2]})&options=checkInterval($CHECK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
  else
    CURRENT_OPERATIONS=("$OPERATION" "playnow" "close")
    for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
      IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
      if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
         PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
      fi
    done
    URL="$PLAYER_URL&operations=$OPERATION(${INTERVAL_ARRAY[0]}),playnow(${INTERVAL_ARRAY[1]}),close(${INTERVAL_ARRAY[2]})&options=checkInterval($CHECK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
  fi
elif [ "$OPERATION" == "rewind" ]; then
    CURRENT_OPERATIONS=("$OPERATION" "close")
    for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
      IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
      if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
         PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
      fi
    done
   CHECK_INTERVAL=$6
   IFS=$' '
   read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
   INTERVAL=$((INTERVAL_ARRAY[0]))
   unset IFS
   URL="$PLAYER_URL&operations=$OPERATION(${INTERVAL_ARRAY[0]}),close(${INTERVAL_ARRAY[1]})&options=checkInterval($CHECK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
elif [ "$OPERATION" == "seekfwd" ] || [ "$OPERATION" == "seekbwd" ]; then
  CURRENT_OPERATIONS=("$OPERATION" "close" "seekInterval")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  ADDITIONAL_ARGS=$6
  IFS=$','
  read -r -a ADDITIONAL_ARRAY <<<"$ADDITIONAL_ARGS"
  SEEK_INTERVAL=${ADDITIONAL_ARRAY[0]}
  CHECK_INTERVAL=${ADDITIONAL_ARRAY[1]}
  unset IFS
  IFS=$' '
  read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
  INTERVAL=${INTERVAL_ARRAY[0]}
  unset IFS
  URL="$PLAYER_URL&operations=$OPERATION(${INTERVAL_ARRAY[0]}),close(${INTERVAL_ARRAY[1]})&options=checkInterval($CHECK_INTERVAL),seekInterval($SEEK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
elif [ "$OPERATION" == "seekpos" ]; then
  CURRENT_OPERATIONS=("seekpos" "close")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  IFS=$' '
  read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
  INTERVAL=$((INTERVAL_ARRAY[0]))
  unset IFS
  CHECK_INTERVAL=$6
  URL="$PLAYER_URL&operations=seekpos(30:${INTERVAL_ARRAY[0]}),seekpos(30:${INTERVAL_ARRAY[1]}),close(30)&options=checkInterval($CHECK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
elif [ "$OPERATION" == "trickplay" ]; then
  CURRENT_OPERATIONS=("seekfwd" "fastfwd" "playnow" "pause" "play" "seekbwd" "loop")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  CHECK_INTERVAL=$6
  URL="$PLAYER_URL&operations=seekfwd($INTERVAL),fastfwd($INTERVAL),fastfwd($INTERVAL),playnow($INTERVAL),pause($INTERVAL),play($INTERVAL),seekbwd($INTERVAL),fastfwd($INTERVAL)&options=checkInterval($CHECK_INTERVAL),loop&autotest=true&logging=RESTAPI&url=$STREAM_URL"
elif [ "$OPERATION" == "changetext" ]; then
  CURRENT_OPERATIONS=("changetext" "close")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  IFS=$' '
  read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
  INTERVAL=$((INTERVAL_ARRAY[0]))
  unset IFS
  CHECK_INTERVAL=$6
  URL="$PLAYER_URL&operations=changetext(${INTERVAL_ARRAY[0]}),close(${INTERVAL_ARRAY[1]}),options=checkInterval($CHECK_INTERVAL)&autotest=true&logging=RESTAPI&url=$STREAM_URL"
elif [ "$OPERATION" == "playpause" ]; then
  CURRENT_OPERATIONS=("play" "pause")
    for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
      IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
      if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
         PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
      fi
    done
    IFS=$' '
    read -r -a INTERVAL_ARRAY <<<"$INTERVAL"
    echo "before: ${INTERVAL_ARRAY[*]}"
    INTERVAL=$((INTERVAL_ARRAY[0]))
    unset IFS
    URL="$PLAYER_URL&operations=pause(${INTERVAL_ARRAY[0]}),play(${INTERVAL_ARRAY[1]})&logging=RESTAPI&url=$STREAM_URL&autotest=true"
else
  CURRENT_OPERATIONS=("$OPERATION")
  for ELEMENT in "${CURRENT_OPERATIONS[@]}";do
    IS_OPERATION_SUPPORTED=$(CHECK_IF_OPERATION_SUPPORTED "${CURRENT_OPERATIONS[@]}")
    if [[ IS_OPERATION_SUPPORTED -eq 0 ]]; then
       PRINT_END_STATEMENT "Operation:$ELEMENT is not supported in $PLAYER Media Player"
    fi
  done
  URL="$PLAYER_URL&operations=$OPERATION($INTERVAL)&logging=RESTAPI&url=$STREAM_URL&autotest=true"
fi

REGEX_SUCCESS_PATTERN='\"success\":true'

#Launch Webkit Browser
echo "Step 1: Launching Webkit Browser"
echo "Launching Webkit browser with $URL"
WEBKIT_LAUNCH_COMMAND="(curl -H 'Content-Type: application/json' \
-X POST \
--data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser", "uri":"about:blank"}}' \
'http://127.0.0.1:9998/jsonrpc')"
WEBKIT_LAUNCH=$(curl -H 'Content-Type: application/json' \
-X POST \
--data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser", "uri":"about:blank"}}' \
'http://127.0.0.1:9998/jsonrpc')
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "                                                        "
echo "Json Command :$WEBKIT_LAUNCH_COMMAND"
echo "Response : $WEBKIT_LAUNCH"
echo "                                                        "
echo "---------------------------------------------------------------------------------------------------------------------------------------"
JSON_RESPONSE=$(echo "$WEBKIT_LAUNCH" |  grep $REGEX_SUCCESS_PATTERN)

if [[ -z "$JSON_RESPONSE" ]]; then
  PRINT_END_STATEMENT "Error: Problem in launching webkit browser: $WEBKIT_LAUNCH"
else
  echo -e "Webkit launch is successful"
fi

#Curl command to load the URL
echo "Step 2: Loading the URL"
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "                                                       "
WEBKIT_LOAD_COMMAND="(curl -H 'Content-Type: application/json' \
-X POST \
--data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser", "uri":'"${URL}"'}}' \
'http://127.0.0.1:9998/jsonrpc')"
WEBKIT_LOAD=$(curl -H 'Content-Type: application/json' \
-X POST \
--data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser","uri":'\"$URL\"'}}' \
'http://127.0.0.1:9998/jsonrpc')

echo "Json Command :$WEBKIT_LOAD_COMMAND"
echo "Response : $WEBKIT_LOAD"
echo "                                                        "
echo "---------------------------------------------------------------------------------------------------------------------------------------"
JSON_RESPONSE1=$(echo "$WEBKIT_LOAD" |  grep $REGEX_SUCCESS_PATTERN)
if [[ -z "$JSON_RESPONSE1" ]]; then
  PRINT_END_STATEMENT "Error: Problem in loading Player with given URL $URL: Response: $WEBKIT_LOAD"
else
  echo "Webkit Browser is launched with given player URL."
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
fi
#Fetch the date to grep the first line
DATE_TIME=$(date +'%b %d %H:%M:%S')
echo "Waiting for the stream URL to load"
sleep $((INTERVAL+10))

#Verifying whether Player loaded with the given URL
echo "Step 3: Verifying whether the Player is loaded with given URL"
URL_SEARCH_STRING=""
if [ "$PLAYER" = "sdk" ]; then
  URL_SEARCH_STRING="[open] Lightning SDK Media Player. Play stream: $STREAM_URL."
else
  URL_SEARCH_STRING="[open] $PLAYER Media Player. Play stream: $STREAM_URL."
fi
STARTING_POINT_FORMATTED=${URL_SEARCH_STRING//[/\\[}
URL_SEARCH_STRING=${STARTING_POINT_FORMATTED//]/\\]}
STARTING_POINT=$(awk -v pattern="$DATE_TIME" '$0~pattern,0' $LOG_FILE| grep -i "$URL_SEARCH_STRING" | tail -1)
echo "---------------------------------------------------------------------------------------------------------------------------------------"
if [ -n "$STARTING_POINT" ] ; then
  echo -e "Player is opened with given URL. ${STREAM_URL}"
else
  PRINT_END_STATEMENT "Error: Player is not loaded with given URL: $STREAM_URL"
fi
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "All the validations in this script are done by reading console logs"``
echo "Pre conditions for the test are set successfully"
echo "---------------------------------------------------------------------------------------------------------------------------------------"
echo "                                                        "
sleep 10

#Fetch Stream Duration
echo "Step 4: Fetching the Stream Details from the Log"
STREAM_DURATION_ENTRY=$(awk -v pattern="$DATE_TIME" '$0~pattern,0' $LOG_FILE | grep -i -m 1 "Stream Duration")
unset IFS
if [[ -z "$STREAM_DURATION_ENTRY" ]]; then
  PRINT_END_STATEMENT "Error: Stream Duration is not found"
fi
IFS=$' '
read -r -a STREAM_DURATION_ARRAY <<< "$STREAM_DURATION_ENTRY"
unset IFS
STREAM_DURATION=${STREAM_DURATION_ARRAY[${#STREAM_DURATION_ARRAY[@]}-1]}
IFS='.'
read -r -a DURATION <<< "$STREAM_DURATION"
unset IFS
STREAM_DURATION_NUMBER=$((DURATION[0]))

if [[ ${#DURATION[@]} -eq 0 ]]; then
  echo "Error: Duration of the stream is empty or 0"
  STREAM_DURATION_NUMBER=30
elif [[ STREAM_DURATION_NUMBER -lt $INTERVAL ]]; then
  echo "Error:Interval is greater than the duration of the video"
  INTERVAL=$DURATION
fi

SEEK_EVENT_LIST="Play,Playing,Ratechange,Pause,Seeking,Seeked,Clear"
FWD_BWD_EVENT_LIST="Play,Playing,Ratechange,Clear"
PLAY_EVENT_LIST="Play,Playing"

VALIDATE_EVENT=""
if [ "$OPERATION" == "muteunmute" ]; then
  VALIDATE_EVENT="Play,Playing,Ratechange,VolumeChange,Clear"
elif [ "$OPERATION" == "playpause" ]; then
  VALIDATE_EVENT="Play,Playing"
elif [ "$OPERATION" == "playtillend" ]; then
  SKIP_PROGRESS_LIST=("playtillend")
  VALIDATE_EVENT=$PLAY_EVENT_LIST
elif [ "$OPERATION" == "seekff" ]; then
  SKIP_PROGRESS_LIST=("seekff")
  VALIDATE_EVENT=$SEEK_EVENT_LIST
elif [[ "$OPERATION" =~ "fastfwd" ]]; then
  SKIP_PROGRESS_LIST=($OPERATION)
  VALIDATE_EVENT=$FWD_BWD_EVENT_LIST
elif [ "$OPERATION" == "rewind" ]; then
  SKIP_PROGRESS_LIST=("rewind")
  VALIDATE_EVENT=$FWD_BWD_EVENT_LIST
elif [ "$OPERATION" == "seekfwd" ] || [ "$OPERATION" == "seekbwd" ] || [ "$OPERATION" == "seekpos"  ] || [ "$OPERATION" == "trickplay" ] ; then
  SKIP_PROGRESS_LIST=("$OPERATION")
  VALIDATE_EVENT=$SEEK_EVENT_LIST
elif [ "$OPERATION" == "changetext" ]; then
  VALIDATE_EVENT="play,Playing,textchanged,clear"
else
  if [ "$PLAYER" == "aamp" ]; then
    VALIDATE_EVENT="playbackStarted"
  else
    VALIDATE_EVENT="Playing"
  fi
fi

echo "Validating $OPERATION Operation"
MONITOR_STREAM_OPERATION "$OPERATION" "$VALIDATE_EVENT" "$DATE_TIME"

POST_REQUISITE(){
  echo "                                                      "
  echo "****************************************************************Post Requisite***********************************************************************"
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  echo "Setting Post Conditions"
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  echo "                                                        "
  echo "Unloading Webkit module"
  WEBKIT_UNLOAD_COMMAND="(curl -H 'Content-Type: application/json' \
  -X POST \
  --data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser", "uri":"about:blank"}}' \
  'http://127.0.0.1:9998/jsonrpc')"
  WEBKIT_UNLOAD=$(curl -H 'Content-Type: application/json' \
  -X POST \
  --data-binary '{"jsonrpc": "2.0", "id": 1234567890, "method": "org.rdk.RDKShell.1.launch","params": {"callsign":"WebKitBrowser", "type":"WebKitBrowser", "uri":"about:blank"}}' \
  'http://127.0.0.1:9998/jsonrpc')
  echo "Json Command :$WEBKIT_UNLOAD_COMMAND"
  echo "Response : $WEBKIT_UNLOAD"
  echo "                                                        "
  echo "---------------------------------------------------------------------------------------------------------------------------------------"
  JSON_RESPONSE2=$(echo "$WEBKIT_UNLOAD" |  grep $REGEX_SUCCESS_PATTERN)
  if [[ -z "$JSON_RESPONSE2" ]]; then
    echo -e "Error: Problem in Unloading webkit browser: $WEBKIT_LOAD"
  else
    echo "Webkit Browser unload is successful"
    echo "---------------------------------------------------------------------------------------------------------------------------------------"
  fi
  echo "*********************************************************** End of Execution *************************************************************"
}

if [[ "$FINAL_RESULT" == "FAILURE" ]]; then
  echo "Test Execution is Failed for the operation: $OPERATION on the player: $PLAYER"
fi
POST_REQUISITE
echo "Validation of the script is: $FINAL_RESULT"
exit
