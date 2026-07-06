##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2022 RDK Management
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

TEST=$1
FILEPATH=$2

# Function to handle checking and extracting kernel config file
extract_kernel_config() {
    # Check if /proc/config.gz is present in DUT
    if [ ! -f "/proc/config.gz" ]; then
        echo "/proc/config.gz not found"
        exit
    else
        echo "/proc/config.gz found"
    fi

    # Kernel config file path
    filePath="$FILEPATH/config"

    # Copy the kernel config file and extract it
    if [ ! -f "$filePath" ]; then
        echo "Copying /proc/config.gz to $FILEPATH"
        cp /proc/config.gz "$FILEPATH"
        gunzip "$FILEPATH/config.gz"

        # Check if extraction was successful
        if [ -f "$filePath" ]; then
            echo "Kernel config extracted successfully to $filePath"
        else
            echo "Kernel config extraction failed"
            exit
        fi
    else
        echo "Config file already exists at $filePath"
    fi
}

# Function to check specific kernel configurations
check_kernel_configs() {
    local configs="$1" # Kernel config list passed as argument

    # Ensure resultFile is removed if it exists
    resultFile="$FILEPATH/resultFile"
    if [ -f "$resultFile" ]; then
        rm "$resultFile"
    fi

    if [ -f "$filePath" ]; then
        echo "Processing config file: $filePath"  # Debugging line

        # Loop through each kernel config and check its status
        for config in $configs; do
            enabled_pattern="${config}=y"
            disabled_pattern="# ${config} is not set"

            echo "Checking $config"  # Debugging line

            # Check if enabled
            if grep -q "^$enabled_pattern" "$filePath"; then
                echo "$config is enabled" >> "$resultFile"
            # Check if disabled
            elif grep -q "^$disabled_pattern" "$filePath"; then
                echo "$config is not set" >> "$resultFile"
            else
                echo "$config is not found in the config file" >> "$resultFile"
            fi
        done

        # Print the resultFile with newlines for each config entry
        cat "$resultFile"

        # Clean up
        rm "$filePath"
        rm "$resultFile"
    else
        echo "Extracted config file could not be found"
    fi
}

# Main script logic
if [ -z "$FILEPATH" ]; then
    echo "FILEPATH is not specified"
    echo "Taking /tmp as default FILEPATH"
    FILEPATH="/tmp"
fi

if [ "$TEST" == "--help" ]; then
    echo "Available test options : \"KERNEL_CONFIG_CHECK\" \"CHECK_KALLSYMS\" \"check_DEVMEM_CONFIG\" \"check_NX_Bit_CONFIG\""
    echo "Sample command : \"sh SecurityTestTDK.sh KERNEL_CONFIG_CHECK /tmp \""

elif [ "$TEST" == "KERNEL_CONFIG_CHECK" ]; then
    extract_kernel_config
    Kernel_Configs="CONFIG_DEBUG_KERNEL CONFIG_MARKERS CONFIG_DEBUG_MEMLEAK CONFIG_KPROBES CONFIG_SLUB_DEBUG CONFIG_PROFILING CONFIG_DEBUG_FS \
                    CONFIG_KPTRACE CONFIG_KALLSYMS CONFIG_LTT CONFIG_UNUSED_SYMBOLS CONFIG_TRACE_IRQFLAGS_SUPPORT CONFIG_RELAY CONFIG_MAGIC_SYSRQ \
                    CONFIG_VM_EVENT_COUNTERS CONFIGU_UNWIND_INFO CONFIG_BPA2_ALLOC_TRACE CONFIG_PRINTK CONFIG_CRASH_DUMP CONFIG_BUG CONFIG_SCSI_LOGGING \
                    CONFIG_ELF_CORE CONFIG_FULL_PANIC CONFIG_TASKSTATUS CONFIG_AUDIT CONFIG_BSD_PROCES S_ACCT CONFIG_KEXEC CONFIG_EARLY_PRINTK CONFIG_IKCONFIG \
                    CONFIG_NETFILTER_DEBUG CONFIG_MTD_UBI_DEBUG CONFIG_B43_DEBUG CONFIG_SSB_DEBUG CONFIG_FB_INTEL_DEBUG CONFIG_TRACING CONFIG_PERF_EVENTS"
    check_kernel_configs "$Kernel_Configs"

elif [ "$TEST" == "CHECK_KALLSYMS" ]; then
    extract_kernel_config
    CHECK_KALLSYMS="CONFIG_KALLSYMS"
    check_kernel_configs "$CHECK_KALLSYMS"

elif [ "$TEST" == "check_DEVMEM_CONFIG" ]; then
    extract_kernel_config
    check_DEVMEM="CONFIG_DEVMEM CONFIG_STRICT_DEVMEM CONFIG_IO_STRICT_DEVMEM"
    check_kernel_configs "$check_DEVMEM"

elif [ "$TEST" == "check_NX_Bit_CONFIG" ]; then
    extract_kernel_config
    check_nx_bit="CONFIG_STRICT_KERNEL_RWX CONFIG_STRICT_MODULE_RWX CONFIG_DEBUG_RODATA"
    check_kernel_configs "$check_nx_bit"

elif [ "$TEST" == "check_PAN_CONFIG" ]; then
    extract_kernel_config
    check_PAN="CONFIG_ARM64_PAN"
    check_kernel_configs "$check_PAN"

elif [ "$TEST" == "check_PAC_CONFIG" ]; then
    extract_kernel_config
    check_PAC="CONFIG_STACKPROTECTOR_STRONG CONFIG_ARM64_PTR_AUTH CONFIG_ARM64_PTR_AUTH_KERNEL"
    check_kernel_configs "$check_PAC"

elif [ "$TEST" == "check_STACK_CONFIG" ]; then
    extract_kernel_config
    check_stack_config="CONFIG_STACKPROTECTOR CONFIG_STACKPROTECTOR_STRONG"
    check_kernel_configs "$check_stack_config"

elif [ "$TEST" == "check_CFI_CONFIG" ]; then
    extract_kernel_config
    check_CFI_config="CONFIG_LTO_CLANG CONFIG_CFI_CLANG"
    check_kernel_configs "$check_CFI_config"

elif [ "$TEST" == "check_KASLR_CONFIG" ]; then
    extract_kernel_config
    check_kaslr_config="CONFIG_RELOCATABLE CONFIG_RANDOMIZE_BASE CONFIG_RANDOMIZE_MODULE_REGION_FULL"
    check_kernel_configs "$check_kaslr_config"

else
    echo "Not a valid test option"
    echo "Available test options : \"KERNEL_CONFIG_CHECK\" \"CHECK_KALLSYMS\" \"check_DEVMEM_CONFIG\" \"check_NX_Bit_CONFIG\" \"check_PAN_CONFIG\" \"check_PAC_CONFIG\" \"check_STACK_CONFIG\" \"check_CFI_CONFIG\" \"check_KASLR_CONFIG\""
fi

