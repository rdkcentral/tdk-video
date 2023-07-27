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
#Test Objective     : To Validate Ethernet_Wifi Toggle functionality.
#Automation_approch : To Toggle to Ethernet/Eth0 or Wifi/Wlano interfaces based on currently connected interface and Validate the ping response.
#Expected Output    : When network interface is altered the test case expects ping response to be success within the given timeout.
date
echo -e "Executing script : system_sanity_check_eth_wifi_toggle_functionality"
echo -e "======================================="
echo -e "Test Execution Name is: system_sanity_check_eth_wifi_toggle_functionality"

#Read the config file
config_file="$(dirname "$0")/sanity_check.config"
if [ ! -f "$config_file" ];then
echo "Please place the config file $config_file in the path and re-execute script"
exit 1
fi

source "$config_file"

#interface names
eth_interface="eth0"
wifi_interface="wlan0"


#Check if ssid, password,timeout,wpa_supplicant_file_path and security are configured
missing_parameters=()

[ -z "$ssid" ] && missing_parameters+=("SSID")
[ -z "$password" ] && missing_parameters+=("PASSWORD")
[ -z "$wpa_supplicant_file_path" ] && missing_parameters+=("WPA SUPPLICANT FILEPATH")
[ -z "$security" ] && missing_parameters+=("SECURITY MODE")

