#!/bin/bash

##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2024 RDK Management
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

device_name=$(grep '^MODEL_NUM=' /etc/device.properties | awk -F'=' '{print $2}')
output_file="/tmp/SYSTEM_FILE_CHECK_$device_name.txt"

# Function to read config file
read_config() {
    source config.sh
}


# Function to format date
format_date() {
    # Extract the BUILD_TIME dynamically from the version_output.txt file
    build_time=$(grep 'BUILD_TIME=' version_output.txt | grep -o 'BUILD_TIME="[0-9\-]* [0-9:]*"' | sed 's/BUILD_TIME="//' | sed 's/"//')
    formatted_date=$(echo $build_time | awk '{print $1}' | sed 's/-//g')
    formatted_date=$(echo ${formatted_date:4:4})
    echo "$formatted_date"
}


# Function to convert date string to YYYYMMDD format for comparison
convert_to_yyyymmdd() {

    local date_str=$1
    # Parse the month and day
    local month=$(echo $date_str | awk '{print $1}')
    local day=$(echo $date_str | awk '{print $2}')
    
    # Convert month to numerical value
    case $month in
        Jan) month_num="01" ;;
        Feb) month_num="02" ;;
        Mar) month_num="03" ;;
        Apr) month_num="04" ;;
        May) month_num="05" ;;
        Jun) month_num="06" ;;
        Jul) month_num="07" ;;
        Aug) month_num="08" ;;
        Sep) month_num="09" ;;
        Oct) month_num="10" ;;
        Nov) month_num="11" ;;
        Dec) month_num="12" ;;
        *) echo "Invalid month"; exit 1 ;;
    esac
    
    # Combine month and day into YYYYMMDD format
    echo "$month_num$day"
}

# Function to check if all files have the same or later date
check_file_dates() 
{
    sleep 3
    local directory_path="$DIRECTORY_PATH"
    #local output_file="/tmp/date_check_output.txt"

    # Execute command and save output to a file
    cat "/version.txt" > version_output.txt

    # Read the file and extract the date with grep
    formatted_date=$(format_date)

    if [[ -z "$formatted_date" ]]; then
        echo "Error: Failed to extract or format date from version_output.txt"
        exit 1
    fi

    # Convert the formatted date to YYYYMMDD for comparison
    #formatted_date_num=$(convert_to_yyyymmdd "$formatted_date")

    # Check all files in the directory
    echo "Checking files in directory: $directory_path"
    echo "Expected date format: $formatted_date"

    mismatch_found=false

    for file in "$directory_path"/*; do
        if [[ -f "$file" ]]; then
            file_date=$(date -r "$file" "+%b %d")
            file_date_num=$(convert_to_yyyymmdd "$file_date")

            # Compare dates using their numerical representations
            if [[ $file_date_num < $formatted_date ]]; then
                echo "Mismatch found: File '$file' has date '$file_date' which is earlier than the expected date '$formatted_date'" >> "$output_file"
                echo "Mismatch found: File '$file' has date '$file_date'"
                mismatch_found=true
            elif [[ $file_date_num > $formatted_date ]]; then
                echo "Mismatch found: File '$file' has date '$file_date' which is later than the expected date '$formatted_date'" >> "$output_file"
                echo "Mismatch found: File '$file' has date '$file_date'"
	    else
		echo "File '$file' has date '$file_date'" >> "$output_file"
            fi
        fi
    done
    echo "All files have the same date: $formatted_date"
    echo "TIME STAMP Check Output saved to: $output_file"
}

#Function to check all mandatory libraries are present or not
Library_check() 
{
    printf "\n\n" 
    printf '*%.0s' {1..100} 
    printf "\n"
    printf "%45s"
    printf "LIBRARY CHECK..." 
    printf "LIBRARY CHECK..." >> "$output_file"
    printf "\n"
    printf '*%.0s' {1..100}
    printf "\n"
    sleep 3
    all_present=true
    for lib in "${libraries[@]}"; do
        if [[ -f "$LIB_DIR/$lib" ]]; then
            echo "$lib is present in $LIB_DIR" >> "$output_file"
            #echo "$lib is present in $LIB_DIR"
        else
            echo "$lib is not present in $LIB_DIR" >> "$output_file"
            echo "$lib is not present in $LIB_DIR"
            all_present=false
        fi
    done

    if $all_present; then
        echo "All mandatory libraries are present in the device" >> "$output_file"
        echo "All mandatory libraries are present in the device"
    fi
    printf "\n"
    echo "LIBRARY Check Output saved to: $output_file"
    printf "\n\n" >> "$output_file"
    printf "\n\n"
}

#Function to check all mandatory binaries are present or not
binary_check() 
{
    printf '*%.0s' {1..100}
    printf "\n"
    printf "%45s" 
    printf "BINARY CHECK..."
    printf "BINARY_CHECK...." >> "$output_file"
    printf "\n" 
    printf '*%.0s' {1..100}
    printf "\n"
    sleep 3
    all_present=true
    for bin in "${binaries[@]}"; do
        if [[ -f "$BIN_DIR/$bin" ]]; then
            echo "$bin is present in $BIN_DIR" >> "$output_file"
            #echo "$bin is present in $BIN_DIR"
        else
            echo "$bin is not present in $BIN_DIR" >> "$output_file"
            echo "$bin is not present in $BIN_DIR"
            all_present=false
        fi
    done

    if $all_present; then
        echo "All binaries are present in the device" >> "$output_file"
        echo "All binaries are present in the device"
    fi
    printf "\n"
    echo "BINARY check Output saved to: $output_file"
    printf "\n\n"
    printf "\n\n" >> "$output_file"
}


#Function to check all unwanted files are present or not
Unwanted_lib_check()
{
    printf '*%.0s' {1..100}
    printf "\n"
    printf "%45s"
    printf "Unwanted LIB CHECK..."
    printf "Unwanted LIB CHECK..." >> "$output_file"
    printf "\n"
    printf '*%.0s' {1..100}
    printf "\n"
    sleep 3

    not_present=true
    for pattern in "${unwanted_lib[@]}"; do
        for lib in "$LIB_DIR"/$pattern; do
            if [[ -f "$lib" ]]; then
                echo "$(basename "$lib") is present in $LIB_DIR" >> "$output_file"
                echo "$(basename "$lib") is present in $LIB_DIR"
                not_present=false
            else
                echo "$pattern is not present in $LIB_DIR" >> "$output_file"
                echo "$pattern is not present in $LIB_DIR"
            fi
        done
    done

    if $not_present; then
        echo "All Unwanted libraries are not present in the device" >> "$output_file"
        echo "All Unwanted libraries are not present in the device"
    fi
    printf "\n"
    echo "UNWANTED LIB check Output saved to: $output_file"
    printf "\n\n"
}
	
# Main script

# Read config file
read_config

# Check for the function name argument and call the corresponding function
if [[ $# -eq 0 ]]; then
    # Execute all functions if no argument is provided
    check_file_dates
    Library_check
    binary_check
    Unwanted_lib_check
else
    case $1 in
        check_file_dates)
            check_file_dates
            ;;
        Library_check)
            Library_check
            ;;
        binary_check)
            binary_check
            ;;
        Unwanted_lib_check)
            Unwanted_lib_check
            ;;
        all)
            check_file_dates
            Library_check
            binary_check
            Unwanted_lib_check
            ;;
        *)
            echo "Error: Invalid function name '$1'"
            echo "Usage: $0 {check_file_dates|Library_check|binary_check|Unwanted_lib_check|all}"
            exit 1
            ;;
    esac
fi
