/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		bt_functions.h
*
*	Description:
*
*
*	Create Date:		2014/08/02
***********************************************************************/

#ifndef _BT_FUNCTIONS_H_
#define _BT_FUNCTIONS_H_

#include "btdatatype.h"
#include "porting.h"

/* HF Error Code */
enum {
    BT_HF_REPLY_SUC,
    BT_HF_REPLY_BD_ERR,
    BT_HF_REPLY_STATUS_ERR,
    BT_HF_REPLY_NOT_SUPPORT,
};
/* A2dp Error Code */
enum {
    BT_A2DP_REPLY_SUC,
    BT_A2DP_REPLY_BD_ERR,
    BT_A2DP_REPLY_STATUS_ERR,
    BT_A2DP_REPLY_NOT_SUPPORT,
};

typedef void (*bt_done_callback)(BOOL result);
/* There Functions Were Used to Support Bluetooth Core Active */
void bt_init();
void bt_done(bt_done_callback callback);
void bt_reset(void);
void bt_runloop_execute(void);
int bt_get_power_state(void);
void bt_action_on_bluetooth_enabled_ex();

/* External Functions For Set And Get Configure */
void bt_dev_get_pairied_device(void);
void bt_dev_remove_device(UINT8 dest_index);
void bt_remove_paired_device(UINT8 *bd_addr);
void bt_dev_operate_local_name(UINT8 *p_name);
void bt_manager_set_local_device_name_ex(UINT8 *p_name);
void bt_manager_set_local_device_pin_ex(UINT8 *p_pincode);
void bt_dev_operate_local_pin(UINT8 *p_pincode);
void bt_dev_set_mic_gain(UINT8 volume);
void bt_dev_set_E2_audio(void);
void bt_vendor_wbs_config(BOOL val);
void bt_set_scan_mode(UINT8 mode);
void bt_get_scan_mode(void);
void bt_dev_get_local_bd(void);
void bt_dev_get_local_flag(void);
void bt_dev_set_local_auto_connect(UINT8 ctl);
void bt_dev_set_local_auto_answer(UINT8 ctl);
void bt_dev_set_local_volume(UINT8 volume);
int bt_dev_get_local_volume(void);
UINT8 bt_dev_get_mic_gain(void);

void bt_start_discovery(int time_len, char type);
void bt_cancel_discovery();
UINT8 bt_is_discovering();
UINT8 bt_get_scan_mode_ex(void);

void bt_get_device_cod();
void bt_get_sdk_prfile();
void bt_set_sdk_prfile(UINT16 value);
void bt_get_sdk_version();
void bt_set_spp_mode(int mode);
void bt_set_usr_pair_mode(int mode);
void bt_set_ssp_cap_mode(int mode);
void bt_set_io_cap_mode(int mode);
void bt_dev_get_local_bd_ex(char *p);

/* The Functions of the Bluetooth Interface */
/* External Functions Interface For HF */
#ifdef CONFIG_HFP
void bt_get_config_hfp(void);
UINT8 bt_hf_connect(UINT8 index, UINT8 *bd);
UINT8 bt_hf_disconnect(void);
UINT8 bt_hf_dial(UINT8 *number);
void bt_hf_set_pair_mode(UINT8 mode);
void bt_hf_answer_call(void);
void bt_hf_deny_call(void);
void bt_hf_hangup(void);
void bt_hf_repeat(void);
void bt_hf_mute_micro_phone(unsigned int flag);
void bt_hf_switch_audio(void);
void bt_hf_set_micmute_stat(int flag);
void bt_hf_speech_recognition(UINT8 ctl);
//void bt_set_scan_mode(UINT8 mode);
void bt_hf_send_dtmf(UINT8 *dtmf);
void bt_hf_query_status(void);
#endif

