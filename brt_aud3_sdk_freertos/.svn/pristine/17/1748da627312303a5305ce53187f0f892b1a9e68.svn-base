/*************************************************************************
    > File Name: app_dun.c
    >
    >
    > Created Time: Fri 07 Aug 2015 11:35:55 AM CST
 ************************************************************************/

#include<stdio.h>
#include "auto_dun.h"
#include "internal_dun.h"

void app_dun_init(module_event_cbk_t event_cbk)
{
	if(hlp_dun_get_status() == ST_DUN_UNINIT) {
		hlp_dun_init(event_cbk);
	}
}

UINT8 app_dun_get_status()
{
	return hlp_dun_get_status();
}

void app_dun_done()
{
	if(hlp_dun_get_status() != ST_DUN_UNINIT) {
		hlp_dun_done();
	}
}

void app_dun_connect(UINT8 *bd)
{
	if(hlp_dun_get_status() == ST_DUN_READY) { //ST_DUN_UNINIT)
		hlp_dun_connect(bd);
	}
}

void app_dun_disconnect()
{
	if(hlp_dun_get_status() >= ST_DUN_CONNECTING) {
		hlp_dun_disconnect();
	}
}

UINT8 app_dun_spp_event_cbk(UINT8 event, void *param)
{
	return hlp_dun_spp_event_cbk(event, param);
}

void app_dun_service_start()
{
	if(hlp_dun_get_status() == ST_DUN_READY) {
		hlp_dun_service_start();
	}
}

void app_dun_service_stop()
{
	if(hlp_dun_get_status() != ST_DUN_UNINIT) {
		hlp_dun_service_stop();
	}
}

profile_t g_dun_profile = {
	app_dun_init,
	app_dun_done,
	app_dun_service_start,
	app_dun_service_stop,
};

