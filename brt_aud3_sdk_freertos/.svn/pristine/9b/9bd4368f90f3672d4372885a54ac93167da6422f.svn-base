/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_cb_screen.c
*
*	Description:
*
*
*	Create Date:		2014/05/29
***********************************************************************/

#include "internal_cb_screen.h"
#include "fsm.h"
#include "auto_iap.h"
#include "brt_trace.h"

#define TAG "bse_cbp"
#ifdef CONFIG_CBP
static struct cb_screen_data_stru *p_cb_screen_data = NULL;
static unsigned short display_range[2][2];
static UINT8 ratio_x = 2;
static UINT8 ratio_y = 2;
static struct FsmTimer *s_ft_start = NULL;

static const UINT16 g_iphone_display_pixel[3][2] = {
	{1136, 640},
	{1334, 750},
	{1920, 1080},
};

void hlp_cb_screen_init(struct user_cb_screen_func *p_user_func)
{
	if (p_user_func != NULL) {
		if (p_cb_screen_data == NULL) {
			p_cb_screen_data = (struct cb_screen_data_stru *)NEW(sizeof(struct cb_screen_data_stru));
		}
		memset(p_cb_screen_data, 0, sizeof(struct cb_screen_data_stru));

		p_user_func->spp_data_ind = hlp_cb_screen_spp_data_ind;
		p_cb_screen_data->spp_connect = p_user_func->spp_connect;
		p_cb_screen_data->spp_disconnect = p_user_func->spp_disconnect;
		p_cb_screen_data->event_cbk = p_user_func->event_cbk;

		p_cb_screen_data->state = ST_CB_SCREEN_INIT;
	}
}

void hlp_cb_screen_done(void)
{
	if (p_cb_screen_data != NULL) {
		FREE(p_cb_screen_data);
		p_cb_screen_data = NULL;
	}
}

void hlp_cb_screen_start(UINT8 *bd, UINT16 resolution_w, UINT16 resolution_h)
{
	if (p_cb_screen_data->state != ST_CB_SCREEN_READY || resolution_w == 0 || resolution_h == 0) {
		return;
	}

	p_cb_screen_data->resolution_w = resolution_w;
	p_cb_screen_data->resolution_h = resolution_h;
#ifdef CONFIG_IAP2
	if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_ANDROID)
#endif
	{
		hlp_cb_screen_set_status(ST_CB_SCREEN_STARTING);
		p_cb_screen_data->spp_connect(bd);
	}
}

void hlp_cb_screen_stop(void)
{
	if (p_cb_screen_data != NULL) {
		p_cb_screen_data->state = ST_CB_SCREEN_READY;
	}
	memset(display_range, 0, sizeof(display_range));
	p_cb_screen_data->spp_disconnect();
}

void hlp_cb_screen_clear(void)
{
	memset(display_range, 0, sizeof(display_range));
}