if [ ${#missing_parameters[@]} -gt 0 ]; then
  echo "Error: ${missing_parameters[*]} not found in config file.Pls configure and re-run."
  exit 1
fi

#Read SSID,password,wpa supplicant file path,timeout & security from configuration file
ssid=$(grep "ssid=" "$config_file" | cut -d "=" -f 2 | tr -d '[:space:]')
password=$(grep "password=" "$config_file" | cut -d "=" -f 2 | tr -d '[:space:]')
wpa_supplicant_file_path=$(grep "wpa_supplicant_file_path=" "$config_file" | cut -d "=" -f 2 | tr -d '[:space:]')
security=$(grep "security=" "$config_file" | cut -d "=" -f 2 | tr -d '[:space:]')

#Append network configuration to wpa_supplicant.conf
echo -e "network={
  ssid=\"$ssid\"
  password=\"$password\"
  key_mgmt=$security
}" >> "$wpa_supplicant_file_path"

#Check if the wpa_supplicant service is inactive
if [ "$(systemctl is-active wpa_supplicant)" = "inactive" ]; then
  systemctl restart wpa_supplicant
  systemctl status wpa_supplicant
fi

#Get the currently connected interface - eth0/wlan0
current_connection=$(ip route | head -n 1 | awk '/^default/ {print $5}')

#When Current interface is ethernet-eth0 : Connecting to wifi
if [ "$current_connection" = "$eth_interface" ]; then
  echo "Currently DUT is connected to Ethernet"
  #kill previous wpa_supplicant and previous processes and files if any
  pkill wpa_supplicant > /dev/null
  if [ -f /var/run/wpa_supplicant/wlan0 ] || [ -f /var/run/wpa_supplicant ]; then
      rm -rf /var/run/wpa_supplicant/wlan0
      rm -rf /var/run/wpa_supplicant
  fi
	echo "Toggling wifi network"
  #Down/Disable eth0 interface and Up/Enable wlan0 interface
  ip link set "$eth_interface" down
  ip link set "$wifi_interface" up
  SECONDS=0
  timeout=60
  #Initialize IP_ADDRESS variable
  IP_ADDRESS=""
  #Extract the assigned IP,Connect to Wi-Fi network using wpa_supplicant&Ping the extracted Wifi interface IP
  until [ -n "$IP_ADDRESS" ] && ping -c 1 -W 5 "$IP_ADDRESS" ; do
    if [[ $SECONDS -gt $timeout  ]];then
      echo "FAILURE: Timeout: Ethernet to WiFi toggle failed"
      ip link set "$eth_interface" up && exit 1
    fi
    wpa_passphrase "$ssid" "$password" "$security" | tee "$wpa_supplicant_file_path" >/dev/null
    wpa_supplicant -B -i "$wifi_interface" -c "$wpa_supplicant_file_path" >/dev/null
    ip addr show > /dev/null
    IP_ADDRESS=$(timeout 10s udhcpc -i "$wifi_interface" 2>&1 | grep -oE 'lease of [0-9.]+ ' | awk '{print $3}')
    #Waiting for the device to be up after the interface change
    sleep 5
  done
  echo "SUCCESS: Ethernet to WiFi toggle successful"
  echo "Ethernet to wifi interface change took $SECONDS seconds"
  echo " "
  echo "Toggling ethernet network"
  #Down/Disable wlan0 interface and Up/Enable eth0 interface
  ip link set "$wifi_interface" down
  ip link set "$eth_interface" up
  SECONDS=0
  timeout=60
  #Initialize IP_ADDRESS variable
  IP_ADDRESS=""
  #Ping the extracted Eth interface Ip
  until [ -n "$IP_ADDRESS" ] && ping -c 1 -W 5 "$IP_ADDRESS" ; do
    if [[ $SECONDS -gt $timeout  ]];then
      echo "FAILURE: Timeout: Wifi to Ethernet toggle failed"
      ip link set "$eth_interface" up && exit 1
    fi
    #Waiting for the device to be up after the interface change
    sleep 5
    ip addr show > /dev/null
    IP_ADDRESS=$(timeout 10s udhcpc -i "$eth_interface" 2>&1 | grep -oE 'lease of [0-9.]+ ' | awk '{print $3}')
  done
  echo "SUCCESS: Wifi to Ethernet toggle successful"
  echo "Wifi to ethernet interface change took $SECONDS seconds"
  echo " "
#When Current interface is Wifi-Wlan0 : Connecting to Ethernet
else
  echo "Currently DUT is connected to Wifi"
  echo "Toggling ethernet network"
  #Down/Disable wlan0 interface and Up/Enable eth0 interface
  ip link set "$wifi_interface" down
  ip link set "$eth_interface" up
  SECONDS=0
  timeout=60
  #Initialize IP_ADDRESS variable
  IP_ADDRESS=""
  #Extract the assigned IP address & Ping
  until  [ -n "$IP_ADDRESS" ] && ping -c 1 -W 5 "$IP_ADDRESS" ; do
    if [[ $SECONDS -gt $timeout  ]];then
      echo "FAILURE: Timeout: Wifi to Ethernet toggle failed"
      ip link set "$wifi_interface" up && exit 1
    fi
    #waiting for the device to be up after the interface change
    sleep 5
    ip addr show >/dev/null
    IP_ADDRESS=$(timeout 10s udhcpc -i "$eth_interface" 2>&1 | grep -oE 'lease of [0-9.]+ ' | awk '{print $3}')
  done
  echo "SUCCESS: Wifi to Ethernet toggle successful"
  echo "Wifi to ethernet interface change took $SECONDS seconds"
  echo " "
  echo "Toggling wifi network"
  #Down/Disable eth0 interface and Up/Enable wlan0 interface
  ip link set "$eth_interface" down
  ip link set "$wifi_interface" up
  SECONDS=0
  timeout=60
  #Initialize IP_ADDRESS variable
  IP_ADDRESS=""
  #Extract the assigned IP address for the WiFi interface
  IP_ADDRESS=$(timeout 10s udhcpc -i "$wifi_interface" 2>&1 | grep -oE 'lease of [0-9.]+ ' | awk '{print $3}')
  #Ping the extracted Wifi interface IP
  until [ -n "$IP_ADDRESS" ] && ping -c 1 -W 5 "$IP_ADDRESS" ; do
    if [[ $SECONDS -gt $timeout  ]];then
      echo "FAILURE: Timeout: Ethernet to WiFi toggle failed"
      ip link set "$wifi_interface" up && exit 1
    fi
    #Waiting for the device to be up after the interface change
    sleep 5
    ip addr show >/dev/null
    IP_ADDRESS=$(timeout 10s udhcpc -i "$wifi_interface" 2>&1 | grep -oE 'lease of [0-9.]+ ' | awk '{print $3}')
  done
  echo "SUCCESS: Ethernet to WiFi toggle successful"
  echo "Ethernet to wifi interface change took $SECONDS seconds"
  echo " "
fi
echo "---------------------------------------------------------------------------------------"

