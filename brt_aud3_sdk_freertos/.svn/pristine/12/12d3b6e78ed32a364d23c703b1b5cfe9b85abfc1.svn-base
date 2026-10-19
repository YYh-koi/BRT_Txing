/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_gap.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _APP_GAP_H_
#define _APP_GAP_H_

#include "gap_ui.h"
#include "bt_event.h"
#include "internal_gap.h"
#include "profile.h"

#define GAP_REQ_DATA_BUFF_LENGTH	32

/* State */
enum {
    ST_GAP_UNINIT,
    ST_GAP_INIT,
    ST_GAP_READY,
    ST_GAP_RPOWER_DOWN,
    ST_GAP_RPOWER_ON,
    ST_GAP_WORKING,
    ST_GAP_SLEEP,
};

/* Event */
enum {
    EV_APP_GAP_STARTED,
    EV_APP_GAP_SHUTTING_DOWN,
    EV_APP_GAP_SHUTTED_DOWN,

    EV_APP_GAP_RET_HARDWARE_DATA,
    EV_APP_GAP_RET_SCAN_MODE,
    EV_APP_GAP_RET_LOCAL_BD,
    EV_APP_GAP_RET_MIC_GAIN,
    EV_APP_GAP_RET_LINK_KEY,
    EV_APP_GAP_RET_AG_DEVICE_NAME,

    EV_APP_GAP_REQ_LOCAL_DEVICE_NAME,
    EV_APP_GAP_REQ_LOCAL_DEVICE_CLASS,
    EV_APP_GAP_REQ_LINK_KEY,
    EV_APP_GAP_REQ_PINCODE,

    EV_APP_GAP_DISCOVERY_STARTED,
    EV_APP_GAP_DISCOVERY_DEVICE_FOUND,
    EV_APP_GAP_DISCOVERY_DEVICE_UPDATED,
    EV_APP_GAP_DISCOVERY_FINISHED,
    EV_APP_GAP_AUTHENTICATION_FAILURE,
    EV_APP_GAP_PIN_OR_KEY_MISSING,
    EV_APP_GAP_LINK_LOST,
    EV_APP_GAP_ACL_DISCONNECTED,

    EV_APP_GAP_BOND_STATE_CHANGED,
    EV_APP_GAP_CONNECTION_DISCONNECTED,
    EV_APP_GAP_PASSKEY_NOTIF,
    EV_APP_GAP_PASSKEY_REQ,
    EV_APP_GAP_PINCODE_REQ,
    EV_APP_GAP_USER_CONFIRMATION,
    EV_APP_GAP_SERVICES_IND,
    EV_APP_GAP_HARDWARE_ERROR,
    EV_APP_GAP_AUTHORIZE_REQUEST,
    EV_APP_GAP_REMOVE_PLIST,
    EV_APP_GAP_OK,
};

struct gap_req_data_stru {
	UINT8 *p_reference;

	UINT8 *p_ret_data;
};

enum {
    BT_SET_SCAN_MODE_TYPE_SCAN = 0x00,		//Fix
    BT_SET_SCAN_MODE_TYPE_PAIR = 0x01,	//Fix

    BT_SET_SCAN_MODE_TYPE_DOUBLE,

    BT_SET_SCAN_MODE_TYPE_RECOVER,
};

extern profile_t g_gap_profile;

void app_gap_init(module_event_cbk_t event_cbk);
void app_gap_done(void);
void app_gap_server_start(void);
void app_gap_init_transport_layer(HANDLE tl_inst, TransportLayerStru *tl_funcs);
void app_gap_destory_transport_layer(HANDLE tl_hdl, GAP_UnregisterTLCfmCbk *cfm_cbk);
void app_gap_reset_hardware();
void app_gap_set_bondable(UINT8 bondable);
void app_gap_set_local_device_class(UINT8 cod[CLS_DEV_LEN]);
void app_gap_set_local_name(UINT8 *name, UINT16 len);
void app_gap_vendor_command(UINT8 flag, UINT8 ocf, void *param, UINT8 param_len);
void app_gap_set_ssp_mode(UINT8 enable);
void app_gap_scan_mode_control(UINT8 type, BOOL ctl);
void app_gap_get_connect_handle(UINT8 *bd, UINT16 *conn_hdl);
//void app_gap_start_discovery();
void app_gap_start_discovery(int time_len, char type);
void app_gap_cancel_discovery();
UINT8 app_gap_is_discovering();
void app_gap_add_eir(UINT8 *p_uuid, UINT8 len, UINT8 type);
UINT8* app_get_device_cod();


enum discovery_event_type {
    BT_DISCOVERY_EVENT_STARTED,
    BT_DISCOVERY_EVENT_DEVICE_FOUND,
    BT_DISCOVERY_EVENT_FINISHED,
};

typedef void (*bt_discovery_callback)(UINT8 event, void *param);

typedef struct tag_bonded_device {
	UINT8 bd[BD_ADDR_LEN];
	UINT8 name[BD_NAME_LEN];
} bt_bonded_device_t;

void app_gap_create_bond(UINT8 *bd_addr);
void app_gap_remove_bond(UINT8 *bd_addr);
void app_gap_pincode_reply(UINT8 *bd_addr, UINT8 accept, UINT8 pin_len, UINT8 *pincode);
void app_gap_passkey_reply(UINT8 *bd_addr, UINT8 accept, UINT32 passkey);
void app_gap_user_confirmation(UINT8 *bd_addr, UINT8 accept);
void app_gap_add_carplay_uuid(void);
void app_set_ssp_mode(int mode);
void app_set_usr_pair_mode(int usr_value);
void app_set_ssp_cap_mode(int usr_value);
void app_set_io_cap_mode(int usr_value);
void app_gap_get_scan_mode(void);
UINT8 app_gap_get_scan_mode_ex(void);
void app_gap_set_scan_mode(UINT8 scan_mode);
void app_gap_get_remote_device_name(UINT8 *bd_addr);
#endif /* _APP_GAP_H_ */

