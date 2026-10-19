/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_cb_screen.h
*
*	Description:
*
*
*	Create Date:		2014/05/29
***********************************************************************/

#ifndef _APP_CB_SCREEN_H_
#define _APP_CB_SCREEN_H_

#include "global.h"
#include "bt_event.h"
#include "profile.h"

enum {
    EV_BTC_CB_STARTED,
    EV_BTC_CB_STOPPED,
    EV_BTC_CB_SUCCESS,

    //For Android
    EV_BTC_CB_SPP_CONNECT_REQ,
    EV_BTC_CB_SPP_DISCONNECT_REQ,
};

//display status
enum {
    CB_SCREEN_DISPLAY_STATUS_H,
    CB_SCREEN_DISPLAY_STATUS_V
};

//stop operator flag
enum {
    CB_SCREEN_STOP_FLAG_STOP_ONLY,
    CB_SCREEN_STOP_FLAG_DISCONNECT
};

//the notification list for calibrate screen
enum {
    NF_CB_SCREEN_SPP_CONNECT,
    NF_CB_SCREEN_SPP_DISCONNECT,
    NF_CB_SCREEN_SPP_DATA_IND,
    NF_CB_SCREEN_HID_CONNECT,
    NF_CB_SCREEN_HID_DISCONNECT,
    NF_CB_SCREEN_END
};

typedef void (*cb_screen_spp_connect)(UINT8 *bd);
typedef void (*cb_screen_spp_disconnect)(void);
typedef void (*cb_screen_spp_data_ind)(UINT8 *p_data, UINT32 len);
typedef void (*cb_screen_hid_send_coordinate)(UINT8 button, INT16 x, INT16 y);

typedef void (*cb_screen_event_cbk)(UINT32 event, void *param);

struct user_cb_screen_func {
	cb_screen_spp_connect spp_connect;
	cb_screen_spp_disconnect spp_disconnect;
	cb_screen_spp_data_ind spp_data_ind;
	cb_screen_hid_send_coordinate hid_send_coordinate;
	cb_screen_event_cbk event_cbk;
};

struct user_cb_screen_data_stru {
	UINT16 display_coordinate_h[2][2];
	UINT16 display_coordinate_v[2][2];
};

extern profile_t g_cbp_profile;

void app_cb_screen_init(module_event_cbk_t event_cbk);
void app_cb_screen_start(void);
void app_cb_screen_connect(UINT8 *bd, UINT16 resolution_w, UINT16 resolution_h);
void app_cb_screen_disconnect(UINT8 flag);
void app_cb_screen_stop(void);
void app_cb_screen_done();
void app_cb_screen_coordinate_data(UINT8 button, INT16 x, INT16 y);
void app_cb_screen_send_key(UINT8 key_value);
UINT8 app_cb_screen_notify_ind(UINT8 event, void *param);
void app_cb_screen_set_iphone_display_status(UINT8 display_status);
#ifdef CONFIG_IAP2
void app_cb_screen_os_type(UINT8 *p_os, UINT8 *bd);
void app_cb_screen_os_version(UINT8 *p_version);
void app_cb_screen_set_iphone_display_status(UINT8 display_status);
#endif
#endif /*_APP_CB_SCREEN_H_*/

