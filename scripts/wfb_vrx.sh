#!/bin/sh

airmon-ng check kill

sudo rmmod rtw88_8822cu
sudo rmmod rtw88_8822c
sudo rmmod rtw88_usb
sudo rmmod rtw88_core
sudo rmmod 8812cu

sudo insmod 8812cu.ko rtw_tx_pwr_by_rate=0 rtw_tx_pwr_lmt_enable=0

# nmcli device set wlan0 managed no
ip link set wlan0 down
iw dev wlan0 set monitor otherbss
iw reg set BO
ip link set wlan0 up

iw dev wlan0 set channel 161 HT20
# iwlist wlan0 channel
iw dev wlan0 info

#iw dev wlan0 set txpower fixed 1700
#cat /proc/net/rtl8812cu/wlan0/tx_power_idx

./wfb_rx -p 0 -u 5600 -K gs.key wlan0 &

gst-launch-1.0 udpsrc port=5600 \
    caps='application/x-rtp, media=(string)video, encoding-name=(string)H265' ! \
    rtph265depay ! \
    avdec_h265 ! \
    autovideosink

gst-launch-1.0 rtspsrc location=rtsp://192.168.100.10/live/0 ! decodebin ! autovideosink