#!/bin/sh

airmon-ng check kill
sleep 3

rmmod cfg80211
sleep 1

rmmod rtl8812cu_monitor
sleep 1

insmod /oem/usr/ko/cfg80211.ko
sleep 3
insmod /vtx/kmod/rtl8812cu_monitor.ko rtw_tx_pwr_by_rate=0 rtw_tx_pwr_lmt_enable=0
sleep 3

# nmcli device set wlan0 managed no
ip link set wlan0 down
# airmon-ng start wlan0
# iwconfig wlan0 mode monitor
# iw dev wlan0 set type monitor
iw dev wlan0 set monitor otherbss
iw reg set BO
ip link set wlan0 up

iw dev wlan0 set channel 100 HT20
# iwlist wlan0 channel
iw dev wlan0 info

#iw dev wlan0 set txpower fixed 1700
#cat /proc/net/rtl8812cu/wlan0/tx_power_idx

/vtx/bin/wfb_tx -p 0 -u 5602 -K drone.key wlan0 &
sleep 1

#ffmpeg -re -stream_loop -1 -i 123.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://127.0.0.1:5602"
/vtx/bin/luckfox_pico_rtp -i 127.0.0.1 -p 5602 -w 1920 -h 1080 -f 90 -e 1
