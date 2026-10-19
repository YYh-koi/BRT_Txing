/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		bt_manager.h
*
*	Description:
*
*
*	Create Date:	2014/06/19
***********************************************************************/

#ifndef _BT_MANAGER_H_
#define _BT_MANAGER_H_

#include "btdatatype.h"
#include "bt_event.h"
#ifdef CONFIG_HID
#include "internal_hid.h"
#endif
#ifdef CONFIG_DUN
#include "internal_dun.h"
#endif
#ifdef CONFIG_PAN
#include "internal_pan.h"
#endif

enum {
    ST_IAP_INIT,
    ST_IAP_READY,
    ST_IAP_CONNECTING,
    ST_IAP_CONNECTED,
    ST_IAP_END,
};

/* control type for auto control */
#define APP_AUTO_CONTROL_TYPE_CLR		0x00
#define APP_AUTO_CONTROL_TYPE_SET		0x01
/* mask for auto control */
#define APP_AUTO_CONTROL_FLAG_CONNECT	0x01
#define APP_AUTO_CONTROL_FLAG_ANSWER	0x02

#define BIND_PROFILE_HFP_A2DP	0x01
#define BIND_PROFILE_A2DP_AVRCP 0x02
#define BIND_PROFILE_HFP_HID	0x04
#define BIND_PROFILE_HFP_PBAP	0x08

#define MAX_DEVICE_MODEL_LEN 40

#define RECONNECT_PROFILE_HFP             0X0001

#define PROFILE_BT_SPP 				(1 << 0 & 0xffff)
#define PROFILE_BT_GATT_SERVER 		(1 << 1 & 0xffff)
#define PROFILE_BT_GATT_CLIENT 		(1 << 2 & 0xffff)
#define PROFILE_BT_HFP 				(1 << 3 & 0xffff)
#define PROFILE_BT_AG 				(1 << 4 & 0xffff)
#define PROFILE_BT_A2DP_SINK 		(1 << 5 & 0xffff)
#define PROFILE_BT_A2DP_SOURCE 		(1 << 6 & 0xffff)
#define PROFILE_BT_AVRCP_CT 		(1 << 7 & 0xffff)
#define PROFILE_BT_AVRCP_TG 		(1 << 8 & 0xffff)
#define PROFILE_BT_HID_KEYBOARD 	(1 << 9 & 0xffff)
#define PROFILE_BT_PBAP_SERVER 		(1 << 10 & 0xffff)
#define PROFILE_BT_AAP				(1 << 14 & 0xffff)
#define PROFILE_BT_IAP2 			(1 << 15 & 0xffff)

#define PROFILE_AVRCP_AUTO_GET_TRACK 1
#define PROFILE_AVRCP_PLAY_PROGRESS  14
#define PROFILE_AVRCP_BROWSING  	 16
#define PROFILE_AVRCP_MEDIA_IMAGE  	 32

struct bt_manager_stru {
	UINT16 binded_profiles;
#ifdef CONFIG_MGR_STATIC
	UINT16 disabled_service;
#endif
#ifdef CONFIG_MGR_DYNAMIC
	int local_profile_flag;
	int local_profile_status;
#endif
	UINT16 conn_hdl;
	UINT8 state;
	/* Default value is 0
	   bit 0: mean whether enable or disable auto connect;
	   bit 1: mean whether enable or disable auto answer incoming call.
	 */
	char dev_model[MAX_DEVICE_MODEL_LEN];
	UINT16 reconn_state;
	UINT8 auto_ctl_flag;
	module_event_cbk_t event_cbk;
	struct FsmTimer *timer_w4_avrcp_disc;
};

void bt_mgr_server_init(module_event_cbk_t event_cbk);
void bt_mgr_server_start(void);
void bt_mgr_server_stop(void);
void bt_mgr_server_done(void);
UINT16 bt_manager_get_conn_hdl(void);
void bt_manager_set_auto_ctl(UINT8 type, UINT8 mask);
BOOL bt_manager_hfp_a2dp_is_binded();
BOOL bt_manager_a2dp_avrcp_is_binded();
void bt_manager_profile_operation(int profile_id, int param);
struct FsmTimer *bt_manager_get_timer_notify_status(); 
void bt_manager_discnnect_all_profile();
void bt_manager_bind_hfp_a2dp(BOOL bind);
void bt_manager_bind_a2dp_avrcp(BOOL bind);
int bt_manager_set_curr_profile(UINT16 value);
int bt_manager_get_binded_profiles();
int bt_manager_get_binded_profiles_value();
int bt_manager_get_profiles_value_a2dp();

#endif	/* _BT_MANAGER_H_ */

