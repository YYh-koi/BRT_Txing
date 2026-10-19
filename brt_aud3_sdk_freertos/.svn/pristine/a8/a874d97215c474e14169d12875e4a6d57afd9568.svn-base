/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_avrcp.h
*
*	Description:	This module defines all of the AVRCP 1.4 advanced
					control application interface functions.
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#ifndef _APP_AVRCP_H_
#define _APP_AVRCP_H_

#include "btdatatype.h"
#include "bt_event.h"
#include "internal_avrcp.h"
#include "profile.h"

/* State */
enum {
    ST_APP_AVRCP_UNINIT,
    ST_APP_AVRCP_INIT,
    ST_APP_AVRCP_READY,
    ST_APP_AVRCP_CONNECTING,
    ST_APP_AVRCP_CONNECTED,
    ST_APP_AVRCP_STOPPED,
    ST_APP_AVRCP_PLAYING,
    ST_APP_AVRCP_PAUSED,
    ST_APP_AVRCP_END
};

/* Event */
typedef enum {
    EV_AVRCP_CONN_COMPLETED,							//struct HLPAVRCPConnResultStru
    EV_AVRCP_DISC_COMPLETED,
    EV_AVRCP_ELEMENT_ATTRS,								//char *str
    EV_AVRCP_FOLDER_LIST_INFO,
    EV_AVRCP_NOW_PLAY_STATUS_IND,
    EV_AVRCP_ELEMENT_END,
    EV_AVRCP_UPDATA_UIDS,								//UINT16
    EV_AVRCP_ELEMENT_ATTRS_OF_NOW_PLAYING,				//char *str
    EV_AVRCP_PLAYBACK_STATUS_CHANGED,					//BOOL
    EV_AVRCP_PLAYBACK_POS_CHANGED,						//UINT32 pos
    EV_AVRCP_NOW_PLAYING_CONTENT_CHANGED,
    EV_AVRCP_BROWSING_CHANEL_COMPLETED,					//BOOL
    EV_AVRCP_RETURN_STATUS,
    EV_AVRCP_GET_CAPS_CMF,
    EV_AVRCP_STAT,
    EV_AVRCP_PLAY_STAT,
    EV_AVRCP_ABS_VOL_IND,

    EV_AVRCP_VENDOR_ERR_RESP_IND,
    EV_AVRCP_AUDIO_PLAY_STATUS_IND,
    EV_AVRCP_PLAYER_SETTTING_STATUS_IND,
    EV_AVRCP_MUSIC_INFO_IND,
    EV_AVRCP_PLAYER_INFO_IND,
    EV_AVRCP_FOLDER_INFO_IND,
    EV_AVRCP_MEDIA_INFO_IND,
    EV_AVRCP_ADD_TO_NOW_PLAYING_RESULT_IND,
    EV_AVRCP_SET_ADDRESSED_PLAYER_RESULT_IND,
    EV_AVRCP_SET_BROWSED_PLAYER_RESULT_IND,
    EV_AVRCP_PLAY_MEDIA_RESULT_IND,
    EV_AVRCP_SET_ABSOLUTE_VOLUME_RESULT_IND,

    EV_AVRCP_ADDRESSED_PLAYER_CHANGED_IND,
    EV_AVRCP_BROWSING_CHANNEL_CONNECTED,

    //bip
    EV_AVRCP_BIP_STATUS,
    EV_AVRCP_BIP_COVERART_IMAGE,
} event_avrcp;

#define AVRCP_OPID_VOLUME_UP 										0x41
#define AVRCP_OPID_VOLUME_DOWN 										0x42
#define AVRCP_CMD_OPID_PLAY 										0x44
#define AVRCP_CMD_OPID_STOP 										0x45
#define AVRCP_CMD_OPID_PAUSE 										0x46
#define AVRCP_CMD_OPID_RECORD 										0x47
#define AVRCP_CMD_OPID_REWIND 										0x48
#define AVRCP_CMD_OPID_FAST_FORWARD 								0x49
#define AVRCP_CMD_OPID_EJECT 										0x4a
#define AVRCP_CMD_OPID_FORWARD 										0x4b
#define AVRCP_CMD_OPID_BACKWARD 									0x4c

#define AVRCP_UID_LEN           8
#define AVRCP_TIME_LEN          16
#define AVRCP_FOLDER_LEN        128

struct AVRCPDataIndStru {
	UINT8 uid[AVRCP_UID_LEN];
	UINT8 time[AVRCP_TIME_LEN];
	UINT8 *title;
	UINT8 *artist;
	UINT8 *album;
	UINT8 *img_handle;
};

struct AVRCPFolderInfoIndStru {
	UINT8 uid[AVRCP_UID_LEN];
	UINT8 name[AVRCP_FOLDER_LEN];
};

struct AVRCPNowPlayStatusIndStru {
	UINT32 	song_length;
	UINT32 	song_position;
	UINT8 	play_status;
};

typedef struct tag_avrcp_status {
	UINT8 status;
	UINT8 bd[BD_ADDR_LEN];
} avrcp_status_t;

extern profile_t g_avrcp_profile;

void app_avrcp_init(module_event_cbk_t event_cbk);
void app_avrcp_server_start(void);
void app_avrcp_server_stop(void);
void app_avrcp_done(void);
void app_avrcp_connect(UINT8 *bd_addr);
void app_avrcp_send_command(UINT8 op_id);
void app_avrcp_get_nowplaying_info(void);
void app_avrcp_get_nowplaying_list(UINT32 offset, UINT32 count);
void app_avrcp_get_play_status(void);
void app_avrcp_get_folder_items(UINT8 scope, UINT32 start_item, UINT32 end_item);
void app_avrcp_get_folder_list(UINT32 start_item, UINT32 end_item);
void app_avrcp_set_player_app(UINT32 eID, UINT32 eValue);
void app_avrcp_change_path(UINT16 uid_counter, UINT8 direction, UINT8 *folder_uid);
void app_avrcp_play_item(UINT8 *uid);
void app_avrcp_disconnect(void);
UINT8 app_avrcp_get_status(void);
void app_avrcp_quiry_status(void);
void app_avrcp_playpause(void);
void app_avrcp_send_command_ex(UINT8 op_id, UINT8 state);
UINT8 app_get_avrcp_capability(void);
UINT8 app_get_avrcp_bip_art();
#ifdef CONFIG_ABS_VOL
void app_avrcp_volume_up();
void app_avrcp_volume_down();
void app_send_abs_vol(UINT8 *bd, UINT8 rsp_type, int abs_vol);
#endif
void app_avrcp_set_config(UINT8 param);
void app_avrcp_get_config();
#endif

