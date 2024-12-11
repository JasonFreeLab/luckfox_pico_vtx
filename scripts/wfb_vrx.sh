#!/bin/sh

sudo airmon-ng check kill

sudo rmmod rtw88_8822cu
sudo rmmod rtw88_8822c
sudo rmmod rtw88_usb
sudo rmmod rtw88_core
sudo rmmod 8812cu

sudo insmod 8812cu.ko rtw_tx_pwr_by_rate=0 rtw_tx_pwr_lmt_enable=0

sudo systemctl start NetworkManager
systemctl status NetworkManager
sudo nmcli device set wlan0 managed no
nmcli device show wlan0

sudo ip link set wlan0 down
sudo iw dev wlan0 set monitor otherbss
sudo iw reg set BO
sudo ip link set wlan0 up

sudo iw dev wlan0 set channel 161 HT20
# iwlist wlan0 channel
iw dev wlan0 info

#sudo iw dev wlan0 set txpower fixed 1700
#sudo cat /proc/net/rtl8812cu/wlan0/tx_power_idx

./wfb_rx -p 0 -c 127.0.0.1 -u 5600 -K /etc/gs.key -i 7669206 wlan0 &

gst-launch-1.0 udpsrc port=5600 \
    caps='application/x-rtp, media=(string)video, encoding-name=(string)H265' ! \
    rtph265depay ! \
    avdec_h265 ! \
    autovideosink
