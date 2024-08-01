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

# config.txt
DIRECTORY_PATH="/lib/rdk"
BIN_DIR="/usr/bin"
LIB_DIR="/usr/lib"

libraries=(
	"libessosrmgr.so.0"
	"libessosrmgr.so"
	"libessosrmgr.so.0.0.0"
	"libessos.so"
	"libessos.so.0"
	"libessos.so.0.0.0"
	"libuinput.so"
	"libuinput.so.0.0.0"
	"libparodusclient.so"
	"libparodusclient.so.0.0.0"
	"libTr69BusAgent.so"
	"libTr69BusAgent.so.0.0.0"
	"libparodusclient.so.0"
	"libTr69BusAgent.so.0"
	"libWPEWebKit-1.0.so"
	"libWPEWebKit-1.0.so.3"  
	"libWPEWebKit-1.0.so.3.18.14"
	"libaampjsbindings.so"
	"libaamp.so"
	"libsubtec.so"
	"libabr.so"
	"libmetrics.so"
	"libRialtoClient.so"
	"libRialtoClient.so.1.0.0"
	"libRialtoServerManager.so"   
	"libRialtoServerManager.so.1.0.0"
	"libRialtoClient.so.1"
	"libRialtoServerMain.a"     
	"libRialtoServerManager.so.1"
)

binaries=(
	"DobbyTool"
	"IARMDaemonMain"
	"IARM_event_sender"
	"QueryPowerState"
	"RialtoServer"
	"SetPowerState"
	"WPEFramework"
	"WPEFrameworkSecurityUtility"
	"WPEProcess"
	"aamp-cli"
	"bluetoothctl"
	"bzip2"
	"crun"
	"curl"
	"dab-adapter"
	"deepSleepMgrMain"
	"dsMgrMain"
	"ermgr"
	"essos-sample"
	"essos-sample-resmgr"
	"groups"
	"gst-device-monitor-1.0"
	"gst-discoverer-1.0"
	"gst-inspect-1.0"
	"gst-launch-1.0"
	"gst-play-1.0"
	"gst-stats-1.0"
	"gst-transcoder-1.0"
	"gst-typefind-1.0"
	"mfrMgrMain"
	"mosquitto_passwd"
)

unwanted_lib=(
	"libtrm*"
	"libdvr*"
)
