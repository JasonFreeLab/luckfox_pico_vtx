#!/bin/sh

airmon-ng check kill
rmmod cfg80211
rmmod rtl8812cu_monitor
insmod /oem/usr/ko/cfg80211.ko
insmod /vtx/kmod/rtl8812cu_monitor.ko rtw_tx_pwr_by_rate=0 rtw_tx_pwr_lmt_enable=0

ip link set wlan0 down
iw dev wlan0 set monitor otherbss
iw reg set BO
ip link set wlan0 up

iw dev wlan0 set channel 161 HT20
# iwlist wlan0 channel
iw dev wlan0 info

#iw dev wlan0 set txpower fixed 1700
#cat /proc/net/rtl8812cu/wlan0/tx_power_idx

/usr/bin/wfb_tx -p 0 -u 5602 -K /vtx/wfb_ng/drone.key wlan0 &
/vtx/bin/luckfox_pico_rtp -i 127.0.0.1 -p 5000 -w 1920 -h 1080 -f 90 -e 1
