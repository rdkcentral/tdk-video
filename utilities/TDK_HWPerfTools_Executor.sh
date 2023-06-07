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
#following function used to call the hardware performance test and metric_parser.sh file
#assign value to TDK_PATH
export TDK_PATH="/opt/TDK"
io()
{
        iozone -a -s 64 -r 64 -f /tmp/iozone.tmp > $TDK_PATH/logs/performance.log ; $TDK_PATH/HWPerf_metric_parser.sh  Iozone ; cat $TDK_PATH/logs/logparser-results.txt
}

switch()
{
        stress-ng --switch 0 -t 30 --metrics-brief --log-file /tmp/stressng-report.txt; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Context_Switching ; cat $TDK_PATH/logs/logparser-results.txt
}

stress()
{
       stress-ng --cpu 0 --cpu-method all -t 30 --metrics-brief --log-file /tmp/stressng-report.txt; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_CPU_Stress ; cat $TDK_PATH/logs/logparser-results.txt
}

malloc()
{
        stress-ng --malloc 0 -t 30 --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Malloc ; cat $TDK_PATH/logs/logparser-results.txt
}

memory_copy()
{
        stress-ng --memcpy 0 -t 30 --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Memory_Copying ; cat $TDK_PATH/logs/logparser-results.txt
}

pipe()
{
        stress-ng --pipe 0 -t 30 --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Pipe ; cat $TDK_PATH/logs/logparser-results.txt
}

stress_io()
{
        stress-ng --io 0 -t 30 --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Io ; cat $TDK_PATH/logs/logparser-results.txt
}

cpu()
{
        sysbench --test=cpu --cpu-max-prime=200 run > $TDK_PATH/logs/performance.log ; $TDK_PATH/HWPerf_metric_parser.sh sysbench_cpu_metric ; cat $TDK_PATH/logs/logparser-results.txt
}

memory()
{
        sysbench --test=memory --memory-block-size=1M --memory-total-size=1G run > $TDK_PATH/logs/performance.log ; $TDK_PATH/HWPerf_metric_parser.sh sysbench_memory_metric ; cat $TDK_PATH/logs/logparser-results.txt
}

membench()
{
        tinymembench > $TDK_PATH/logs/performance.log ; $TDK_PATH/HWPerf_metric_parser.sh TinymemBench ; cat $TDK_PATH/logs/logparser-results.txt
}

bench()
{
        cd /usr/bin/; nbench > $TDK_PATH/logs/performance.log ; $TDK_PATH/HWPerf_metric_parser.sh NBench ; cat $TDK_PATH/logs/logparser-results.txt
}

ipc()
{
	stress-ng --mq 4 --shm 4 --sem 4 -t 30s  --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_IPC ; cat $TDK_PATH/logs/logparser-results.txt
}

os()
{
	stress-ng --loop 1 -t 30s --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_OS ; cat $TDK_PATH/logs/logparser-results.txt
}

systemcalls()
{
        stress-ng --fork 8 --open 8 --close 8 --seek 8 --wait 8 --kill 8 -t 20s --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_System_calls ; cat $TDK_PATH/logs/logparser-results.txt

}

disk()
{
        stress-ng --class io --all 4 --exclude aiol,io-uring,rawdev,readahead -t 1m --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Disk_I/O ; cat $TDK_PATH/logs/logparser-results.txt
}

signals()
{
	stress-ng --sigfd 4 --sigfpe 4 --sigchld 4 --sigio 4 --sigpending 4 --sigsegv 4 --sigsuspend 4 --sigq 4 --sigtrap 4 -t 20s --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Signals ; cat $TDK_PATH/logs/logparser-results.txt
}

timer()
{
	stress-ng --timer 4 --timer-freq 1000000 --rtc 4 --clock 4 --itimer 4 --timerfd 4 -t 1m --times --metrics-brief  --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Timer ; cat $TDK_PATH/logs/logparser-results.txt
}

