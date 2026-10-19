/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_hf.h
*
*	Description:	Internal Functions For HandFree
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HLP_HF_H_
#define _HLP_HF_H_

#include "btdatatype.h"
#include "bt_event.h"
#include "hfp_ui.h"

#undef CONFIG_HFP_RINGTONE_EVENT

#define HF_NUM_LEN 32

/*0=active, 1=held, 2=dialling(outgoing), 3=alerting(outgoing), 4=incoming(incoming), 5=waiting(incoming)*/
enum {
    HF_3_WAY_CALLS_STATE_ACTIVE,
    HF_3_WAY_CALLS_STATE_HELD,
    HF_3_WAY_CALLS_STATE_DIALING,
    HF_3_WAY_CALLS_STATE_ALERTING,
    HF_3_WAY_CALLS_STATE_INCOMING,
    HF_3_WAY_CALLS_STATE_WAITING
};

/*EV_HF_ROAMING_STATE_IND*/
enum {
    HF_ROAMING_RESET, //roaming is not active
    HF_ROAMING_ACTIVE,//a roaming is active
};

/*EV_HF_NETWORK_STATE_IND */
enum {
    HF_NETWORK_UNAVAILABLE, //cellular network is unavailable
    HF_NETWORK_AVAILABLE,  //cellular network is available
};


/* Guideline P95, AT+CHLD=0, Releases all held calls or sets User Determined User Busy (UDUB) for a waiting call. */
/* Guideline P99, AT+CHLD=1, Releases all active calls (if any exist) and accepts the other (held or waiting) call. */
/* Guideline P103, AT+CHLD=1<idx>, Releases specified active call only (<idx>). */
/* Guideline P105, AT+CHLD=2, Places all active calls (if any exist) on hold and accepts the other (held or waiting) call. */
/* Guideline P107, AT+CHLD=2<idx>, Request private consultation mode with specified call (<idx>). (Place all calls on hold EXCEPT the call indicated by <idx>.) */
/* Guideline P109, AT+CHLD=3, Adds a held call to the conversation */
/* Guideline P111, AT+CHLD=4, Connects the two calls and disconnects the subscriber from both calls (Explicit Call Transfer) */

enum {
    HF_3_WAY_CONTROL_0,
    HF_3_WAY_CONTROL_1,
    HF_3_WAY_CONTROL_2,
    HF_3_WAY_CONTROL_3,
    HF_3_WAY_CONTROL_4
};

enum {
    HF_3_WAY_MARK_TYPE_NONE,
    HF_3_WAY_MARK_TYPE_NEW,
    HF_3_WAY_MARK_TYPE_MARK
};

typedef struct _hf_3_way_call_stru {
	UINT8 index;
	UINT8 number[HF_NUM_LEN];
	UINT8 state;
	UINT8 mark;
} hf_3_way_call_t;

void hlp_hf_init(module_event_cbk_t p_event_cbk);
void hlp_hf_server_start(void);
void hlp_hf_server_stop(void);
void hlp_hf_done(void);
void hlp_hf_dial_ind(UCHAR event, void *arg);
void hlp_hf_dial_num_ind(UCHAR event, void *arg);
void hlp_hf_ring_ind(void *arg);
void hlp_hf_terminate_local_ringtone_ind(void);
void hlp_hf_current_calls_ind(void *arg);
void hlp_hf_standby(void *arg);
void hlp_hf_audio_established(UINT16 handle, unsigned char is_wbs);
void hlp_hf_audio_release(void);
void hlp_hf_slc_release(struct HFP_SLCConnectInfoStru *info);
void hlp_get_config_hfp(void);
void hlp_hf_connect(UINT8* bd_addr);
void hlp_hf_disconnect(void *hdl);
void hlp_hf_audio_transfer(void);
void hlp_hf_auto_connect(void *arg);
void hlp_hf_get_current_calls(void);
void hlp_hf_extend_cmd(void *cmd, UINT16 len, UINT32 timeout);
void hlp_hf_get_phone_manu_and_model();
void hlp_hf_btrh_operation(CHAR *chr);
void hlp_hf_3way_calling_handler(CHAR *chr);
void hlp_hf_svr_stop(CHAR *chr);
void hlp_hf_set_status(unsigned char iStatus);
UINT8 hlp_hf_get_status(void);
void hlp_hf_set_waiting(UINT8 status);
void hlp_hf_connecting(UINT8 *bd);
void hlp_hf_connect_complete(UINT8 *bd, UINT8 role);
void hlp_hf_disconnect_complete(struct HFP_ConnectEvStru *info);
void hlp_hf_connect_fail(struct HFP_ConnectEvStru *info);
void hlp_hf_slc_established(UINT8 *bd, UINT8 role);
UINT8 hlp_hf_event_cbk(UINT8 *hdl, UCHAR event, UCHAR* param, UINT16 param_len);
UINT8 *hlp_hf_get_handle(void);
UINT8 *hlp_hf_get_hfp_server_handle(void);
UINT8 *hlp_hf_get_hsp_server_handle(void);
void hlp_hf_3_way_control(UINT8 idx, UINT8 ctl);
void hlp_hf_event_OK(void);
UINT16 hlp_hf_get_sco_handle(void);
UINT8 hlp_hf_3_way_call_get_count(void);
int hlp_hf_exist_3rd_call();
void hlp_hf_cancel_active_call();
void hlp_hf_query_status(void);
void event_hf_indication(UINT8 event, void *param);
#endif /*_HLP_HF_H_*/