void hlp_cb_screen_set_local_info(UINT16 phone_width, UINT16 phone_height)
{
	p_cb_screen_data->phone_width = phone_width;
	p_cb_screen_data->phone_height = phone_height;

	if(p_cb_screen_data->resolution_w != 0 && p_cb_screen_data->resolution_h != 0) {
		UINT16 map_phone_w = p_cb_screen_data->resolution_h * phone_width / phone_height;
		UINT16 map_phone_h = p_cb_screen_data->resolution_h;

		if (phone_width < phone_height) {
			//Portrait mode
#ifdef CONFIG_IAP2
			if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS) {
				if (hlp_cb_screen_get_os_version() == CB_SCREEN_OS_VERSION_8P4S) {
					ratio_x = 7;
					ratio_y = 13;
				} else {
					if (p_cb_screen_data->iphone_version < CB_SCREEN_IPHONE_VERSION_6PLUS) {
						ratio_x = ratio_y = 7;
					} else {
						ratio_x = ratio_y = 9;
					}
				}
			} else {
				ratio_x = ratio_y = 2;
			}
#endif
			map_phone_w = p_cb_screen_data->resolution_h * phone_width / phone_height;
			map_phone_h = p_cb_screen_data->resolution_h;
			display_range[0][0] = p_cb_screen_data->resolution_w / 2 - map_phone_w / 2;
			display_range[0][1] = 0;
			display_range[1][0] = p_cb_screen_data->resolution_w / 2 + map_phone_w / 2;
			display_range[1][1] = map_phone_h;
		} else {
			//Landscape mode
#ifdef CONFIG_IAP2
			if (hlp_cb_screen_get_os_type() == CB_SCREEN_OS_TYPE_IOS) {
				if (hlp_cb_screen_get_os_version() == CB_SCREEN_OS_VERSION_8P4S) {
					ratio_x = 13;
					ratio_y = 7;
				} else {
					if (p_cb_screen_data->iphone_version < CB_SCREEN_IPHONE_VERSION_6PLUS) {
						ratio_x = ratio_y = 7;
					} else {
						ratio_x = ratio_y = 9;
					}
				}
			} else {
				ratio_x = ratio_y = 2;
			}
#endif
			map_phone_w = p_cb_screen_data->resolution_w;
			map_phone_h = p_cb_screen_data->resolution_w * phone_height / phone_width;
			display_range[0][0] = 0;
			display_range[0][1] = p_cb_screen_data->resolution_h / 2 - map_phone_h / 2;
			display_range[1][0] = map_phone_w;
			display_range[1][1] = p_cb_screen_data->resolution_h / 2 + map_phone_h / 2;
		}

		if (p_cb_screen_data->event_cbk != NULL && p_cb_screen_data->state == ST_CB_SCREEN_STARTING) {
			p_cb_screen_data->event_cbk(EV_BTC_CB_SUCCESS, NULL);
			hlp_cb_screen_set_status(ST_CB_SCREEN_SUCCESS);
		}
	}
}

void hlp_cb_screen_coordinate_data(UINT8 button, UINT16 *x, UINT16 *y)
{
	UINT16 x_source = *x;
	UINT16 y_source = *y;

	if (x_source < display_range[0][0]) {
		*x = 0;
	} else if (x_source > display_range[1][0]) {
		*x = p_cb_screen_data->phone_width << 1 / ratio_x;
	} else {
		*x = (((x_source-display_range[0][0])*p_cb_screen_data->phone_width)<<1) / ((display_range[1][0]-display_range[0][0])*ratio_x);
	}

	if (y_source < display_range[0][1]) {
		*y = 0;
	} else if (y_source > display_range[1][1]) {
		*y = p_cb_screen_data->phone_height << 1 / ratio_y;
	} else {
		*y = (((y_source-display_range[0][1])*p_cb_screen_data->phone_height)<<1) / ((display_range[1][1]-display_range[0][1])*ratio_y);
	}
}

void hlp_cb_screen_spp_data_ind(UINT8 *p_data, UINT32 len)
{
	static UINT16 cb_screen_pixel[2];
	cb_screen_spp_data_t *p_spp_data = NULL;

	while (len > 0) {
		if (p_data != NULL) {
			p_spp_data = (cb_screen_spp_data_t *)p_data;
			if ((p_spp_data->sync[0] == 0xAF) && (p_spp_data->sync[1] == 0xFA)) {
				UINT32 data_len = ((p_spp_data->length[0]&0xFF)|((p_spp_data->length[1]&0xFF)<<8));

				len -= data_len + 5;
				switch (p_spp_data->type) {
				case PACKET_TYPE_SCREEN_PIXEL_INFO: {
					cb_screen_pixel[0] = (p_spp_data->data[0]&0xFF)|(p_spp_data->data[1]&0xFF)<<8;
					cb_screen_pixel[1] = (p_spp_data->data[2]&0xFF)|(p_spp_data->data[3]&0xFF)<<8;
					if (p_cb_screen_data && (data_len == 0x4)) {
						hlp_cb_screen_set_local_info(cb_screen_pixel[0], cb_screen_pixel[1]);
					}
				}
				break;
				case PACKET_TYPE_SCREEN_ROTATION_INFO: {
					if (data_len == 0x02) {
					}
				}
				break;
				default:
					break;
				}
			} else {
			}
		} else {
			bse_log_info(TAG, "[CB]Receive From SPP Data Format is Error!\n");
		}
	}
}


