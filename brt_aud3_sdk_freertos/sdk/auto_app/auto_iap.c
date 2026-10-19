/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2015 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_iap.c
*
*	Description:		Internal Functions For IAP
*
*
*	Create Date:		2015/07/28
***********************************************************************/

#include "global.h"
#include "internal_iap.h"
#include "auto_iap.h"

#ifdef CONFIG_IAP2
#define TAG "bse_iap2"

static INT16 s_x = 0x00;
static INT16 s_y = 0x00;
static UINT8 s_flag_ctl = 0;

void app_iap_init(module_event_cbk_t event_cbk)
{
	hlp_iap_init(event_cbk);
}

void app_iap_done()
{
	hlp_iap_done();
}

void app_iap_start(void)
{
	hlp_iap_start();
}

void app_iap_connect(UINT8 *bd)
{
	hlp_iap_connect(bd);
}

void app_iap_disconnect(void)
{
	hlp_iap_disconnect();
}

void app_iap_hid_start(void)
{
	s_flag_ctl = 0;
	hlp_iap_hid_start();
}

void app_iap_hid_stop(void)
{
	hlp_iap_hid_stop();
}

void app_iap_rfcomm_cbk(UINT8 event, void *param)
{
	hlp_iap_rfcomm_event_cbk(event, param);
}

void app_iap_mouse_event(UINT8 ctrl, INT16 x, INT16 y)
{
	bse_log_api(TAG, "app_iap_hid_mouse, %d, %d, %d\r\n", ctrl, x, y);
	if (s_flag_ctl == 0 && ctrl == 1) {
		hlp_iap_mouse_coord(0, -250, -250);
		hlp_iap_mouse_coord(ctrl, x, y);
		s_x = x;
		s_y = y;
	} else if (s_flag_ctl == 1 && ctrl == 0) {
		hlp_iap_mouse_coord(0, 0, 0);
		hlp_iap_mouse_coord(1, 0, 0);
		hlp_iap_mouse_coord(0, 0, 0);
		//hlp_iap_mouse_coord(0, -600, -600);
		//app_iap_hid_report(0, s_middle_x, s_middle_y);
		s_x = 0;
		s_y = 0;
	} else if (s_flag_ctl == 1 && ctrl == 1) {
		hlp_iap_mouse_coord(ctrl, x - s_x, y - s_y);
		s_x = x;
		s_y = y;
	} else if (ctrl == 0xFF) {
		hlp_iap_mouse_coord(0, 0, 0);
	}

	s_flag_ctl = ctrl;
}

void app_iap_send_mouse_data(UINT8 button, INT16 x, INT16 y)
{
	app_iap_mouse_event(button, x, y);
	//hlp_iap_mouse_coord(0, -200, -200);
	//hlp_iap_mouse_coord(button, x, y);
}

void app_iap_send_mouse_test(UINT8 button, INT16 x, INT16 y)
{
	hlp_iap_send_hid_mouse(button, x, y);
}

void app_iap_send_key_value(UINT8 key_value_1, UINT8 key_value_2)
{
	hlp_iap_send_key_value(key_value_1, key_value_2);
}

#endif

