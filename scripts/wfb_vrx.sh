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

ip link set wlan0 down
# airmon-ng start wlan0
iwconfig wlan0 mode monitor
ip link set wlan0 up

iwconfig wlan0 channel 100
iwlist wlan0 channel

#iw dev wlan0 set txpower fixed 1700
#cat /proc/net/rtl8812cu/wlan0/tx_power_idx

./wfb_rx -p 0 -u 5600 -K gs.key wlan0 &
#sleep 1

#ffmpeg -re -stream_loop -1 -i 123.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://127.0.0.1:5602"
#gst-launch-1.0 udpsrc port=5600 caps='application/x-rtp, media=(string)video, encoding-name=(string)H265' ! rtph265depay ! avdec_h265 ! autovideosink