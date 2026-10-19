/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_cb_screen.h
*
*	Description:
*
*
*	Create Date:		2014/05/29
***********************************************************************/

#include "auto_cb_screen.h"
#include "internal_cb_screen.h"
#include "auto_rfcomm.h"
#include "auto_hid.h"
#include "auto_iap.h"
#include "brt_trace.h"

#ifdef CONFIG_CBP
#define TAG "bse_cbp"

cb_screen_hid_send_coordinate hid_send_coordinate = app_hid_send_mouse_data;
static UINT32 spp_handle_t;
static UINT8 s_bd[BD_ADDR_LEN];

void hlp_cb_screen_spp_connect(UINT8 *bd);
void hlp_cb_screen_spp_disconnect(void);
void hlp_cb_screen_event_cbk(UINT32 event, void *param);
void hlp_cb_screen_iap_event_cbk(UINT8 event, void *param);

struct user_cb_screen_func cb_user_func = {
	hlp_cb_screen_spp_connect,
	hlp_cb_screen_spp_disconnect,
	NULL,
	NULL,
	NULL
};

static module_event_cbk_t s_cb_event_cbk;

static void cb_event_indicate(unsigned int event, void *param)
{
	if (s_cb_event_cbk != NULL) {
		s_cb_event_cbk((PROFILE_ID_CBP << 8) | event, param);
	}
}

void app_cb_screen_init(module_event_cbk_t event_cbk)
{
	s_cb_event_cbk = event_cbk;
	cb_user_func.event_cbk = cb_event_indicate;
	hlp_cb_screen_init(&cb_user_func);
#ifdef CONFIG_IAP2
	app_iap_init(hlp_cb_screen_iap_event_cbk);
#endif
}

UINT8 app_cb_screen_get_status(void)
{
	return hlp_cb_screen_get_status();
}

void app_cb_screen_start(void)
{
#ifdef CONFIG_IAP2
	app_iap_start();
#endif
}

void app_cb_screen_connect(UINT8 *bd, UINT16 resolution_w, UINT16 resolution_h)
{
	if (hlp_cb_screen_get_status() != ST_CB_SCREEN_READY) {
		return;
	}
	hlp_cb_screen_start(bd, resolution_w, resolution_h);
#ifdef CONFIG_IAP2
	if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS) {
		app_iap_hid_start();
	} else
#endif
	{
		if (hid_send_coordinate != NULL) {
			hid_send_coordinate(0, MIN_X_OF_MOUSE, MIN_Y_OF_MOUSE);
		}
	}
}

void app_cb_screen_disconnect(UINT8 val)
{
	if (hlp_cb_screen_get_status() <= ST_CB_SCREEN_READY) {
		return;
	}
#ifdef CONFIG_IAP2
	if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS) {
		if (val == CB_SCREEN_STOP_FLAG_DISCONNECT) {
			app_iap_disconnect();
			app_cb_screen_os_type("Android", NULL);
		} else {
			app_iap_hid_stop();
		}
	} else
#endif
	{
		hlp_cb_screen_stop();
	}
}

void app_cb_screen_stop(void)
{
}

void app_cb_screen_done()
{
	hlp_cb_screen_done();
#ifdef CONFIG_IAP2
	app_iap_done();
#endif
}

void app_cb_screen_coordinate_data(UINT8 button, INT16 x, INT16 y)
{
	INT16 x_convert = x;
	INT16 y_convert = y;

	if (hlp_cb_screen_get_status() == ST_CB_SCREEN_SUCCESS) {
		hlp_cb_screen_coordinate_data(button, &x_convert, &y_convert);
		bse_log_info(TAG, "app_cb_screen_coordinate_data, %d, %d", x_convert, y_convert);
		if (hid_send_coordinate != NULL) {
			hid_send_coordinate(button, x_convert, y_convert);
		}
	} else {
		bse_log_info(TAG, "app_cb_screen_coordinate_data, status error! %d\r\n", hlp_cb_screen_get_status());
	}
}

void app_cb_screen_send_key(UINT8 key_value)
{
#ifdef CONFIG_IAP2
	if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS) {
		app_iap_send_key_value(key_value, 0);
	} else
#endif
	{
		app_hid_send_key_value(key_value);
	}
}

