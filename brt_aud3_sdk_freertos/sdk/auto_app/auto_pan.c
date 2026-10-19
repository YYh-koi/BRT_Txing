/*************************************************************************
    > File Name: auto_app/app_pan.c
    > Author: hualiangyu
    > Mail: sheldon_wy@126.com
    > Created Time: Fri 26 Jun 2015 09:28:04 AM CST
 ************************************************************************/
#include<stdio.h>
#include "auto_pan.h"
#include "internal_pan.h"
#ifdef CONFIG_PAN
#define TAG "bse_pan"

void app_pan_init(module_event_cbk_t event_cbk)
{
	if(hlp_pan_get_status() == ST_PAN_UNINIT ) {
		hlp_pan_init(event_cbk);
	}
}

UINT8 app_pan_get_status(void)
{
	return hlp_pan_get_status();
}

void app_pan_done()
{
	if(hlp_pan_get_status() != ST_PAN_UNINIT ) {
		hlp_pan_done();
	}
}

void app_pan_connect(UINT8 *bd)
{
	if(hlp_pan_get_status() == ST_PAN_INIT ) {
		hlp_pan_connect(bd);
	}
}

void app_pan_disconnect()
{
	if(hlp_pan_get_status() > ST_PAN_CONNECTING) {
		hlp_pan_disconnect();
	}
}

profile_t g_pan_profile = {
	app_pan_init,
	app_pan_done,
	NULL,
	NULL,
};

#endif
