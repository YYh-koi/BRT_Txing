/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_a2dp.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HLP_A2DP_H_
#define _HLP_A2DP_H_

#include "global.h"
#include "a2dp_ui.h"
#include "bt_event.h"

struct A2DPUserDataStru {
	UINT8 remote_bd[BD_ADDR_LEN];
	UINT16 cid;
	UINT8 seid;
	void  *stream_handle;
	HANDLE sbc_handle;
	UINT8 state;
	UINT32 svr_hdl;

	module_event_cbk_t event_cbk;
};

#define A2DP_USER_DATA			(s_a2dp_user_data)
#define A2DP_STATE				(s_a2dp_user_data->state)

void hlp_a2dp_init(module_event_cbk_t event_cbk);
void hlp_a2dp_server_start(void);
void hlp_a2dp_server_stop(void);
void hlp_a2dp_done(void);
void hlp_a2dp_register_sep(void);
void hlp_a2dp_register_cbk(void);
void hlp_a2dp_stream_open(void *param);
void hlp_a2dp_stream_start(void);
void hlp_a2dp_stream_suspend(void);
void hlp_a2dp_stream_create(void *param);
void hlp_a2dp_stream_done(void *param);
void hlp_a2dp_cbk(UINT8 event, void *param);
void  *hlp_a2dp_get_handle(void);
void hlp_a2dp_set_status(unsigned char iStatus);
UINT8 hlp_a2dp_get_status(void);
void hlp_a2dp_quiry_status(void);
void hlp_a2dp_event_OK(void);
void hlp_a2dp_event_mute(int value);
void hlp_a2dp_query_status(void);
void hlp_a2dp_unregister_sdp();
void hlp_a2dp_register_sdp();
#endif

