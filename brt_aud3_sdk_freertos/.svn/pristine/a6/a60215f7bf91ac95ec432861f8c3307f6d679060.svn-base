/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_hf.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HF_FI_H
#define _HF_FI_H

#include "global.h"
#include "internal_hf.h"
#include "profile.h"


/* State */
enum {
    ST_HF_UNINIT,
    ST_HF_INIT,
    ST_HF_READY,
    ST_HF_DISCONNECTING,
    ST_HF_CONNECTING,
    ST_HF_CONNECTED,
    ST_HF_OUTGOING,
    ST_HF_INCOMING,
    ST_HF_ONGOING,
    ST_HF_HELD,
    ST_HF_HUNGUPING,
    ST_HF_END,
};

/* Event */
enum {
    EV_HF_ENTER_PAIRING_MODE,
    EV_HF_EXIT_PAIRING_MODE,
    EV_HF_EXIT_SCAN_MODE,
    EV_HF_CONNECTING,
    EV_HF_DISCONNECTED,
    EV_HF_CONNECTED,//5
    EV_HF_CONN_CFM,
    EV_HF_OUTGOING,
    EV_HF_INCOMMINGCALL,
    EV_HF_STANDBY,
    EV_HF_ONGOING,//10
    EV_HF_AG_DEVICE_NAME,
    EV_HF_AG_DEVICE_VENDOR_NAME,
    EV_HF_AG_DEVICE_MODEL,
    EV_HF_AG_OS_VERSION_IND,
    EV_HF_SCO_CONNECTED,//15
    EV_HF_SCO_DISCONNECTED,
    EV_HF_RETURN_STATUS,
    EV_HF_CONNECT_FAIL,
    EV_HF_RINGTONE,
    EV_HF_NETWORK_OPERATOR_IND,//20
    EV_HF_SIGNAL_STRENGTH_IND,
    EV_HF_BATTERY_CHARGE_IND,
    EV_HF_SUBSCRIBER_NUMBER_IND,
    EV_HF_3_WAY_CALL_NEW,
    EV_HF_3_WAY_CALL_STATE_IND,//25
    EV_HF_3_WAY_CALL_DONE,
    EV_HF_MIC_GAIN,
    EV_HF_RINGTONE_TERMINATED,
    EV_HF_ROAMING_STATE_IND,
    EV_HF_NETWORK_STATE_IND,//30
    EV_HF_VOICE_RECOGN_START,
    EV_HF_VOICE_RECOGN_STOP,
    EV_HF_EXIT_CFG,
    EV_HF_DEV_INFO,
    EV_HF_RINGTONE_ON,
    EV_HF_RINGTONE_OFF,
    EV_HF_STAT,
    EV_HF_OK,//38
};

enum {
    HF_RING_TONE_TYPE_OUT_BAND,
    HF_RING_TONE_TYPE_IN_BAND,
};

struct connected_info_t {
	UINT8 bd[BD_ADDR_LEN];
	BOOL  outgoing;
};

struct app_hf_ring_param_stru {
	UINT8 ring_tone_type;
	UINT8 ring_count;
};

typedef struct tag_hf_status {
	UINT8 status;
	UINT8 bd[BD_ADDR_LEN];
}hf_status_t;

extern profile_t g_hf_profile;

void app_hf_init(module_event_cbk_t p_event_cbk);
void app_hf_server_start(void);
void app_hf_server_stop(void);
void app_hf_done(void);
void app_get_config_hfp(void);
UINT8 app_hf_connect(UINT8* bd_addr);
UINT8 app_hf_disconnect(void);
void app_hf_dial(UINT8 *number);
void app_hf_send_dtmf(UINT8 *dtmf);
void app_hf_answer_call();
void app_hf_reject_call();
void app_hf_redial();
void app_hf_audio_transfer();
void app_hf_hangup();
void app_get_services(UINT8 *bd);
void app_hf_voice_recognition_req(UINT8 ctl);
void app_hf_query_status(void);
void app_hf_3_way_control(UINT8 idx, UINT8 ctl);

#endif