network()
{
	stress-ng --sequential 1 --class network --exclude dccp,sctp,sock -t 3m --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh stress-ng_Network ; cat $TDK_PATH/logs/logparser-results.txt
}

file()
{
	stress-ng --sequential 1 --class filesystem --exclude binderfs,chattr,fiemap,xattr,iomix,copy-file,fanotify,procfs -t 1s --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh File ; cat $TDK_PATH/logs/logparser-results.txt
}

thermalzonetest()
{
	stress-ng --matrix 0 --tz -t 60 --log-brief -v --times --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh ThermalZoneTest ; cat $TDK_PATH/logs/logparser-results.txt
}

schedulertest()
{
	stress-ng --sequential 1 --class scheduler -t 20s --exclude netlink-task,vforkmany,clone,sem-sysv,schedpolicy,session,hrtimers,softlockup --metrics-brief --log-file /tmp/stressng-report.txt ; $TDK_PATH/HWPerf_metric_parser.sh SchedulerTest ; cat $TDK_PATH/logs/logparser-results.txt
}


#$# count the number of arguments passed
arg_count=$#

#If there is no argument, the script will display the available tests
if [ $arg_count -eq 0 ];
then
	echo "-----------------------------------------------------------------"
	echo "             Hardware Performance Executor Tests                 "
	echo "-----------------------------------------------------------------"
        echo "[INFO] - Please choose your Test"
        echo "1   Iozone"
        echo "2   Stress-ng_Context_Switch"
        echo "3   Stress-ng_Cpu_Stress"
        echo "4   Stress-ng_Malloc"
        echo "5   Stress-ng_Memory_Copy"
        echo "6   Stress-ng_Pipe"
        echo "7   Stress-ng_Io"
        echo "8   Sysbench_Cpu"
        echo "9   Sysbench_Memory"
        echo "10  NBench"
        echo "11  Tinymembench"
	echo "12  IPC"
	echo "13  OS"
	echo "14  SystemCalls"
	echo "15  DiskInAndOut"
	echo "16  Signals"
	echo "17  Timer"
	echo "18  Network"
        echo "19  File"
	echo "20  ThermalZoneTest"
	echo "21  SchedulerTest"
	echo "22  all"
        read -p "Enter your Input : " value
        set -- $value
fi

#If the argument matches one of the cases listed below, the appropriate function will be called
case $1 in

        Iozone|1)
                io
                ;;
        Stress-ng_Context_Switch|2)
                switch
                ;;
        Stress-ng_Cpu_Stress|3)
		stress
                ;;
        Stress-ng_Malloc|4)
		malloc
                ;;
        Stress-ng_Memory_Copy|5)
                memory_copy
                ;;
        Stress-ng_Pipe|6)
		pipe
                ;;
        Stress-ng_Io|7)
		stress_io
                ;;
        Sysbench_Cpu|8)
		cpu
                ;;
        Sysbench_Memory|9)
		memory
                ;;
        NBench|10)
		bench
                ;;
        Tinymembench|11)
		membench
                ;;
	IPC|12)
		ipc
		;;
	OS|13)
		os
		;;
	SystemCalls|14)
		systemcalls
                ;;
        DiskInAndOut|15)
		disk
                ;;
	Signals|16)
		signals
                ;;
	Timer|17)
		timer
		;;
	Network|18)
		network
		;;
	File|19)
		file
		;;
	ThermalZoneTest|20)
		thermalzonetest
		;;
        SchedulerTest|21)
		schedulertest
		;;
        all|22)
                io
                switch
                stress
                malloc
                memory_copy
                pipe
                stress_io
                cpu
                memory
                membench
		ipc
		os
		systemcalls
		disk
		signals
		timer
		network
		file
		thermalzonetest
		schedulertest
                bench
                ;;
#if a user provides a false argument, it will print
        *)
                echo "[INFO] - Please enter a valid input"
esac
