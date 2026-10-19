/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_avrcp.h
*
*	Description:	This module defines all of the AVRCP 1.4 advanced
					control application interface functions.
*
*	Autor:			Lu Dongmei
					Wang Yao
					HanBiao
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _HLP_AVRCP_H_
#define _HLP_AVRCP_H_

#include "avrcp_ui.h"
#include "bt_event.h"


struct HLPAVRCPConnResultStru {
	UINT8 	bd_addr[BD_ADDR_LEN];
	BOOL	result;
};

enum
{	
	AVRCP_CMD_PASA_ID_ILLEGAL,
	AVRCP_CMD_PASA_ID_EQUALIZER,
	AVRCP_CMD_PASA_ID_REPEAT,
	AVRCP_CMD_PASA_ID_SHUFFLE,
	AVRCP_CMD_PASA_ID_SCAN,
	AVRCP_CMD_PASA_ID_UNDEFINED
};

struct APP_AVRCP_ResourceStru {
	UINT16 uid_counter;
	UINT8 remote_bd[BD_ADDR_LEN];
	UINT8 bBrowsingChanel;
	UINT8 capability;

	UINT8 state;
	UINT8 config;

	module_event_cbk_t event_cbk;
};

struct avrcp_get_folder_res_items_type {
	UINT8 item_type;
};

typedef enum {
    BTRC_PLAYSTATE_STOPPED = 0x00,    /* Stopped */
    BTRC_PLAYSTATE_PLAYING = 0x01,    /* Playing */
    BTRC_PLAYSTATE_PAUSED = 0x02,    /* Paused  */
    BTRC_PLAYSTATE_FWD_SEEK = 0x03,    /* Fwd Seek*/
    BTRC_PLAYSTATE_REV_SEEK = 0x04,    /* Rev Seek*/
    BTRC_PLAYSTATE_ERROR = 0xFF,    /* Error   */
} btrc_play_status_t_ex;

typedef void (hlp_avrcp_cbk_func)(UINT8 ev, void *param);/* callback to upper profile */

#define MAX_STR_LEN			127
#define NOTIF_COUNT			6
#define ELEMENT_COUNT		4

#define POS_CHANGED_SUPPORT 0X01
#define TRACK_CHANGED_SUPPORT 0X02

void hlp_avrcp_init(module_event_cbk_t event_cbk);
void hlp_avrcp_server_start(void);
void hlp_avrcp_server_stop(void);
void hlp_avrcp_done(void);
void hlp_avrcp_connect(UINT8 *remote_bd);
void hlp_avrcp_disconnect(UINT8 *remote_bd);
void hlp_avrcp_passthrough_req(UINT8 *remote_bd, UINT8 op_id, UINT8 vendor_id);
void hlp_avrcp_getcaps_req(UINT8 *bd, UINT8 capability_id);
void hlp_avrcp_get_nowplaying_req(UINT8 *bd, UINT32 start_item, UINT32 end_item);
void hlp_avrcp_play_item_req(UINT8 *bd, UINT8 *uid, UINT16 uid_counter, UINT8 scope);
void hlp_avrcp_get_play_status(UINT8 *bd);
void hlp_avrcp_get_folder_items(UINT8 *bd, UINT8 scope, UINT32 start_item, UINT32 end_item);
void hlp_avrcp_get_folder_list(UINT8 *bd, UINT32 start_item, UINT32 end_item);
void hlp_avrcp_set_player_app_setting_value_req(UINT8 *remote_bd, UINT8 attribute_id,UINT8 value);
void hlp_avrcp_change_path(UINT8 *bd, UINT16 uid_counter, UINT8 direction, UINT8 *folder_uid);
void hlp_avrcp_get_folder_items_cfm(UINT8 rsp_type, struct AVRCP_GetFolderItemsRspStru *in);
void hlp_avrcp_get_element_attr_req(UINT8 *bd, UINT8 intentional);
void hlp_avrcp_group_conntion(UINT32 event, UINT8 *param);
void hlp_avrcp_group_command(UINT8 *bd, UINT32 event);
void hlp_avrcp_get_caps_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_GetCapabilitiesRspStru *in);
void hlp_avrcp_get_element_attr_cfm(UINT8 rsp_type, struct AVRCP_Num4IDValueStru *in);
void hlp_avrcp_element_attrs_to_str(UINT8 *str, UINT8 num, struct AVRCP_4IDStringStru *in);
void hlp_avrcp_register_noification_req(UINT8 *bd, UINT8 event_id);
void hlp_avrcp_register_noification_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_RegisterNotifRspStru *in);
#ifdef CONFIG_ABS_VOL
void hlp_avrcp_send_register_abs_vol_rsp(UINT8 *bd, UINT8 rsp_type, int abs_vol);
static void hlp_avrcp_getcaps_ind(UINT8 *bd, UINT32 event, UINT8 cap_id);
static void hlp_avrcp_reg_notification_ind(UINT8 *bd, UINT32 event, struct AVRCP_RegisterNotifRspStru *param);
static void hlp_avrcp_set_abs_vol_ind(UINT8 *bd, UINT32 event, UINT8 volume);
#endif
void hlp_avrcp_passthough_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_PassThroughStru *in);
void hlp_avrcp_group_response(UINT8 *bd, UINT32 event, UINT8 *param);
void hlp_avrcp_cbk(UINT8 *bd, UINT32 event, UINT8 *param, UINT16 len);
void hlp_avrcp_set_status(UINT8 status);
UINT8 hlp_avrcp_get_status(void);
void hlp_avrcp_quiry_status();
UINT8 *hlp_avrcp_get_connected_bd(void);
UINT16 hlp_avrcp_get_uid_counter(void);
struct AVRCPDataIndStru *hlp_avrcp_create_media_info(UINT8 num, struct AVRCP_4IDStringStru *in);
void hlp_avrcp_destroy_media_info(struct AVRCPDataIndStru *p_data);
UINT8 hlp_avrcp_get_capability(void);
void hlp_avrcp_passthrough_req_ex(UINT8 *remote_bd, UINT8 op_id, UINT8 vendor_id, UINT8 state);
void hlp_avrcp_fast_forward_timer(void);
void hlp_avrcp_set_config(UINT8 param);
UINT8 hlp_avrcp_get_config();

#endif
