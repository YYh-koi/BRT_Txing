/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_a2dp.c
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _APP_A2DP_H_
#define _APP_A2DP_H_

#include "bt_event.h"
#include "internal_a2dp.h"
#include "profile.h"

/* State */
enum {
    ST_A2DP_UNINIT,
    ST_A2DP_INIT,
    ST_A2DP_READY,
    ST_A2DP_CONNECTING,
    ST_A2DP_CONNECTED,
    ST_A2DP_PLAYING,
    ST_A2DP_STREAMING,
    ST_A2DP_END
};

/* Event */
enum {
    EV_A2DP_AV_OPEN,
    EV_A2DP_AV_PLAYING,
    EV_A2DP_AV_SUSPEND,
    EV_A2DP_DISCONNECTED,
    EV_A2DP_RETURN_STATUS,
    EV_A2DP_AV_SET_CONFIG,
    EV_A2DP_AV_DATA_IND,
    EV_A2DP_APTX_STATUS,
    EV_A2DP_PROFILESTAT,
    EV_A2DP_SET_MUTE,
    EV_A2DP_SET_SAMP,
    EV_A2DP_GET_RDEVICE_INFO,
    EV_A2DP_PLAY_STAT,
    EV_A2DP_OK,
};

typedef struct tag_a2dp_status {
	UINT8 status;
	UINT8 bd[BD_ADDR_LEN];
}a2dp_status_t;
extern profile_t g_a2dp_profile;

void app_a2dp_init(module_event_cbk_t event_cbk);
void app_a2dp_server_start(void);
void app_a2dp_server_stop(void);
void app_a2dp_done(void);
UINT8 app_a2dp_connect(UINT8 *bd_addr);
UINT8 app_a2dp_disconnect(void);
UINT8 app_a2dp_get_status(void);
void app_a2dp_query_status(void);
#ifdef CONFIG_BIND_PROFILES
void app_a2dp_set_deny(UINT8 *bd_addr);
void app_a2dp_cannel_deny();
#endif
#endif


