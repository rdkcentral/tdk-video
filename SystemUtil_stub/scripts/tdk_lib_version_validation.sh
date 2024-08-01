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


# Directory containing the RTK files
LIB_DIR="/usr/lib"
# Directory where the output file will be stored and checked
OUTPUT_DIR="/tmp"

# Output files for each device
OUTPUT_FILE_DEVICE1="$OUTPUT_DIR/sorted_files_device1.csv"
OUTPUT_FILE_DEVICE2="$OUTPUT_DIR/sorted_files_device2.csv"
DEVICE2_OUTPUT_FILE="$OUTPUT_DIR/device2.csv"
OUTPUT_FILE_DEVICE3="$OUTPUT_DIR/sorted_files_device3.csv"
DEVICE3_OUTPUT_FILE="$OUTPUT_DIR/device3.csv"
FINAL_OUTPUT_FILE="$OUTPUT_DIR/comparison_files.csv"
OUTPUT_FILE_DEVICE4="$OUTPUT_DIR/comparison_device4_files.csv"

generate_versions() {
    #local device_num=$1
    #local output_file=$2
    echo "Generating versions file for device $device_num..."

    # Create or clear the output file
    > "$OUTPUT_FILE_DEVICE1"

    # Add the header to the CSV file
    device_name=$(grep 'VERSION=' /version.txt | awk -F'=' '{print $2}' | grep -oE '\b[A-Z0-9]+(_[A-Z0-9]+)*\b' | grep -oE '^[A-Z]+' | head -n 1)
    echo "lib_name,$device_name" >> "$OUTPUT_FILE_DEVICE1"
    echo "lib_name,$device_name"
    #echo "lib_name,version_device${device_num}" > "$output_file"

    # Collect all .so files into an array
    files=($(ls "$LIB_DIR"/*.so* 2>/dev/null))

    # Process each file
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
	    base_name=$(basename "$file")
            lib_name=$(echo $base_name | sed 's/\(.*\)\.so.*/\1/')
	    version=$(echo "$base_name" | grep -o '\.so\..*')
            if [[ -z $version ]]; then
                version=".so"  # If only ".so" is found
            fi
            #if [[ -z $version ]]; then
                #version=".so"  # If only ".so" is found
            #fi
            echo "$lib_name,$version" >> "$OUTPUT_FILE_DEVICE1"
            echo "Processed $file: lib_name=$lib_name, version=$version"
        fi
    done

    # Create a temporary file excluding the header line for sorting
    temp_file=$(mktemp)
    grep -v '^lib_name,' "$OUTPUT_FILE_DEVICE1" > "$temp_file"

    # Sort the temporary file and append the header
    sort -t, -k1,1 "$temp_file" >> "$OUTPUT_FILE_DEVICE1"

    # Clean up
    rm "$temp_file"

    # Sort the output file by library name
    #sort -t, -k1,1 "$OUTPUT_FILE_DEVICE1" -o "$OUTPUT_FILE_DEVICE1"

    echo "Output saved to $OUTPUT_FILE_DEVICE1. Transfer this file to the next device."
}

compare_versions_device2() {
    echo "Comparing versions for device 2..."

    # Create or clear the output file for device 2
    > "$OUTPUT_FILE_DEVICE2"
    echo "lib_name,version_device1,version_device2,status" > "$OUTPUT_FILE_DEVICE2"
    echo "lib_name,version_device1,version_device2" > "$DEVICE2_OUTPUT_FILE"

    # Read the versions from device 1 into arrays
    device1_libs=()
    device1_versions=()
    while IFS=, read -r lib_name version; do
        if [[ $lib_name != "lib_name" ]]; then
            device1_libs+=("$lib_name")
            device1_versions+=("$version")
        fi
    done < "$OUTPUT_FILE_DEVICE1"

    # Read the versions from device 2 into arrays
    device2_libs=()
    device2_versions=()

    # Collect all .so files into an array
    files=($(ls "$LIB_DIR"/*.so* 2>/dev/null))

    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
	    base_name=$(basename "$file")
	    lib_name=$(echo $base_name | sed 's/\(.*\)\.so.*/\1/')
            version=$(echo "$base_name" | grep -o '\.so\..*')
            if [[ -z $version ]]; then
                version=".so"  # If only ".so" is found
            fi
            device2_libs+=("$lib_name")
            device2_versions+=("$version")
        fi
    done

    # Initialize an array to keep track of compared libraries in device 2
    compared_device2=()
    for ((i = 0; i < ${#device2_libs[@]}; i++)); do
        compared_device2[$i]=0
    done

    # Perform the comparison, prioritizing exact matches
    for ((i = 0; i < ${#device1_libs[@]}; i++)); do
        lib_name="${device1_libs[$i]}"

        version_device1="${device1_versions[$i]}"
        found_exact=0

        # Check for exact version match
        for ((j = 0; j < ${#device2_libs[@]}; j++)); do
            if [[ "${device2_libs[$j]}" == "$lib_name" ]] && [[ "${device2_versions[$j]}" == "$version_device1" ]]; then
                echo "$lib_name,$version_device1,${device2_versions[$j]},SAME" >> "$OUTPUT_FILE_DEVICE2"
		echo "$lib_name,$version_device1,${device2_versions[$j]}" >> "$DEVICE2_OUTPUT_FILE"
                echo "Compared $lib_name: version_device1=$version_device1, version_device2=${device2_versions[$j]}, SAME"
                compared_device2[$j]=1
                found_exact=1
                break
            fi
        done

        # If no exact match found, mark it as N/A or DIFFERENT
        if [[ $found_exact -eq 0 ]]; then
            found_diff=0
            for ((j = 0; j < ${#device2_libs[@]}; j++)); do
                if [[ "${device2_libs[$j]}" == "$lib_name" ]] && [[ "${compared_device2[$j]}" -eq 0 ]]; then
                    echo "$lib_name,$version_device1,${device2_versions[$j]},DIFFERENT" >> "$OUTPUT_FILE_DEVICE2"
		    echo "$lib_name,$version_device1,${device2_versions[$j]}" >> "$DEVICE2_OUTPUT_FILE"
                    echo "Compared $lib_name: version_device1=$version_device1, version_device2=${device2_versions[$j]}, DIFFERENT"
                    compared_device2[$j]=1
                    found_diff=1
                    break
                fi
            done

            if [[ $found_diff -eq 0 ]]; then
                echo "$lib_name,$version_device1,N/A" >> "$OUTPUT_FILE_DEVICE2"
		echo "$lib_name,$version_device1,N/A" >> "$DEVICE2_OUTPUT_FILE"
                echo "Compared $lib_name: version_device1=$version_device1, version_device2=N/A"
            fi
        fi
    done

    # Mark remaining device 2 libraries as N/A for device 1
    for ((j = 0; j < ${#device2_libs[@]}; j++)); do
        if [[ "${compared_device2[$j]}" -eq 0 ]]; then
            device2_lib_name="${device2_libs[$j]}"
            device2_version="${device2_versions[$j]}"
            echo "$device2_lib_name,N/A,$device2_version" >> "$OUTPUT_FILE_DEVICE2"
	    echo "$device2_lib_name,N/A,$device2_version" >> "$DEVICE2_OUTPUT_FILE"
            echo "Compared $device2_lib_name: version_device1=N/A, version_device2=$device2_version"
        fi
    done

    echo "Comparison for device 2 complete. Results saved to $OUTPUT_FILE_DEVICE2."
    echo "Transfer $DEVICE2_OUTPUT_FILE to the Device3 for comparision"
}


compare_versions_device3() {
    echo "Comparing versions for device 3..."

    # Create or clear the output file for device 3
    > "$OUTPUT_FILE_DEVICE3"
    echo "lib_name,version_device1,version_device2,version_device3,status" > "$OUTPUT_FILE_DEVICE3"
    echo "lib_name,version_device1,version_device2,version_device3" > "$DEVICE3_OUTPUT_FILE"

    # Read the versions from device 2 into arrays
    device1_libs=()
    device1_versions=()
    device2_versions=()
    while IFS=, read -r lib_name version1 version2; do
        if [[ $lib_name != "lib_name" ]]; then
            device1_libs+=("$lib_name")
            device1_versions+=("$version1")
            device2_versions+=("$version2")
        fi
    done < "$DEVICE2_OUTPUT_FILE"

    # Collect all .so files into an array
    files=($(ls "$LIB_DIR"/*.so* 2>/dev/null))

    # Read the versions from device 3 into arrays
    device3_libs=()
    device3_versions=()
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            base_name=$(basename "$file")
	    lib_name=$(echo $base_name | sed 's/\(.*\)\.so.*/\1/')
            #version=$(echo $base_name | cut -d "." -f2-)
            version=$(echo "$base_name" | grep -o '\.so\..*')
            if [[ -z $version ]]; then
                version=".so"  # If only ".so" is found
            fi
            device3_libs+=("$lib_name")
            device3_versions+=("$version")
        fi
    done

    # Initialize an array to keep track of compared libraries in device 3
    compared_device3=()
    for ((i = 0; i < ${#device3_libs[@]}; i++)); do
        compared_device3[$i]=0
    done

    # Perform the comparison
    for ((i = 0; i < ${#device1_libs[@]}; i++)); do
        lib_name="${device1_libs[$i]}"
        version_device1="${device1_versions[$i]}"
        version_device2="${device2_versions[$i]}"
        found_exact=0

        # Check for exact version match in device 3
        for ((j = 0; j < ${#device3_libs[@]}; j++)); do
            if [[ "${device3_libs[$j]}" == "$lib_name" ]] && [[ "${device3_versions[$j]}" == "$version_device2" ]]; then
                echo "$lib_name,$version_device1,$version_device2,${device3_versions[$j]},SAME" >> "$OUTPUT_FILE_DEVICE3"
                echo "$lib_name,$version_device1,$version_device2,${device3_versions[$j]}" >> "$DEVICE3_OUTPUT_FILE"
                echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=${device3_versions[$j]}, SAME"
                compared_device3[$j]=1
                found_exact=1
                break
            fi
        done

        # If no exact match found, mark it as N/A or DIFFERENT
        if [[ $found_exact -eq 0 ]]; then
            found_diff=0
            for ((j = 0; j < ${#device3_libs[@]}; j++)); do
                if [[ "${device3_libs[$j]}" == "$lib_name" ]] && [[ "${compared_device3[$j]}" -eq 0 ]]; then
                    echo "$lib_name,$version_device1,$version_device2,${device3_versions[$j]},DIFFERENT" >> "$OUTPUT_FILE_DEVICE3"
                    echo "$lib_name,$version_device1,$version_device2,${device3_versions[$j]}" >> "$DEVICE3_OUTPUT_FILE"
                    echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=${device3_versions[$j]}, DIFFERENT"
                    compared_device3[$j]=1
                    found_diff=1
                    break
                fi
            done

            if [[ $found_diff -eq 0 ]]; then
                echo "$lib_name,$version_device1,$version_device2,N/A,DIFFERENT" >> "$OUTPUT_FILE_DEVICE3"
                echo "$lib_name,$version_device1,$version_device2,N/A" >> "$DEVICE3_OUTPUT_FILE"
                echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=N/A"
            fi
        fi
    done

    # Mark remaining device 3 libraries as N/A for device 1 and device 2
    for ((j = 0; j < ${#device3_libs[@]}; j++)); do
        if [[ "${compared_device3[$j]}" -eq 0 ]]; then
            device3_lib_name="${device3_libs[$j]}"
            device3_version="${device3_versions[$j]}"
            echo "$device3_lib_name,N/A,N/A,$device3_version,DIFFERENT" >> "$OUTPUT_FILE_DEVICE3"
            echo "$device3_lib_name,N/A,N/A,$device3_version" >> "$DEVICE3_OUTPUT_FILE"
            echo "Compared $device3_lib_name: version_device1=N/A, version_device2=N/A, version_device3=$device3_version"
        fi
    done

    # Filter out lines starting with "li," or "lib,"
    grep -v -E '^li,|^lib,' "$DEVICE3_OUTPUT_FILE" > "$OUTPUT_DIR/temp_output.csv"
    mv "$OUTPUT_DIR/temp_output.csv" "$DEVICE3_OUTPUT_FILE"

    echo "Comparison for device 3 complete. Results saved to $OUTPUT_FILE_DEVICE3."
    echo "Transfer $DEVICE3_OUTPUT_FILE to the Device4 for comparision"
}


compare_versions_device4() 
{

    echo "Comparing versions for device 4..."

    # Create or clear the final output file
    > "$OUTPUT_FILE_DEVICE4"
    echo "lib_name,version_device1,version_device2,version_device3,version_device4" > "$OUTPUT_FILE_DEVICE4"

# Read the versions from device 3 into arrays
device1_libs=()
device1_versions=()
device2_versions=()
device3_versions=()
while IFS=, read -r lib_name version1 version2 version3; do
    if [[ $lib_name != "lib_name" ]]; then
        device1_libs+=("$lib_name")
        device1_versions+=("$version1")
        device2_versions+=("$version2")
        device3_versions+=("$version3")
    fi
done < "$DEVICE3_OUTPUT_FILE"

# Collect all .so files into an array
files=($(ls "$LIB_DIR"/*.so* 2>/dev/null))

# Read the versions from device 4 into arrays
device4_libs=()
device4_versions=()
for file in "${files[@]}"; do
    if [[ -f "$file" ]]; then
        base_name=$(basename "$file")
	lib_name=$(echo $base_name | sed 's/\(.*\)\.so.*/\1/')
        version=$(echo "$base_name" | grep -o '\.so\..*')
        if [[ -z $version ]]; then
             version=".so"  # If only ".so" is found
        fi
        device4_libs+=("$lib_name")
        device4_versions+=("$version")
    fi
done

# Initialize an array to keep track of compared libraries in device 4
compared_device4=()
for ((i = 0; i < ${#device4_libs[@]}; i++)); do
    compared_device4[$i]=0
done

# Perform the comparison
for ((i = 0; i < ${#device1_libs[@]}; i++)); do
    lib_name="${device1_libs[$i]}"
    version_device1="${device1_versions[$i]}"
    version_device2="${device2_versions[$i]}"
    version_device3="${device3_versions[$i]}"
    found_exact=0

    # Check for exact version match in device 4
    for ((j = 0; j < ${#device4_libs[@]}; j++)); do
        if [[ "${device4_libs[$j]}" == "$lib_name" ]] && [[ "${device4_versions[$j]}" == "$version_device3" ]]; then
            echo "$lib_name,$version_device1,$version_device2,$version_device3,${device4_versions[$j]},SAME" >> "$OUTPUT_FILE_DEVICE4"
#            echo "$lib_name,$version_device1,$version_device2,$version_device3,${device4_versions[$j]}" >> "$DEVICE4_OUTPUT_FILE"
            echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=$version_device3, version_device4=${device4_versions[$j]}, SAME"
            compared_device4[$j]=1
            found_exact=1
            break
        fi
    done

    # If no exact match found, mark it as N/A or DIFFERENT
    if [[ $found_exact -eq 0 ]]; then
        found_diff=0
        for ((j = 0; j < ${#device4_libs[@]}; j++)); do
            if [[ "${device4_libs[$j]}" == "$lib_name" ]] && [[ "${compared_device4[$j]}" -eq 0 ]]; then
                echo "$lib_name,$version_device1,$version_device2,$version_device3,${device4_versions[$j]},DIFFERENT" >> "$OUTPUT_FILE_DEVICE4"
#                echo "$lib_name,$version_device1,$version_device2,$version_device3,${device4_versions[$j]}" >> "$DEVICE4_OUTPUT_FILE"
                echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=$version_device3, version_device4=${device4_versions[$j]}, DIFFERENT"
                compared_device4[$j]=1
                found_diff=1
                break
            fi
        done

        if [[ $found_diff -eq 0 ]]; then
            echo "$lib_name,$version_device1,$version_device2,$version_device3,N/A,DIFFERENT" >> "$OUTPUT_FILE_DEVICE4"
            #echo "$lib_name,$version_device1,$version_device2,$version_device3,N/A" >> "$DEVICE4_OUTPUT_FILE"
            echo "Compared $lib_name: version_device1=$version_device1, version_device2=$version_device2, version_device3=$version_device3, version_device4=N/A"
        fi
     fi
    
done

# Mark remaining device 4 libraries as N/A for device 1, device 2, and device 3
for ((j = 0; j < ${#device4_libs[@]}; j++)); do
    if [[ "${compared_device4[$j]}" -eq 0 ]]; then
        device4_lib_name="${device4_libs[$j]}"
        device4_version="${device4_versions[$j]}"
        echo "$device4_lib_name,N/A,N/A,N/A,$device4_version,DIFFERENT" >> "$OUTPUT_FILE_DEVICE4"
        #echo "$device4_lib_name,N/A,N/A,N/A,$device4_version" >> "$DEVICE4_OUTPUT_FILE"
        echo "Compared $device4_lib_name: version_device1=N/A, version_device2=N/A, version_device3=N/A, version_device4=$device4_version"
    fi
done

# Filter out lines starting with "li," or "lib,"
#grep -v -E '^li,|^lib,' "$DEVICE4_OUTPUT_FILE" > "$OUTPUT_DIR/temp_output.csv"
#mv "$OUTPUT_DIR/temp_output.csv" "$DEVICE4_OUTPUT_FILE"

echo "Comparison for device 4 complete. Results saved to $OUTPUT_FILE_DEVICE4."
}


# Determine which step to execute based on the presence of output files
if [[ ! -f $OUTPUT_FILE_DEVICE1 ]]; then
    generate_versions 1 "$OUTPUT_FILE_DEVICE1"
elif [[ -f $OUTPUT_FILE_DEVICE1 ]] && [[ ! -f $DEVICE2_OUTPUT_FILE ]]; then
    compare_versions_device2
elif [[ -f $DEVICE2_OUTPUT_FILE ]] && [[ ! -f $DEVICE3_OUTPUT_FILE ]]; then
    compare_versions_device3
elif [[ -f $DEVICE3_OUTPUT_FILE ]] && [[ ! -f $OUTPUT_FILE_DEVICE4 ]]; then
    compare_versions_device4
else
    echo "All comparisons are already done."
fi
