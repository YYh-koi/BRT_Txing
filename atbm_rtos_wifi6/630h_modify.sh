#!/bin/bash

#export ATBM_WIFI_SVN_PATH=http://192.168.0.13/svn_server/work_svn/trunk/sw/Solution/ark_freertos/atbm_rtos_wifi6
#export ATBM_WIFI_LOACAL_DIR=atbm_rtos_wifi6
#
#echo "check out atbm_rtos_wifi6"
#svn export --quiet $@ $ATBM_WIFI_SVN_PATH $ATBM_WIFI_LOACAL_DIR
#
#cd $ATBM_WIFI_LOACAL_DIR
echo "patch in 630h_sdio_function.patch!"
patch -p1 < 630h_sdio_function.patch
echo "patch end"
