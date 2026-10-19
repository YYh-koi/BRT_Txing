#!/bin/bash

#export ATBM_WIFI_SVN_PATH=http://192.168.0.13/svn_server/work_svn/trunk/sw/Solution/ark_freertos/atbm_rtos_wifi6
#export ATBM_WIFI_LOACAL_DIR=atbm_rtos_wifi6
#
#echo "check out atbm_rtos_wifi6"
#svn export --quiet $@ $ATBM_WIFI_SVN_PATH $ATBM_WIFI_LOACAL_DIR
#
#cd $ATBM_WIFI_LOACAL_DIR
./630h_modify.sh

echo "modify ip for yadi"
sed -i 's/ucIPAddress\[4\] = {192, 168, 13, 1};/ucIPAddress\[4\] = {192, 168, 7, 1};/g' os/freertos/atbm_os_skbuf.c
sed -i 's/#define DHCP_POOL_START.*/#define DHCP_POOL_START         2/g' os/freertos/dhcps.h

echo "end!!!"