UINT8 hlp_cb_screen_get_status(void)
{
	if(p_cb_screen_data) {
		return p_cb_screen_data->state;
	} else {
		return ST_CB_SCREEN_UNINIT;
	}
}

void hlp_cb_screen_set_status(UINT8 state)
{
	bse_log_info(TAG, "hlp_cb_screen_set_status, now is %d, you set status=%d\r\n", p_cb_screen_data->state, state);
	p_cb_screen_data->state = state;
}

#ifdef CONFIG_IAP2
UINT8 hlp_cb_screen_set_os_type(UINT8 *p_os)
{
	if (strncmp(p_os, "iPhone", 6) == 0) {
		UINT8 *p_version = p_os + 6;

		p_cb_screen_data->phone_os_type = CB_SCREEN_OS_TYPE_IOS;
		bse_log_api(TAG, "hlp_cb_screen_os_type, %s\r\n", p_version);
		switch(p_version[0]) {
		case '5':
			p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_5;
			break;
		case '6':
			p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_5S;
			break;
		case '7':
			if (p_version[2] == '1') {
				p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_6PLUS;
			} else {
				p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_6;
			}
			break;
		case '8':
			p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_6PLUS;
			break;
		default:
			p_cb_screen_data->iphone_version = CB_SCREEN_IPHONE_VERSION_UNKNOW;
			break;
		}
	} else {
		p_cb_screen_data->phone_os_type = CB_SCREEN_OS_TYPE_ANDROID;
	}

	return p_cb_screen_data->phone_os_type;
}

UINT8 hlp_cb_screen_get_os_type()
{
	return p_cb_screen_data->phone_os_type;
}

UINT8 hlp_cb_screen_set_os_version(UINT8 *p_version)
{
	if (p_cb_screen_data->phone_os_type == CB_SCREEN_OS_TYPE_ANDROID) {
		ratio_x = ratio_y = 2;
		return p_cb_screen_data->os_version;
	}

	if (((strncmp(p_version, "IOS", 3) == 0) && (strncmp(p_version+3, "8.4", 3) < 0))
	    || ((strncmp(p_version, "12", 2) == 0) && (strncmp(p_version+2, "H143", 4) < 0))) {

		p_cb_screen_data->os_version = CB_SCREEN_OS_VERSION_8P4M;
	} else {
		p_cb_screen_data->os_version = CB_SCREEN_OS_VERSION_8P4S;
	}

	return p_cb_screen_data->os_version;
}

UINT8 hlp_cb_screen_get_os_version(void)
{
	return p_cb_screen_data->os_version;
}

void hlp_cb_screen_get_iphone_pixel(UINT16 *width, UINT16 *height)
{
	*width = g_iphone_display_pixel[p_cb_screen_data->iphone_version][0];
	*height = g_iphone_display_pixel[p_cb_screen_data->iphone_version][1];
}

void hlp_cb_screen_iap_event_cbk(UINT8 event, void *param)
{
	bse_log_api(TAG, "hlp_cb_screen_iap_event_cbk, event=%d, param=%08x\r\n", event, param);

	switch (event) {
	case EV_IAP_STARTED:
		hlp_cb_screen_set_status(ST_CB_SCREEN_SUCCESS);
		hlp_cb_screen_set_local_info(g_iphone_display_pixel[p_cb_screen_data->iphone_version][1], g_iphone_display_pixel[p_cb_screen_data->iphone_version][0]);
		p_cb_screen_data->event_cbk(EV_BTC_CB_SUCCESS, NULL);
		break;
	case EV_IAP_STOPPED:
		hlp_cb_screen_set_status(ST_CB_SCREEN_READY);
		p_cb_screen_data->event_cbk(EV_BTC_CB_STOPPED, NULL);
		break;
	default:
		break;
	}
}
#endif
#endif

