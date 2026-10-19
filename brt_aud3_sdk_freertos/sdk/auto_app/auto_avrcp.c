/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_avrcp.c
*
*	Description:	This module defines all of the AVRCP 1.4 advanced
					control application interface functions.
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "auto_avrcp.h"
#include "internal_avrcp.h"
#include "avrcp_ui.h"
#include "brt_trace.h"
#include "bta_bip.h"
#include "brlinkd.h"
#include "bt_manager.h"
#include "auto_at_internal.h"
#define TAG "bse_avrcp"

void app_avrcp_init(module_event_cbk_t event_cbk)
{
	if (hlp_avrcp_get_status() != ST_APP_AVRCP_UNINIT) {
		return;
	}

	hlp_avrcp_init(event_cbk);
}

UINT8 app_avrcp_get_status(void)
{
	return hlp_avrcp_get_status();
}

void app_avrcp_quiry_status(void)
{
	hlp_avrcp_quiry_status();
}

void app_avrcp_server_start(void)
{
	if (hlp_avrcp_get_status() != ST_APP_AVRCP_INIT) {
		return;
	}

	hlp_avrcp_server_start();
}

void app_avrcp_server_stop(void)
{
	if (hlp_avrcp_get_status() < ST_APP_AVRCP_READY) {
		return;
	}

	hlp_avrcp_server_stop();
}

void app_avrcp_done(void)
{
	if (hlp_avrcp_get_status() == ST_APP_AVRCP_UNINIT) {
		return;
	} else {
		app_avrcp_server_stop();
	}

	hlp_avrcp_done();
}

void app_avrcp_set_config(UINT8 param)
{
	hlp_avrcp_set_config(param);
}

void app_avrcp_get_config()
{
	UINT8 ret = hlp_avrcp_get_config();
	bt_event_indicate(BT_EV_AVRCP_GET_PROFILE_CFG, (void *)ret);
}


void app_avrcp_connect(UINT8 *bd_addr)
{
	if (hlp_avrcp_get_status() == ST_APP_AVRCP_READY) {
		hlp_avrcp_connect(bd_addr);
	}
}

void app_avrcp_disconnect(void)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_disconnect(hlp_avrcp_get_connected_bd());
	}
}

void app_avrcp_send_command(UINT8 op_id)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_passthrough_req(hlp_avrcp_get_connected_bd(), op_id, 0);
	}
}
void app_avrcp_send_command_ex(UINT8 op_id, UINT8 state)
{
    if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED)
    {
        hlp_avrcp_passthrough_req_ex(hlp_avrcp_get_connected_bd(), op_id, 0, state);
    }
}

void app_avrcp_playpause()
{
	UINT8 curr_avstat =  hlp_avrcp_get_status();
	if (curr_avstat >= ST_APP_AVRCP_CONNECTED)
	{
		if(curr_avstat == ST_APP_AVRCP_PLAYING)
		{
			app_avrcp_send_command(AVRCP_CMD_OPID_PAUSE);
		}else{
			app_avrcp_send_command(AVRCP_CMD_OPID_PLAY);
		}
	}
}

#ifdef CONFIG_ABS_VOL
void app_avrcp_volume_up()
{
	UINT8 curr_avstat =  hlp_avrcp_get_status();
	if (curr_avstat >= ST_APP_AVRCP_CONNECTED)
	{
		app_avrcp_send_command(AVRCP_OPID_VOLUME_UP);
	}
}

void app_avrcp_volume_down()
{
	UINT8 curr_avstat =  hlp_avrcp_get_status();
	if (curr_avstat >= ST_APP_AVRCP_CONNECTED)
	{
		app_avrcp_send_command(AVRCP_OPID_VOLUME_DOWN);
	}
}
#endif

void app_avrcp_get_nowplaying_info(void)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_get_element_attr_req(hlp_avrcp_get_connected_bd(), 1);
	}
}

void app_avrcp_get_nowplaying_list(UINT32 offset, UINT32 count)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_get_nowplaying_req(hlp_avrcp_get_connected_bd()
		                             , offset
		                             , (offset + count));
	}
}

void app_avrcp_play_item(UINT8 *uid)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_play_item_req(hlp_avrcp_get_connected_bd()
		                        , uid
		                        , hlp_avrcp_get_uid_counter()
		                        , AVRCP_SCOPE_MEDIAPLAYER_VIRTUAL_FILESYSTEM);
	}
}

void app_avrcp_get_play_status()
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_get_play_status(hlp_avrcp_get_connected_bd());
	}
}

void app_avrcp_set_player_app(UINT32 eID, UINT32 eValue)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_set_player_app_setting_value_req(hlp_avrcp_get_connected_bd(), eID, eValue);
	}
}

void app_avrcp_get_folder_list(UINT32 start_item, UINT32 end_item)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_get_folder_list(hlp_avrcp_get_connected_bd(), start_item, end_item);
	}
}

void app_avrcp_get_folder_items(UINT8 scope, UINT32 start_item, UINT32 end_item)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_get_folder_items(hlp_avrcp_get_connected_bd(), scope, start_item, end_item);
	}
}

void app_avrcp_change_path(UINT16 uid_counter, UINT8 direction, UINT8 *folder_uid)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		hlp_avrcp_change_path(hlp_avrcp_get_connected_bd(), uid_counter, direction, folder_uid);
	}
}

UINT8 app_get_avrcp_capability(void)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		return hlp_avrcp_get_capability();
	}
	return -1;
}

extern bip_user_date_t s_bip_user_data;
UINT8 app_get_avrcp_bip_art()
{
	if(hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED && s_bip_user_data.bip_connected && (sw_get_avrcp_bip_permission() || (hlp_avrcp_get_config() & PROFILE_AVRCP_MEDIA_IMAGE))) {
		bta_bip_update_cover_art(NULL);
	}
}
void app_send_abs_vol(UINT8 *bd, UINT8 rsp_type, int abs_vol)
{
	hlp_avrcp_send_register_abs_vol_rsp(bd, rsp_type, abs_vol);
}
profile_t g_avrcp_profile = {
	app_avrcp_init,
	app_avrcp_done,
	app_avrcp_server_start,
	app_avrcp_server_stop,
};