UINT8 hlp_android_spp_event_cbk(UINT8 event, void *param)
{
	UINT8 ret = 0;

	bse_log_api(TAG, "hlp_android_spp_event_cbk, event=%d, param=%08x\r\n", event, param);
	switch(event) {
	case NF_CB_SCREEN_SPP_CONNECT: {
		rfc_con_info_t *conn_info = (rfc_con_info_t *)param;

		if(spp_handle_t == 0x01) {
			//Mark initiative connect android phone's SPP callback
			ret = 1;
			spp_handle_t = conn_info->handle;
		}
	}
	break;
	case NF_CB_SCREEN_SPP_DISCONNECT:
		if (spp_handle_t == (UINT32)param) {
			ret = 1;
			spp_handle_t = 0;
			if (cb_user_func.event_cbk != NULL) {
				cb_user_func.event_cbk(EV_BTC_CB_STOPPED, NULL);
			}
		}
		break;
	case NF_CB_SCREEN_SPP_DATA_IND: {
		struct app_spp_data_ind_stru *data_ind = (struct app_spp_data_ind_stru *)param;
		if (spp_handle_t == data_ind->handle) {
			ret = 1;
			cb_user_func.spp_data_ind(data_ind->data, data_ind->data_len);
		}
	}
	break;
	case NF_CB_SCREEN_HID_CONNECT:
		if(param != NULL) {
			memcpy(s_bd, param, BD_ADDR_LEN);
			hlp_cb_screen_set_status(ST_CB_SCREEN_READY);
		}
		break;
	case NF_CB_SCREEN_HID_DISCONNECT:
		hlp_cb_screen_clear();
		hlp_cb_screen_set_status(ST_CB_SCREEN_INIT);
		memset(s_bd, 0, BD_ADDR_LEN);
		break;
	default:
		break;
	}

	return ret;
}

UINT8 app_cb_screen_notify_ind(UINT8 event, void *param)
{
	UINT8 ret = 0;
	static UINT8 s_first_event = 0;
	static UINT32 server_handle = 0;
#ifdef CONFIG_IAP2
	if ((spp_handle_t == 0x00)
	    || (s_first_event == 0 && event == EV_SPP_SERVER_STARTED)
	    || (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS)) {
		if (event == EV_SPP_CONNECT_RET) {
			//Set the phone's OS type is IOS
			hlp_cb_screen_set_os_type("iPhone0,0");
		}
		ret = 1;
		app_iap_rfcomm_cbk(event, param);
	} else
#endif
	{
		ret = hlp_android_spp_event_cbk(event, param);
	}
	s_first_event = 1;
	return ret;
}

void hlp_cb_screen_spp_connect(UINT8 *bd)
{
#ifndef CONFIG_SPP_ANDROID
	UINT8 spp_uuid[16] = {0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};

	if (bd != NULL) {
		app_spp_connect_ex(0, bd, spp_uuid);
	}
#else
	cb_user_func.event_cbk(EV_BTC_CB_SPP_CONNECT_REQ, s_bd);
#endif
	//Mark initiative connect android phone's SPP
	spp_handle_t = 0x01;
}

void hlp_cb_screen_spp_disconnect(void)
{
#ifndef CONFIG_SPP_ANDROID
	app_spp_disconnect(spp_handle_t);
#else
	cb_user_func.event_cbk(EV_BTC_CB_SPP_DISCONNECT_REQ, NULL);
#endif
}

profile_t g_cbp_profile = {
	app_cb_screen_init,
	app_cb_screen_done,
	app_cb_screen_start,
	app_cb_screen_stop,
};

#endif

#ifdef CONFIG_IAP2
void app_cb_screen_os_type(UINT8 *p_os, UINT8 *bd)
{
	if (hlp_cb_screen_set_os_type(p_os) == CB_SCREEN_OS_TYPE_IOS) {
		app_iap_connect(bd);
		hid_send_coordinate = app_iap_send_mouse_data;
	} else {
		hid_send_coordinate = app_hid_send_mouse_data;
	}
}

void app_cb_screen_os_version(UINT8 *p_version)
{
	hlp_cb_screen_set_os_version(p_version);
}

void app_cb_screen_set_iphone_display_status(UINT8 display_status)
{
	UINT16 width = 0;
	UINT16 height = 0;

	if (hlp_cb_screen_get_os_type() != CB_SCREEN_OS_TYPE_IOS) {
		return;
	}
	hlp_cb_screen_get_iphone_pixel(&width, &height);
	if (display_status == CB_SCREEN_DISPLAY_STATUS_H) {
		hlp_cb_screen_set_local_info(width, height);
	} else {
		hlp_cb_screen_set_local_info(height, width);
	}
}

#endif


