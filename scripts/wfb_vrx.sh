#!/bin/sh

airmon-ng check kill
sleep 3

sudo rmmod rtw88_8822cu
sleep 1
sudo rmmod rtw88_8822c
sleep 1
sudo rmmod rtw88_usb
sleep 1
sudo rmmod rtw88_core
sleep 1
sudo rmmod 8812cu
sleep 1

sudo insmod 8812cu.ko rtw_tx_pwr_by_rate=0 rtw_tx_pwr_lmt_enable=0
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

/vtx/bin/wfb_rx -p 0 -u 5600 -K gs.key wlan0 &
sleep 1

#ffmpeg -re -stream_loop -1 -i 123.mp4 -vcodec copy -pkt_size 1300 -f h264 "udp://127.0.0.1:5600"
# gst-launch-1.0 udpsrc port=5600 caps='application/x-rtp, media=(string)video, encoding-name=(string)H265' ! rtph265depay ! avdec_h265 ! autovideosink
gst-launch-1.0 udpsrc port=5600 \
    caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H265' \
    ! rtph265depay \
    ! avdec_h265 \
    ! clockoverlay valignment=bottom \
    ! autovideosink sync=false

gst-launch-1.0 -v udpsrc port=5600 \
    buffer-size=200000 ! \
    application/x-rtp,media=video,encoding-name=H265 ! \
    rtph265depay ! \
    queue max-size-buffers=3 leaky=downstream ! \
    avdec_h265 ! \
    videoconvert ! \
    autovideosink sync=false

gst-launch-1.0 udpsrc port=5600 \
    buffer-size=50000 \
    caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H265' \
    ! rtph265depay \
    ! queue max-size-buffers=1 leaky=downstream \
    ! nvh265dec \
    ! clockoverlay valignment=bottom \
    ! autovideosink sync=false