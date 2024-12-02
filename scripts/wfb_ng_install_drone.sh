#!/bin/sh

cd /vtx/wfb_ng
gzip -dk /vtx/wfb_ng/wfb_ng-24.8.2.linux-arm.tar.gz
tar -xf /vtx/wfb_ng/wfb_ng-24.8.2.linux-arm.tar -C /vtx/wfb_ng
rm /vtx/wfb_ng/wfb_ng-24.8.2.linux-arm.tar
cd -

cp -rv /vtx/wfb_ng/etc /
cp -rv /vtx/wfb_ng/usr /

cp -rv /vtx/wfb_ng/home/jc/miniconda3/envs/luckfox/bin /usr
cp -rv /vtx/wfb_ng/home/jc/miniconda3/envs/luckfox/lib /usr

rm -rf /vtx/wfb_ng/etc /vtx/wfb_ng/home /vtx/wfb_ng/lib /vtx/wfb_ng/usr


cp -v /vtx/wfb_ng/drone.key /etc

cp -v /vtx/scripts/wifibroadcast.cfg /etc


cp -v /vtx/scripts/S98wlan0_setup /etc/init.d
chmod 755 /etc/init.d/S98wlan0_setup

cp -v /vtx/scripts/S98luckfox_pico_rtp /etc/init.d
chmod 755 /etc/init.d/S98luckfox_pico_rtp

cp -v /vtx/scripts/S99wifibroadcast /etc/init.d
chmod 755 /etc/init.d/S99wifibroadcast

cp -v /vtx/scripts/S99eth0_staticip /etc/init.d
chmod 755 /etc/init.d/S99eth0_staticip