/* External Functions Interface For A2DP */
#ifdef CONFIG_A2DP
UINT8 bt_a2dp_connect(UINT8 *bd);
void bt_a2dp_connect_last(void);
UINT8 bt_a2dp_disconnect_current(void);
void bt_a2dp_query_status(void);
void bt_a2dp_mute(BOOL ismute);
#ifdef CONFIG_BIND_PROFILES
void bt_a2dp_set_deny(UINT8 *bd_addr);
void bt_a2dp_cannel_deny();
#endif
#endif

/* External Functions Interface For AVRCP */
#ifdef CONFIG_AVRCP
#ifdef CONFIG_BIND_PROFILES
void bt_avrcp_connect(UINT8 *addr);
void bt_avrcp_disconnect();
void bt_avrcp_connect_last(void);
#endif
void bt_avrcp_play_pause(void);
void bt_avrcp_play(void);
void bt_avrcp_pause(void);
void bt_avrcp_stop(void);
void bt_avrcp_next(void);
void bt_avrcp_previous(void);
void bt_avrcp_fast_forward(UINT8 op);
void bt_avrcp_fast_rewind(UINT8 op);
void bt_avrcp_get_nowplaying_info(void);
void bt_avrcp_get_nowplaying_list(UINT32 offset, UINT32 count);
void bt_avrcp_play_item(UINT8 *uid);
void bt_avrcp_get_folder_list(UINT32 start_item, UINT32 end_item);
void bt_avrcp_set_player_app(UINT32 eID, UINT32 eValue);
void bt_avrcp_change_path(UINT8 direction, UINT8 *folder_uid);
void bt_avrcp_get_play_status(void);
void bt_avrcp_query_status(void);
void bt_avrcp_bip_art();
#ifdef CONFIG_ABS_VOL
void bt_avrcp_send_abs_vol(int abs_vol);
void bt_avrcp_volume_up();
void bt_avrcp_volume_down();
void bt_avrcp_set_config(UINT8 param);
void bt_avrcp_get_config();
#endif
#endif

/* External Functions Interface For BTC */
#ifdef CONFIG_HID
void bt_hid_connect(void);
void bt_hid_disconnect(void);
void bt_hid_send_coordinate(UINT8 button, INT16 x, INT16 y);
void bt_hid_send_key_value(UINT8 key_value);
void bt_hid_send_media_value(UINT8 comm_value);
void bt_hid_get_status(void);
#endif

#ifdef CONFIG_CBP
void bt_btc_start_calibrate_screen(void);
void bt_btc_cancel_calibrate_screen(void);
void bt_btc_set_screen_data(void);
void bt_btc_restart_calibrate_screen(void);
#endif

/* External Functions Interface For BTC */
#ifdef CONFIG_PBAP
void bt_pbap_get_phonebook(UINT8 store_type, UINT16 offset, UINT16 count);
void bt_pbap_disc_stop();
void bt_pbap_connect(UINT8 *address);
void bt_pbap_cancel_get_phonebook(void);
#endif

/* External Functions Interface For BTC */
#ifdef CONFIG_SPP
int bt_spp_get_status();
void bt_spp_start();
void bt_spp_connect(UINT16 channel, UINT8 *addr, char *uuid);
void bt_spp_disconnect();
void bt_spp_send_data(UINT8 *data, UINT32 len);
#endif

#ifdef CONFIG_FTP
void bt_ftp_connect(UINT8 *bd);
void bt_ftp_disconnect(void);
void bt_ftp_list_remote_folder(void);
void bt_ftp_set_local_path(char * path);
void bt_ftp_browse_remote_folder(void);
void bt_ftp_change_folder(char * path);
void bt_ftp_push_object(char * path);
void bt_ftp_pull_object(char * path);
void bt_ftp_delete_object(char *path);
void bt_ftp_abort(void);
#endif
void bt_set_debug_level(int level);
void bt_le_start_adv(char* advData,int advLen);
void bt_gatt_get_le_local_name(void);
void bt_le_stop_adv(void);

extern void open_pcm_capture_thread(void);
void bt_set_max_bond_devices(int );
void bt_discnnect_all_profile(void);

#endif /* _BT_FUNCTIONS_H_ */

