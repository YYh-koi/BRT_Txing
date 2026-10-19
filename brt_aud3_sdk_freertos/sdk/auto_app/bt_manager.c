/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		bt_manager.c
*
*	Description:
*
*
*	Create Date:		2014/05/29
***********************************************************************/
#include "bt_manager.h"
#include <unistd.h>
#include "brt_trace.h"
#include "fsm.h"
#include "profile.h"
#include "profiles_def.h"

#include "bt_functions.h"
#include "bt_power_manager.h"
#include "bt_audio_hw.h"
#include "internal_map.h"
#ifdef PATCH_RECONN
#include "patch_reconn.h"
#endif
#include "Auto_at_internal.h"
#include "Auto_avrcp.h"
#include "Auto_pbap.h"
#include "internal_manager.h"
#include "auto_gatt_event.h"
#include "att_uuid.h"
#ifdef CONFIG_SPP
#include "app_rfcomm.h"
#endif
#include "auto_a2dp.h"
#include "internal_hid.h"
#include "auto_hid.h"
#include "brtbt_interface.h"


#define TAG "bse_mgr"


extern profile_t *g_profiles[];
extern BOOL g_sco_connect;
static int r_conn_conut = 0;

static void profile_event_indcate(unsigned int event, void *param);

static void hlp_mgr_cbk(unsigned int event, void *param);
static void hlp_mgr_gap_cbk(unsigned int event, void *param);
#ifdef CONFIG_HFP
static void hlp_mgr_hf_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_A2DP
//SYSTEM_LOCK s_a2dp_mute_lock = NULL;
static void hlp_mgr_a2dp_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_AVRCP
static void hlp_mgr_avrcp_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_PBAP
static void hlp_mgr_pbap_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_HID
static void hlp_mgr_hid_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_SPP
static void hlp_mgr_spp_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_CBP
static void hlp_mgr_cb_screen_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_FTP
static void hlp_mgr_ftp_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_DUN
static void hlp_mgr_dun_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_GATT
static void hlp_mgr_gatt_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_OPP
static void hlp_mgr_opp_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_MAP
static void hlp_mgr_map_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_PAN
static void hlp_mgr_pan_cbk(unsigned int event, void *param);
#endif
#ifdef CONFIG_ANCS_CLIENT
static void hlp_mgr_ancs_cbk(unsigned int event, void *param);
#endif

static void porting_a2dp_event_cbk(unsigned int event, void *param);
static void bt_manager_wait_avrcp_cbk(void *arg);
static void bt_manager_delete_w4_avrcp_disc_timer(void *arg);

/* profile manager */
#ifdef CONFIG_MGR_STATIC
static void load_service_config();
#endif

static struct bt_manager_stru *s_bt_manager_data = NULL;

//The pointer for timer
#ifdef CONFIG_HID
static struct FsmTimer *s_timer_connect_hid = NULL;
#endif
#ifdef CONFIG_PBAP
static struct FsmTimer *s_timer_connect_pbap = NULL;
#endif
static struct FsmTimer *s_timer_notify_status = NULL;
struct FsmTimer *s_timer_connect_a2dp = NULL;
struct FsmTimer *s_timer_disconnect_a2dp = NULL;

static struct FsmTimer *s_auto_conn_timer = NULL;
static struct FsmTimer *s_linklost_conn_timer = NULL;

#ifdef CONFIG_IAP
static struct FsmTimer *s_timer_connect_iap2 = NULL;
void bt_manager_timer_connect_iap(UINT8 * bd_addr);
static UINT8 iap_state = ST_IAP_INIT;
#endif

int s_auto_conn_count = 5;
int s_linklost_conn_count = 0;
static int g_ffa0_service_registered = 0;
static UINT8 g_clcast_server_if = 0;
static UINT8 g_clcast_connected = 0;
static UINT16 g_clcast_conn_id = 0;
static UINT16 g_clcast_read_handle = 0;
static UINT16 g_clcast_write_handle = 0;
static UINT16 g_clcast_notify_handle = 0;
static UINT8 g_clcast_notify_enabled = 0;
static UINT16 g_clcast_mtu = 23;
static UINT8 g_clcast_read_value[256] = {0};
static UINT16 g_clcast_read_value_len = 0;
static UINT8 ll_rc_state = 0; //To Control reconnect profile when link lost .
static UINT8 sr_rc_state =  RECONNECT_PROFILE_HFP; //To Control reconnect profile when bluetooth sofrware(bluetooth stack) is reset.
#if 0
static struct FsmTimer *s_event_conn_timer = NULL;
static void hlp_mgr_event_conn_status()
{
	if (s_event_conn_timer) {
		FSM_DelTimer(&s_event_conn_timer);
		s_event_conn_timer = NULL;
	}

	if(app_a2dp_get_status() >= ST_A2DP_CONNECTED && hlp_hf_get_status() >= ST_HF_CONNECTED)
	{
		int event = hlp_hf_get_status();
		bse_log_info(TAG, "hfp stat %d.", event);
		event_hf_indication(EV_HF_STAT, (void *)event);
	}
}
#endif
static void profile_event_indcate(unsigned int event, void *param)
{
	unsigned int profile_id = event >> 8;
	unsigned int evt = event & 0xFF;

	switch (profile_id) {
	case PROFILE_ID_MGR:
		hlp_mgr_cbk(evt, param);
		break;
	case PROFILE_ID_GAP:
		hlp_mgr_gap_cbk(evt, param);
		break;
#ifdef CONFIG_HFP
	case PROFILE_ID_HFP:
		hlp_mgr_hf_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_A2DP
	case PROFILE_ID_A2DP:
		hlp_mgr_a2dp_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_AVRCP
	case PROFILE_ID_AVRCP:
		hlp_mgr_avrcp_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_PBAP
	case PROFILE_ID_PBAP:
		hlp_mgr_pbap_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_HID
	case PROFILE_ID_HID:
		hlp_mgr_hid_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_CBP
	case PROFILE_ID_CBP:
		hlp_mgr_cb_screen_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_SPP
	case PROFILE_ID_SPP:
		hlp_mgr_spp_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_FTP
	case PROFILE_ID_FTP:
		hlp_mgr_ftp_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_DUN
	case PROFILE_ID_DUN:
		hlp_mgr_dun_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_GATT
	case PROFILE_ID_GATT:
		hlp_mgr_gatt_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_OPP
	case PROFILE_ID_OPP:
		hlp_mgr_opp_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_MAP
	case PROFILE_ID_MAP:
		hlp_mgr_map_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_PAN
	case PROFILE_ID_PAN:
		hlp_mgr_pan_cbk(evt, param);
		break;
#endif
#ifdef CONFIG_ANCS_CLIENT
	case PROFILE_ID_ANCS:
		hlp_mgr_ancs_cbk(evt, param);
		break;
#endif
	}
}

/* Initialize Environment of Every Module */
void bt_mgr_server_init(module_event_cbk_t event_cbk)
{
	int i;

	/* Initializes Bluetooth Manager */
	if (s_bt_manager_data == NULL)
		s_bt_manager_data = (struct bt_manager_stru *)NEW(sizeof(struct bt_manager_stru));

	memset(s_bt_manager_data, 0, sizeof(struct bt_manager_stru));
	bt_event_lock_init();
	s_bt_manager_data->event_cbk = event_cbk;

#ifdef CONFIG_BIND_PROFILES
	s_bt_manager_data->binded_profiles = BIND_PROFILE_HFP_A2DP | BIND_PROFILE_A2DP_AVRCP;
#else
	s_bt_manager_data->binded_profiles = 0;
#endif
	//s_a2dp_mute_lock = CREATE_CRITICAL("mute");
#ifdef CONFIG_MGR_DYNAMIC
	s_bt_manager_data->local_profile_flag = 0;
	s_bt_manager_data->local_profile_status = 0;
#endif
#ifdef CONFIG_MGR_STATIC
	s_bt_manager_data->disabled_service = 0;
	load_service_config();
#endif
	/* initialize Module */
	for (i=0; i<MAX_PROFILE_NUM; i++) {
#ifdef CONFIG_MGR_STATIC
		if((s_bt_manager_data->disabled_service & (1 << i)) != 0) {
			bse_log_info(TAG, "disabled service : %d", i);
			continue ;
		}
#endif
		if (g_profiles[i] != NULL) {
			if (g_profiles[i]->init != NULL) {
				g_profiles[i]->init(profile_event_indcate);
			}
#ifdef CONFIG_MGR_DYNAMIC
			s_bt_manager_data->local_profile_status |= (1 << i);
#endif
		}
	}
}

/* Start All Server*/
void bt_mgr_server_start(void)
{
	int i;

	/* Start Module */
	for (i=0; i<MAX_PROFILE_NUM; i++) {
#ifdef CONFIG_MGR_STATIC
		if((s_bt_manager_data->disabled_service & (1 << i)) != 0) {
			bse_log_info(TAG, "disabled service : %d", i);
			continue ;
		}
#endif
		if (g_profiles[i] != NULL) {
			if (g_profiles[i]->start != NULL) {
				g_profiles[i]->start();
			}
		}
	}
}

void bt_mgr_server_stop()
{
	int i;

	/* done in reverse order against init */
	for (i=MAX_PROFILE_NUM-1; i>=0; i--) {
#ifdef CONFIG_MGR_STATIC
		if((s_bt_manager_data->disabled_service & (1 << i)) != 0) {
			bse_log_info(TAG, "disabled service : %d", i);
			continue ;
		}
#endif
		if (g_profiles[i] != NULL) {
			if (g_profiles[i]->stop != NULL) {
				g_profiles[i]->stop();
			}
		}
	}
}

void bt_mgr_server_done(void)
{
	int i;

	if (s_linklost_conn_timer) {
			FSM_DelTimer(&s_linklost_conn_timer);
			s_linklost_conn_timer = NULL;
	}
	/* done in reverse order against init */
	for (i=MAX_PROFILE_NUM-1; i>=0; i--) {
#ifdef CONFIG_MGR_STATIC
		if((s_bt_manager_data->disabled_service & (1 << i)) != 0) {
			bse_log_info(TAG, "disabled service : %d", i);
			continue ;
		}
#endif
		if (g_profiles[i] != NULL) {
			if (g_profiles[i]->done != NULL) {
#ifdef CONFIG_MGR_DYNAMIC
				s_bt_manager_data->local_profile_status &= (0 << i);
#endif
				g_profiles[i]->done();
			}
		}
	}

	if(s_bt_manager_data) {
		if (s_bt_manager_data->timer_w4_avrcp_disc != NULL) {
			FSM_DelTimer(&s_bt_manager_data->timer_w4_avrcp_disc);
			s_bt_manager_data->timer_w4_avrcp_disc = NULL;
		}
		bse_log_info(TAG, "[DONE] bt manager\r\n");
		FREE(s_bt_manager_data);
		s_bt_manager_data = NULL;
	}
	bt_event_lock_done();
}

/*Set TRUE for auto connect a2dp after hfp connected, FALSE for not*/
void bt_manager_bind_hfp_a2dp(BOOL bind)
{
	if (bind) {
		s_bt_manager_data->binded_profiles |= BIND_PROFILE_HFP_A2DP;
	} else {
		s_bt_manager_data->binded_profiles &= ~BIND_PROFILE_HFP_A2DP;
	}
}

void bt_manager_bind_a2dp_avrcp(BOOL bind)
{
	if (bind) {
		s_bt_manager_data->binded_profiles |= BIND_PROFILE_A2DP_AVRCP;
	} else {
		s_bt_manager_data->binded_profiles &= ~BIND_PROFILE_A2DP_AVRCP;
	}
}

BOOL bt_manager_hfp_a2dp_is_binded()
{
	return (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_A2DP) > 0 ? TRUE : FALSE;
}

BOOL bt_manager_a2dp_avrcp_is_binded()
{
	return (s_bt_manager_data->binded_profiles & BIND_PROFILE_A2DP_AVRCP) > 0 ? TRUE : FALSE;
}

/* Interface of BT Manager Functions */
UINT16 bt_manager_get_conn_hdl(void)
{
	return s_bt_manager_data->conn_hdl;
}

/* Timer Functions */
void bt_manager_set_auto_ctl(UINT8 type, UINT8 mask)
{
	if (type == APP_AUTO_CONTROL_TYPE_SET) {
		UINT8 bd_addr[BD_ADDR_LEN] = {0};

		if (app_dev_get_address(0, bd_addr) == 1 && (mask & APP_AUTO_CONTROL_FLAG_CONNECT) && (sr_rc_state>0)) {
			app_hf_connect(bd_addr);
		}
		s_bt_manager_data->auto_ctl_flag |= mask;
		MANAGER_USER_DATA->local_info->flag |= mask;
	} else if (type == APP_AUTO_CONTROL_TYPE_CLR) {
		s_bt_manager_data->auto_ctl_flag &= ~mask;
		MANAGER_USER_DATA->local_info->flag &= ~mask;
	}
#ifndef CONFIG_SETTING_BY_USER
	hlp_manager_save_configure(0, DEV_INFO_LEN);
#endif
}

void bt_manager_timer_auto_conn(UINT8 *bd_addr)
{
        bse_log_api(TAG,"%s",__FUNCTION__);
	if (bd_addr != NULL) {
		app_hf_connect(bd_addr);
		FREE(bd_addr);
	} else {
		bt_hf_connect(0, NULL);
		if (s_auto_conn_count > 0) {
			s_auto_conn_count--;
		}
	}
	s_auto_conn_timer = NULL;
}

void bt_manager_timer_linklost( UINT8 *bd_addr)
{
	UINT8 bd[BD_ADDR_LEN];
	if (s_linklost_conn_count == 0) {
		goto END;
	}
	memset(bd, 0, sizeof(bd));
	if (app_dev_get_address(0, bd) == 1) {
		if (ll_rc_state & RECONNECT_PROFILE_HFP) {
			app_hf_connect(bd);
		}
		if (s_linklost_conn_count > 0) {
			s_linklost_conn_count--;
			s_linklost_conn_timer = FsmAddTimer(NULL, 20000, 0, NULL, NULL, (UINT32) bt_manager_timer_linklost);
			return;
		}
	}
END:
	s_linklost_conn_timer = NULL;
}

static void delete_auto_conn_timer_if_exists()
{
	s_auto_conn_count = 0;
	if (s_auto_conn_timer) {
		FSM_DelTimer(&s_auto_conn_timer);
		s_auto_conn_timer = NULL;
	}
}

void bt_manager_timer_auto_answer(void *arg)
{
	app_hf_answer_call();
}

struct FsmTimer *bt_manager_get_timer_notify_status() 
{
	return s_timer_notify_status;
}

#define PROFILE_BT_A2DP_STATUS 		1
#define PROFILE_BT_AVRCP_STATUS 	1<<1
#define PROFILE_BT_HF_STATUS 		1<<2
#define PROFILE_BT_PBAP_STATUS 		1<<3
#define PROFILE_BT_RFCOMM_STATUS 	1<<4
#define PROFILE_BT_HID_STATUS 		1<<5

UINT16 sdkprofile = 0xFFFF;//1192;


int bt_manager_get_profiles_value_a2dp()
{
	return sdkprofile  & PROFILE_BT_A2DP_SINK;
}

int bt_manager_get_binded_profiles_value()
{
	bse_log_api(TAG, "%s s_bt_manager_data->binded_profiles = %d\r\n", __func__, sdkprofile);
	return sdkprofile;
}

int bt_manager_get_binded_profiles()
{
	if(s_bt_manager_data->binded_profiles & (BIND_PROFILE_HFP_A2DP | BIND_PROFILE_A2DP_AVRCP))
		return 1;

	bse_log_api(TAG, "bt_manager_get_binded_profiles A2DP AVRCP Not bound\r\n");
	return 0;
}

int bt_manager_set_curr_profile(UINT16 value)
{
	//sdkprofile = value;
	int profile_f = 0;

	if(value & PROFILE_BT_SPP) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_SPP) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_SPP)) &  0xffff;

	if(value & PROFILE_BT_GATT_SERVER) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_GATT_SERVER) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_GATT_SERVER)) &  0xffff;

	if(value & PROFILE_BT_GATT_CLIENT) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_GATT_CLIENT) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_GATT_CLIENT)) &  0xffff;

	if(value & PROFILE_BT_HFP) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_HFP) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_HFP)) &  0xffff;

	if(value & PROFILE_BT_AG) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_AG) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_AG)) &  0xffff;

	if(value & PROFILE_BT_A2DP_SINK) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_A2DP_SINK) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_A2DP_SINK)) &  0xffff;

	if(value & PROFILE_BT_A2DP_SOURCE) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_A2DP_SOURCE) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_A2DP_SOURCE)) &  0xffff;

	if(value & PROFILE_BT_AVRCP_CT) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_AVRCP_CT) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_AVRCP_CT)) &  0xffff;

	if(value & PROFILE_BT_AVRCP_TG) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_AVRCP_TG) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_AVRCP_TG)) &  0xffff;

	if(value & PROFILE_BT_HID_KEYBOARD) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_HID_KEYBOARD) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_HID_KEYBOARD)) &  0xffff;

	if(value & PROFILE_BT_PBAP_SERVER) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_PBAP_SERVER) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_PBAP_SERVER)) &  0xffff;

	if(value & PROFILE_BT_AAP) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_AAP) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_AAP)) &  0xffff;

	if(value & PROFILE_BT_IAP2) {
		
		sdkprofile = (sdkprofile | PROFILE_BT_IAP2) & 0xffff;
	} else 
		sdkprofile = (sdkprofile & (~PROFILE_BT_IAP2)) &  0xffff;

	bse_log_api(TAG, "bt_manager_set_curr_profile, value = %d sdkprofile = %d profile_f = %d\r\n", value, sdkprofile, profile_f);
#if 0
	if(profile_f == 2) {
		s_bt_manager_data->binded_profiles = BIND_PROFILE_HFP_A2DP | BIND_PROFILE_A2DP_AVRCP;
		//app_a2dp_cannel_deny();
		hlp_a2dp_register_sdp();
	} else {
		//app_a2dp_set_deny(NULL);
		hlp_a2dp_unregister_sdp();
		s_bt_manager_data->binded_profiles = BIND_PROFILE_HFP_A2DP;
	}
#endif
	if(sdkprofile & PROFILE_BT_A2DP_SINK ) {
		hlp_a2dp_register_sdp();
		if(sdkprofile & PROFILE_BT_HFP)
			s_bt_manager_data->binded_profiles |= BIND_PROFILE_HFP_A2DP;
		if(sdkprofile & PROFILE_BT_AVRCP_CT)
			s_bt_manager_data->binded_profiles |= BIND_PROFILE_A2DP_AVRCP;
	} else {
		hlp_a2dp_unregister_sdp();
		s_bt_manager_data->binded_profiles = 0;//BIND_PROFILE_HFP_A2DP;
	}
		
	bse_log_api(TAG, "bt_manager_set_curr_profile end, s_bt_manager_data->binded_profiles = %d\r\n", s_bt_manager_data->binded_profiles);

}

#ifdef CONFIG_HID
void bt_manager_timer_connect_hid(void *arg)
{
	app_hid_connect(hlp_manager_get_last_device_bd_addr());

	s_timer_connect_hid = NULL;
}
#endif

#ifdef CONFIG_PBAP
void bt_manager_timer_connect_pbap(void *arg)
{
    bse_log_api(TAG,"%s\r\n",__FUNCTION__);
	app_pbap_connect(hlp_manager_get_last_device_bd_addr());
	s_timer_connect_pbap = NULL;
}
#endif

#ifdef CONFIG_A2DP
void bt_manager_timer_connect_a2dp(void *arg)
{
	if (arg != NULL) {
	        bse_log_api(TAG,"%s\r\n",__FUNCTION__);
		app_a2dp_connect(arg);
		FREE(arg);
	}
	s_timer_connect_a2dp = NULL;
}

void bt_manager_timer_disconnect_a2dp(void *arg)
{
	if(hlp_hf_get_status() < ST_HF_CONNECTED)
	{
		bse_log_api(TAG,"%s\r\n",__FUNCTION__);
		app_a2dp_disconnect();
	}
	s_timer_disconnect_a2dp = NULL;
}

#endif

static void bt_manager_timer_notify_status(void)
{
	//printf("bt_manager_timer_notify_status-------------------- hlp_avrcp_get_status() = %d\n", hlp_avrcp_get_status());
	//if (hlp_avrcp_get_status() >= ST_APP_AVRCP_PLAYING) {

	if((hlp_avrcp_get_config() & PROFILE_AVRCP_AUTO_GET_TRACK == 0) && (hlp_avrcp_get_config() & PROFILE_AVRCP_PLAY_PROGRESS == 0))
	{
		bse_log_info(TAG, "get tarck permission Insufficiency\r\n");
		return ;
	}

	int temp_avrcp_state = ST_APP_AVRCP_PLAYING;
	int is_continue = 0;
	if(!bt_manager_get_profiles_value_a2dp()) {
		if(hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED)
			is_continue = 1;
	} else {
		if(hlp_avrcp_get_status() == ST_APP_AVRCP_PLAYING)
			is_continue = 1;
	}
	bse_log_info(TAG, "bt_manager_timer_notify_status START is_continue = %d", is_continue);

	//if (hlp_avrcp_get_status() == ST_APP_AVRCP_PLAYING) {
	if(is_continue) {
		UINT8 avrcp_capability = app_get_avrcp_capability();
		s_timer_notify_status = NULL;

		if(hlp_avrcp_get_config() & PROFILE_AVRCP_AUTO_GET_TRACK)
			app_avrcp_get_nowplaying_info();

		//if(!(avrcp_capability & POS_CHANGED_SUPPORT)) {
		//if(!(avrcp_capability & POS_CHANGED_SUPPORT) && (hlp_avrcp_get_config() & PROFILE_AVRCP_PLAY_PROGRESS)) {
		if(hlp_avrcp_get_config() & PROFILE_AVRCP_PLAY_PROGRESS) {
			app_avrcp_get_play_status();
		}
		s_timer_notify_status = FsmAddTimer(NULL, 500/*1000*/, 0, NULL, "id3", (UINT32)bt_manager_timer_notify_status);
	} else {
		s_timer_notify_status = NULL;
	}
}

static void bt_manager_get_notify(UINT8 param)
{
	if (param == TRUE) {
		bse_log_info(TAG, "bt_manager_notify_status timer start\r\n");
		if (s_timer_notify_status == NULL) {
			s_timer_notify_status = FsmAddTimer(NULL, 1000, 0, NULL, NULL, (UINT32)bt_manager_timer_notify_status);
		}
	} else {
		bse_log_info(TAG, "bt_manager_notify_status timer end\r\n");
		if (s_timer_notify_status != NULL) {
			FSM_DelTimer(&s_timer_notify_status);
			s_timer_notify_status = NULL;
		}
	}
}

static int hlp_mgr_event_convert_to_AT_event(UINT8 event)
{
	int evt = -1;

	switch (event) {
	case EV_OTHER_PAIRIED_DEVICE_IND:
		evt = BT_EV_OTHER_PAIRIED_DEVICE_IND;
		break;
	case EV_OTHER_PAIRIED_DEVICE_END:
		evt = BT_EV_OTHER_PAIRIED_DEVICE_END;
		break;
	case EV_OTHER_LOCAL_DEVICE_NAME:
		evt = BT_EV_OTHER_LOCAL_DEVICE_NAME;
		break;
	case EV_OTHER_CHANNEL_LOCAL_DEVICE_NAME:
		evt = BT_EV_OTHER_LOCAL_DEVICE_NAME;
		break;
	case EV_OTHER_LOCAL_PINCODE:
		evt = BT_EV_OTHER_LOCAL_PINCODE;
		break;
	case EV_OTHER_LOCAL_BD_IND:
		evt = BT_EV_OTHER_LOCAL_BD_IND;
		break;
	case EV_OTHER_LOCAL_LE_BD_IND:
		evt = BT_EV_OTHER_LOCAL_LE_BD_IND;
		break;
	case EV_OTHER_AUTO_ANSWER_CONNECT_IND:
		evt = BT_EV_OTHER_AUTO_ANSWER_CONNECT_IND;
		break;
	case EV_OTHER_DUT_MODE_CONFIGURE:
		evt = BT_EV_OTHER_DUT_MODE_CONFIGURE;
		break;
	case EV_EVENT_OK:
		evt = BT_EV_EVENT_OK;
		break;
	default:
		break;
	}

	return evt;
}

static void hlp_mgr_cbk(unsigned int event, void *param)
{
	int evt = hlp_mgr_event_convert_to_AT_event(event);

	if (evt >= 0) {
		s_bt_manager_data->event_cbk(evt, param);
	}
}

static void hlp_manager_time_conn_avrcp_hid()
{
	if(app_avrcp_get_status() <= ST_APP_AVRCP_READY) {
		app_avrcp_connect(hlp_manager_get_last_device_bd_addr());	
	}

	if(app_hid_get_status() <= ST_APP_HID_READY)
		app_hid_connect(hlp_manager_get_last_device_bd_addr());
}

static void bt_manager_kill_a2dp_delay_timer()
{
	if (s_timer_connect_a2dp) {
		FSM_DelTimer(&s_timer_connect_a2dp);
		s_timer_connect_a2dp = NULL;
	}

	if (s_timer_disconnect_a2dp) {
		FSM_DelTimer(&s_timer_disconnect_a2dp);
		s_timer_disconnect_a2dp = NULL;
	}
} 

static void bt_manager_kill_pbap_conn_timer()
{
	if (s_timer_connect_pbap) {
		FSM_DelTimer(&s_timer_connect_pbap);
		s_timer_connect_pbap = NULL;
	}
}

static void bt_manager_kill_gap_conn_timer()
{
	if (s_auto_conn_timer) {
		FSM_DelTimer(&s_auto_conn_timer);
		s_auto_conn_timer = NULL;
	}
}

static void bt_manager_kill_hid_conn_timer()
{
	if (s_timer_connect_hid) {
		FSM_DelTimer(&s_timer_connect_hid);
		s_timer_connect_hid = NULL;
	}
}

static void bt_manager_kill_linklost_conn_timer()
{
	if (s_linklost_conn_timer) {
		FSM_DelTimer(&s_linklost_conn_timer);
		s_linklost_conn_timer = NULL;
	}
}


void bt_manager_kill_iap2_conn_timer()
{
#ifdef CONFIG_IAP
	if (s_timer_connect_iap2) {
		FSM_DelTimer(&s_timer_connect_iap2);
		s_timer_connect_iap2 = NULL;
	}
#endif
}


static int bt_manager_get_curr_profilr_start()
{
	UINT8 pf = 0;
	if(hlp_a2dp_get_status() >= ST_A2DP_CONNECTED)
		pf |= PROFILE_BT_A2DP_STATUS;
	if(hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED)
		pf |= PROFILE_BT_AVRCP_STATUS;
	if(hlp_hf_get_status() >= ST_HF_CONNECTED)
		pf |= PROFILE_BT_HF_STATUS;
	if(hlp_pbap_get_status() >= ST_PBAP_CONNECTED)
		pf |= PROFILE_BT_PBAP_STATUS;
	if(hlp_spp_get_status() >= ST_SPP_CONNECTED)
		pf |= PROFILE_BT_RFCOMM_STATUS;
	if(hlp_hid_get_status() >= ST_APP_HID_CONNECTED)
		pf |= PROFILE_BT_HID_STATUS;
	return pf;
	
}

void bt_manager_discnnect_all_profile()
{

	bt_manager_kill_a2dp_delay_timer();
	bt_manager_kill_pbap_conn_timer();
	bt_manager_kill_gap_conn_timer();
	bt_manager_kill_hid_conn_timer();
	bt_manager_kill_linklost_conn_timer();
	bt_manager_kill_iap2_conn_timer();

	UINT8 profile_f = bt_manager_get_curr_profilr_start();
	printf("profile_f = %d\n", profile_f);
	if(profile_f)
	{
		if(profile_f & PROFILE_BT_A2DP_STATUS)
			app_a2dp_disconnect();
		if(profile_f & PROFILE_BT_AVRCP_STATUS)
			app_avrcp_disconnect();
		if(profile_f & PROFILE_BT_HF_STATUS)
			app_hf_disconnect();
		if(profile_f & PROFILE_BT_PBAP_STATUS)
			app_pbap_disconnect();
		if(profile_f & PROFILE_BT_RFCOMM_STATUS){
			UINT32	handle_t;
			UINT8 bd_addr[BD_ADDR_LEN] = {0};
			handle_t = app_get_iap2_conn_handle();
			memcpy(bd_addr, app_get_iap2_conn_address(), BD_ADDR_LEN);
			if(!handle_t) {
				handle_t = app_get_aa_conn_handle();
				memcpy(bd_addr, app_get_iap2_conn_address(), BD_ADDR_LEN);
			}
				
			if(handle_t)
				app_spp_disconnect(bd_addr ,handle_t);
		}
		if(profile_f & PROFILE_BT_HID_STATUS)
			app_hid_disconnect(NULL);

	}	
}

static void hlp_mgr_gap_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_gap_cbk, event = %d, pointer of param = 0x%08x\r\n", event, param);

	switch (event) {
	case EV_APP_GAP_STARTED:
		s_bt_manager_data->event_cbk(BT_EV_SYSTEM_INIT_COMPLETE, param);
		on_profile_state_changed(PROFILE_ID_GAP, PROFILE_STATE_CONNECTED);

//zjgtest  default auto conn	
		MANAGER_USER_DATA->local_info->flag |= APP_AUTO_CONTROL_FLAG_CONNECT;
		
		if ((MANAGER_USER_DATA->local_info->flag & APP_AUTO_CONTROL_FLAG_CONNECT) && (sr_rc_state!=0)) {
			delete_auto_conn_timer_if_exists();
			if(s_auto_conn_timer == NULL) {
				s_auto_conn_count = 5;
				s_auto_conn_timer = FsmAddTimer(NULL, 2000, 0, NULL, NULL, (UINT32) bt_manager_timer_auto_conn);
			}
		}
		break;
	case EV_APP_GAP_SHUTTING_DOWN:
		break;
	case EV_APP_GAP_SHUTTED_DOWN:
		break;
	case EV_APP_GAP_RET_HARDWARE_DATA:
		hlp_manager_load_configure(param);
		sr_rc_state = hlp_manager_get_reconnet_profile(hlp_manager_get_last_device_bd_addr());
		break;
	case EV_APP_GAP_RET_SCAN_MODE:
		//if ((UINT8)(UINT32) param & 1<<1) {
		//	s_bt_manager_data->event_cbk(BT_EV_HF_ENTER_PAIRING_MODE, NULL);
		//} else {
		//	s_bt_manager_data->event_cbk(BT_EV_HF_EXIT_PAIRING_MODE, param);
		//}
		s_bt_manager_data->event_cbk(BT_EV_OTHER_LOCAL_SCAN_MODE_IND, param);
		break;
	case EV_APP_GAP_RET_LOCAL_BD:
		hlp_manager_save_local_device_bd(param);
		hlp_manager_query_local_device_bd_addr();
		break;
	case EV_APP_GAP_RET_LINK_KEY:
		hlp_manager_insert_link_key(param);
		break;
	case EV_APP_GAP_RET_AG_DEVICE_NAME: {
		struct HCI_Remote_Name_Request_CompleteEvStru *outp = (struct HCI_Remote_Name_Request_CompleteEvStru *)(param);

		if(strlen(outp->remote_name) > 0) {
			hlp_manager_insert_device_name(outp->bd, outp->remote_name);
		} else { //AG no have name
			hlp_manager_insert_device_name(outp->bd, "Unknow Name");
		}
        //zhonghong send GPF with AG device name
		s_bt_manager_data->event_cbk(BT_EV_HF_AG_DEVICE_NAME, outp->remote_name);
	}
	break;
	case EV_APP_GAP_REQ_LOCAL_DEVICE_CLASS: {
		struct gap_req_data_stru *p_req_data = (struct gap_req_data_stru *)param;

		p_req_data->p_ret_data = hlp_manager_get_local_device_class();
	}
	break;
	case EV_APP_GAP_REQ_LOCAL_DEVICE_NAME: {
		struct gap_req_data_stru *p_req_data = (struct gap_req_data_stru *)param;

		p_req_data->p_ret_data = hlp_manager_get_local_name();
	}
	break;
	case EV_APP_GAP_REQ_LINK_KEY: {
		struct gap_req_data_stru *p_data = (struct gap_req_data_stru *)param;

		p_data->p_ret_data = hlp_dev_get_link_key(p_data->p_reference);
	}
	break;
	case EV_APP_GAP_REQ_PINCODE: {
		struct gap_req_data_stru *p_data = (struct gap_req_data_stru *)param;

		p_data->p_ret_data = hlp_manager_get_local_pincode();
		break;
	}
	case EV_APP_GAP_DISCOVERY_STARTED: {
		s_bt_manager_data->event_cbk(BT_EV_DISCOVERY_STARTED, param);
		break;
	}
	case EV_APP_GAP_DISCOVERY_DEVICE_FOUND: {
		bt_remote_device_t * dev = (bt_remote_device_t *) param;
		if (dev && dev->name[0]) {
			hlp_manager_insert_device_name(dev->bd, dev->name);
		}
		s_bt_manager_data->event_cbk(BT_EV_DISCOVERY_DEVICE_FOUND, param);
		break;
	}
	case EV_APP_GAP_DISCOVERY_DEVICE_UPDATED: {
		bt_remote_device_t * dev = (bt_remote_device_t *) param;
		if (dev && dev->name[0]) {
			hlp_manager_insert_device_name(dev->bd, dev->name);
		}
		s_bt_manager_data->event_cbk(BT_EV_DISCOVERY_DEVICE_UPDATED, param);
		break;
	}
	case EV_APP_GAP_DISCOVERY_FINISHED: {
		s_bt_manager_data->event_cbk(BT_EV_DISCOVERY_FINISHED, param);
		break;
	}
	case EV_APP_GAP_HARDWARE_ERROR: {
		s_bt_manager_data->event_cbk(BT_EV_APP_GAP_HARDWARE_ERROR, param);
		break;
	}
	case EV_APP_GAP_AUTHENTICATION_FAILURE: {
		struct HCI_AddressStru *p_addr = (struct HCI_AddressStru *)param;
		bse_log_info(TAG, "Remove Link_key bd[0]: %02X, bd[5]: %02X\r\n", p_addr->bd[0], p_addr->bd[5]);
		hlp_manager_remove_bond(p_addr->bd);
		break;
	}
	case EV_APP_GAP_PIN_OR_KEY_MISSING: {
		struct HCI_AddressStru *p_addr = (struct HCI_AddressStru *)param;

		bse_log_info(TAG, "Remove Link_key bd[0]: %02X, bd[5]: %02X\r\n", p_addr->bd[0], p_addr->bd[5]);
		hlp_manager_remove_bond(p_addr->bd);
#ifdef PATCH_RECONN
		pr_set_reconn_mgr(PR_CONN_KEY_MISS);
#endif
		break;
	}
	case EV_APP_GAP_LINK_LOST: {
		bse_log_info(TAG, "[BRT]hlp_mgr_gap_cbk, EV_APP_GAP_LINK_LOST\n");
		//struct HCI_AddressStru *p_addr = (struct HCI_AddressStru *)param;
		if (s_linklost_conn_timer) {
			FSM_DelTimer(&s_linklost_conn_timer);
			s_linklost_conn_timer = NULL;
		}
		if(s_linklost_conn_timer == NULL) {
			s_linklost_conn_count = 16;
			ll_rc_state = s_bt_manager_data->reconn_state;
			s_linklost_conn_timer = FsmAddTimer(NULL, 20000, 0, NULL, NULL, (UINT32) bt_manager_timer_linklost);
		}
		break;
	}
	case EV_APP_GAP_ACL_DISCONNECTED: {
		on_profile_state_changed(PROFILE_ID_ACL, PROFILE_STATE_DISCONNECTED);
		break;
	}
	case EV_APP_GAP_RET_MIC_GAIN:
		s_bt_manager_data->event_cbk(BT_EV_HF_MIC_GAIN, param);
		break;
	case EV_APP_GAP_BOND_STATE_CHANGED:
		s_bt_manager_data->event_cbk(BT_EV_BOND_STATE_CHANGED, param);
		break;
	case EV_APP_GAP_CONNECTION_DISCONNECTED:
		s_bt_manager_data->event_cbk(BT_EV_CONNECTION_DISCONNECTED, param);
		break;
	case EV_APP_GAP_PASSKEY_NOTIF:
		s_bt_manager_data->event_cbk(BT_EV_PASSKEY_NOTIF, param);
		break;
	case EV_APP_GAP_PASSKEY_REQ:
		s_bt_manager_data->event_cbk(BT_EV_PASSKEY_REQ, param);
		break;
	case EV_APP_GAP_PINCODE_REQ:
		s_bt_manager_data->event_cbk(BT_EV_PINCODE_REQ, param);
		break;
	case EV_APP_GAP_USER_CONFIRMATION:
		s_bt_manager_data->event_cbk(BT_EV_USER_CONFIRMATION, param);
		break;
	case EV_APP_GAP_SERVICES_IND:
		s_bt_manager_data->event_cbk(BT_EV_APP_GAP_SERVICES_IND, param);
		break;
	case EV_APP_GAP_AUTHORIZE_REQUEST:
		bse_log_info(TAG, "hlp_mgr_gap_cbk EV_APP_GAP_AUTHORIZE_REQUEST");
		s_bt_manager_data->event_cbk(BT_EV_AUTHORIZE_REQUEST, param);
		break;
	case EV_APP_GAP_REMOVE_PLIST:
		bse_log_info(TAG, "hlp_mgr_gap_cbk BT_APP_GAP_REMOVE_PLIST");
		s_bt_manager_data->event_cbk(BT_APP_GAP_REMOVE_PLIST, param);
		break;
	case EV_APP_GAP_OK:
		s_bt_manager_data->event_cbk(BT_EV_EVENT_OK, param);
		break;
	default:
		break;
	}
}

#ifdef CONFIG_HFP
static void hlp_mgr_hf_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "bt_hf_cbk, event = %d, pointer of param = 0x%08x\r\n", event, param);

	switch (event) {
	case EV_HF_CONNECTING:
		s_bt_manager_data->event_cbk(BT_EV_HF_CONNECTING, param);
		break;
	case EV_HF_DISCONNECTED:
#ifdef CONFIG_DEFINE_BT_POWER
        if (!bt_bluetooth_is_disabling()) {
            app_gap_scan_mode_control(BT_SET_SCAN_MODE_TYPE_RECOVER, TRUE);
        }
#else
        //app_gap_scan_mode_control(BT_SET_SCAN_MODE_TYPE_RECOVER, TRUE);
#endif
		r_conn_conut = 0;
		if(s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_A2DP) {
			if (s_timer_connect_a2dp != NULL) {
				FSM_DelTimer(&s_timer_connect_a2dp);
				s_timer_connect_a2dp = NULL;
			}
			if (s_timer_disconnect_a2dp == NULL) {
				s_timer_disconnect_a2dp = FsmAddTimer(NULL, 500, 0, NULL, "dis_a2", (UINT32)bt_manager_timer_disconnect_a2dp);
			}
		}
		//delete_auto_conn_timer_if_exists();
#ifdef CONFIG_HID
		if (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_HID) {
			if (s_timer_connect_hid != NULL) {
				FSM_DelTimer(&s_timer_connect_hid);
				s_timer_connect_hid = NULL;
			}
			app_hid_disconnect(NULL);
		}
#endif

#ifdef CONFIG_CBP
		app_cb_screen_disconnect(CB_SCREEN_STOP_FLAG_DISCONNECT);
#endif
#ifdef CONFIG_PBAP
		if (app_pbap_get_flag_auto_connect() == TRUE) {
			if(hlp_pbap_get_status() == ST_PBAP_GETTING) {
				app_pbap_abort_phonebook();
			}
			app_pbap_stop();
		}
		
		if (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_PBAP) {
			if (s_timer_connect_pbap != NULL) {
				FSM_DelTimer(&s_timer_connect_pbap);
				s_timer_connect_pbap = NULL;
			}
			app_pbap_disconnect(NULL);
		}

#endif

#ifdef CONFIG_IAP
		if (s_timer_connect_iap2 != NULL) {
			FSM_DelTimer(&s_timer_connect_iap2);
			s_timer_connect_iap2 = NULL;
		}
#endif
		s_bt_manager_data->conn_hdl = 0x00;
		on_profile_state_changed(PROFILE_ID_HFP, PROFILE_STATE_DISCONNECTED);
#ifdef PATCH_RECONN
		if(pr_is_connect_by_app() ) { //&& pr_is_conn_pair_succ())
			if(pr_is_acl_disconnect_rem() || pr_is_conn_key_miss()) {
				bse_log_info(TAG, "hly####acl has disconnect by rem");
				UCHAR *bd = NEW(6);
				memcpy(bd, param, 6);
				FsmAddTimer(NULL, 1200, 0, bd, NULL, (UINT32)patch_reconn);
			} else {
				bse_log_info(TAG, "hly####acl not disconnect by rem");
				if(pr_is_conn_pair_succ()) {
					pr_set_reconn_mgr(PR_HFP_CONNECT_FAILED);
				} else {
					s_bt_manager_data->event_cbk(BT_EV_HF_DISCONNECTED, param);
					hlp_manager_set_last_device_bd_addr(NULL);
				}
			}
			break;
		} else {
			pr_clear_reconn_mgr();
			s_bt_manager_data->event_cbk(BT_EV_HF_DISCONNECTED, param);
		}
#else
		s_bt_manager_data->event_cbk(BT_EV_HF_DISCONNECTED, param);
#endif
		hlp_manager_set_last_device_bd_addr(NULL);
		s_bt_manager_data->reconn_state &= ~RECONNECT_PROFILE_HFP;

		break;
	case EV_HF_CONNECTED: {
		struct connected_info_t *connected_info = (struct connected_info_t *)param;
		hlp_manager_set_last_device_bd_addr(connected_info->bd);
		app_gap_get_remote_device_name(connected_info->bd);
		app_gap_scan_mode_control(BT_SET_SCAN_MODE_TYPE_DOUBLE, FALSE);
		delete_auto_conn_timer_if_exists();

		if (s_linklost_conn_timer) {
			FSM_DelTimer(&s_linklost_conn_timer);
			s_linklost_conn_timer = NULL;
		}
		s_linklost_conn_count = 0;
		bse_log_api(TAG,"connected_info->outgoing:%d",connected_info->outgoing);
		if (connected_info->outgoing) {
		        bse_log_api(TAG,"BIND_PROFILE_HFP_A2DP:%d",(s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_A2DP));
			if (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_A2DP) {
				if (s_timer_connect_a2dp == NULL) {
					UINT8 *bd = NEW(BD_ADDR_LEN);
					//app_a2dp_connect(connected_info->bd);
					memcpy(bd, connected_info->bd, BD_ADDR_LEN);
					s_timer_connect_a2dp = FsmAddTimer(NULL, 3000, 0, bd, NULL, (UINT32)bt_manager_timer_connect_a2dp);
				}
			}
#ifdef CONFIG_IAP
			bt_remote_device_t *rmt_dev = hlp_manager_find_target_device(connected_info->bd);
			if(rmt_dev) {
				bse_log_api(TAG,"rmt_dev->carplay:%d", rmt_dev->carplay);
				if (rmt_dev->carplay && (s_timer_connect_iap2 == NULL)) {
					UINT8 *iap_bd = NEW(BD_ADDR_LEN);
					memcpy(iap_bd, connected_info->bd, BD_ADDR_LEN);
					bse_log_api(TAG,"IAP2  tiemr start:%d");
					s_timer_connect_iap2 = FsmAddTimer(NULL, 1000, 0, iap_bd, NULL, (UINT32)bt_manager_timer_connect_iap);
				}
			}
#endif
		}


		if((sdkprofile & PROFILE_BT_A2DP_SINK) == 0) {
			FsmAddTimer(NULL, 1000, 0, NULL, NULL, (UINT32)hlp_manager_time_conn_avrcp_hid);
		}
		
/*#ifdef CONFIG_HID
			if (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_HID) {
				if (s_timer_connect_hid == NULL) {
					s_timer_connect_hid = FsmAddTimer(NULL, 500, 0, NULL, NULL, (UINT32)bt_manager_timer_connect_hid);
				}
			}
#endif*/

#ifdef CONFIG_PBAP
		if (s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_PBAP) {
			if (s_timer_connect_pbap == NULL) {
				s_timer_connect_pbap = FsmAddTimer(NULL, 1000, 0, NULL, NULL, (UINT32)bt_manager_timer_connect_pbap);
			}
		}
#endif

		app_gap_get_connect_handle(connected_info->bd, &(s_bt_manager_data->conn_hdl));
#ifdef PATCH_RECONN
		pr_clear_reconn_mgr();
#endif
		s_bt_manager_data->event_cbk(BT_EV_HF_CONNECTED, param);
		hlp_manager_insert_last_connected_device(NULL);
		s_bt_manager_data->reconn_state |= RECONNECT_PROFILE_HFP;
		if (sr_rc_state == 0) {
			sr_rc_state |= RECONNECT_PROFILE_HFP;
			hlp_manager_insert_reconn_profile(connected_info->bd, sr_rc_state);
		}

	}
	break;
	case EV_HF_CONN_CFM:
		s_bt_manager_data->event_cbk(BT_EV_HF_CONN_CFM, param);
		break;
	case EV_HF_OUTGOING:
		s_bt_manager_data->event_cbk(BT_EV_HF_OUTGOING, param);
		break;
	case EV_HF_INCOMMINGCALL:
		if (MANAGER_USER_DATA->local_info->flag & APP_AUTO_CONTROL_FLAG_ANSWER) {
			FsmAddTimer(NULL, 3000, 0, NULL, NULL, (UINT32)bt_manager_timer_auto_answer);
		}
		s_bt_manager_data->event_cbk(BT_EV_HF_INCOMMINGCALL, param);
		break;
	case EV_HF_STANDBY:
		s_bt_manager_data->event_cbk(BT_EV_HF_STANDBY, NULL);
		break;
	case EV_HF_ONGOING:
		s_bt_manager_data->event_cbk(BT_EV_HF_ONGOING, param);
		break;
	case EV_HF_AG_DEVICE_NAME:
		s_bt_manager_data->event_cbk(BT_EV_HF_AG_DEVICE_NAME, param);
		break;
	case EV_HF_AG_DEVICE_VENDOR_NAME:
		s_bt_manager_data->event_cbk(BT_EV_HF_AG_DEVICE_VENDOR_NAME, param);
		break;
	case EV_HF_AG_DEVICE_MODEL: {
		int mdl_len = 0;
		mdl_len = (strlen(param)>(MAX_DEVICE_MODEL_LEN-1)?(MAX_DEVICE_MODEL_LEN-1):strlen(param));
		memset(s_bt_manager_data->dev_model, 0, MAX_DEVICE_MODEL_LEN);
		memcpy(s_bt_manager_data->dev_model, param, mdl_len);
#ifdef CONFIG_IAP2
		app_cb_screen_os_type((UINT8 *)param, hlp_manager_get_last_device_bd_addr());
#endif
		s_bt_manager_data->event_cbk(BT_EV_HF_AG_DEVICE_MODEL, param);
	}
	break;
	case EV_HF_AG_OS_VERSION_IND:
#ifdef CONFIG_IAP2
		app_cb_screen_os_version((UINT8 *)param);
#endif
		break;
	case EV_HF_SCO_CONNECTED:
	{
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_open(AUDIO_TYPE_SCO, param);
#endif
		// bse_log_api(TAG,"EV_HF_SCO_CONNECTED, after play sco");
		s_bt_manager_data->event_cbk(BT_EV_HF_SCO_CONNECTED, NULL);
		
	}
	break;
	case EV_HF_SCO_DISCONNECTED:
		bse_log_api(TAG,"EV_HF_SCO_DISCONNECTED, befor close sco");
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_close(AUDIO_TYPE_SCO);
#endif
		bse_log_api(TAG,"EV_HF_SCO_DISCONNECTED, after close sco");
		s_bt_manager_data->event_cbk(BT_EV_HF_SCO_DISCONNECTED, NULL);
		break;
	case EV_HF_RETURN_STATUS:
		s_bt_manager_data->event_cbk(BT_EV_HF_RETURN_STATUS, param);
		break;
	case EV_HF_CONNECT_FAIL:
		bse_log_api(TAG,"s_auto_conn_count:%d, MANAGER_USER_DATA->local_info->flag:%x, sr_rc_state:%x", s_auto_conn_count, MANAGER_USER_DATA->local_info->flag, sr_rc_state);
		if (s_auto_conn_count && (MANAGER_USER_DATA->local_info->flag & APP_AUTO_CONTROL_FLAG_CONNECT) && (sr_rc_state>0)) {
			UINT32 nTimeCount = 15000;
			if( s_auto_conn_count == 4)
				nTimeCount = 9000;
			else if(s_auto_conn_count == 3)
				nTimeCount = 12000;

			if (memcmp(param,MANAGER_USER_DATA->last_bd_addr,BD_ADDR_LEN) == 0) {
				if (s_auto_conn_timer) {
					FSM_DelTimer(&s_auto_conn_timer);
					s_auto_conn_timer = NULL;
				}
				bse_log_api(TAG,"add timer %dms for bt_manager_timer_auto_conn", nTimeCount);
				s_auto_conn_timer = FsmAddTimer(NULL, nTimeCount, 0, NULL, NULL, (UINT32)bt_manager_timer_auto_conn);	
			}
		}
		break;
	case EV_HF_RINGTONE: {
		//struct app_hf_ring_param_stru *p_ring_param = (struct app_hf_ring_param_stru *)param;
               // bse_log_api(TAG,"###%d##%d##%d\r\n",p_ring_param->ring_count,g_sco_connect, *((UINT8 *)param));
		//if ((p_ring_param->ring_count == 0x00) && (g_sco_connect == TRUE)) {
		        bse_log_api(TAG,"BT_EV_HF_RINGTONE:%d",*(UINT8 *)param);
			s_bt_manager_data->event_cbk(BT_EV_HF_RINGTONE, param);
		//}
	}
	break;
	case EV_HF_RINGTONE_TERMINATED:
		s_bt_manager_data->event_cbk(BT_EV_HF_RINGTONE_TERMINATED, param);
		break;
	case EV_HF_NETWORK_OPERATOR_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_NETWORK_OPERATOR_IND, param);
		break;
	case EV_HF_SIGNAL_STRENGTH_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_SIGNAL_STRENGTH_IND, param);
		break;
	case EV_HF_BATTERY_CHARGE_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_BATTERY_CHARGE_IND, param);
		break;
	case EV_HF_SUBSCRIBER_NUMBER_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_SUBSCRIBER_NUMBER_IND, param);
		break;
	case EV_HF_3_WAY_CALL_NEW:
		s_bt_manager_data->event_cbk(BT_EV_HF_3_WAY_CALL_NEW, param);
		break;
	case EV_HF_3_WAY_CALL_STATE_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_3_WAY_CALL_STATE_IND, param);
		break;
	case EV_HF_3_WAY_CALL_DONE:
		s_bt_manager_data->event_cbk(BT_EV_HF_3_WAY_CALL_DONE, param);
		break;
	case EV_HF_ROAMING_STATE_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_ROAMING_STATE_IND, param);
		break;
	case EV_HF_NETWORK_STATE_IND:
		s_bt_manager_data->event_cbk(BT_EV_HF_NETWORK_STATE_IND, param);
		break;
	case EV_HF_VOICE_RECOGN_START:
		s_bt_manager_data->event_cbk(BT_EV_HF_VOICE_RECOGN_START, param);
		break;
	case EV_HF_VOICE_RECOGN_STOP:
		s_bt_manager_data->event_cbk(BT_EV_HF_VOICE_RECOGN_STOP, param);
		break;
	case EV_HF_EXIT_CFG:
		s_bt_manager_data->event_cbk(BT_EV_HF_EXIT_CFG, param);
		break;
	case EV_HF_DEV_INFO:
		s_bt_manager_data->event_cbk(BT_EV_HF_DEV_INFO, param);
		break;
	case EV_HF_RINGTONE_ON:
		s_bt_manager_data->event_cbk(BT_EV_HF_RINGTONE_ON, param);
		break;
	case EV_HF_RINGTONE_OFF:
		s_bt_manager_data->event_cbk(BT_EV_HF_RINGTONE_OFF, param);
		break;
	case EV_HF_STAT:
		s_bt_manager_data->event_cbk(BT_EV_PROFILE_HFPSTAT, param);
		break;
	case EV_HF_OK:
		s_bt_manager_data->event_cbk(BT_EV_EVENT_OK, NULL);
		break;
	default:
		break;
	}
        bse_log_api(TAG, "bt_hf_cbk, event = %d, pointer of param = 0x%08x Exit\r\n", event, param);
}
#endif

#ifdef CONFIG_A2DP
static void hlp_mgr_a2dp_cbk(unsigned int event, void *param)
{
	if (event != EV_A2DP_AV_DATA_IND)
		bse_log_api(TAG, "hlp_mgr_a2dp_cbk, event = %d, pointer of param = 0x%08x\r\n", event, param);

	switch(event) {
	case EV_A2DP_AV_OPEN: {
		r_conn_conut = 0;
#ifdef CONFIG_AVRCP
		if(bt_manager_get_binded_profiles())
			app_avrcp_connect((UINT8 *)param);
		//if (hlp_avrcp_get_status() == ST_APP_AVRCP_READY)
			//s_event_conn_timer = FsmAddTimer(NULL, 1000, 0, NULL, NULL, (UINT32)hlp_mgr_event_conn_status);
#endif
		s_bt_manager_data->event_cbk(BT_EV_A2DP_AV_OPEN, param);
		}break;
	case EV_A2DP_AV_PLAYING:
		//s_bt_manager_data->event_cbk(BT_EV_A2DP_AV_PLAYING, param);
		app_avrcp_get_play_status();
		break;
	case EV_A2DP_AV_SUSPEND:
		//s_bt_manager_data->event_cbk(BT_EV_A2DP_AV_SUSPEND, param);
		if(bt_manager_get_profiles_value_a2dp())
	    	bt_manager_get_notify(FALSE);
		break;
	case EV_A2DP_AV_DATA_IND:
		break;
	case EV_A2DP_DISCONNECTED: {
		BOOL w4_avrcp_disc = FALSE;
		on_profile_state_changed(PROFILE_ID_A2DP, PROFILE_STATE_DISCONNECTED);
#ifdef CONFIG_AVRCP

		if(hlp_hf_get_status() >= ST_HF_CONNECTED)
		{
			if (r_conn_conut < 3 && s_bt_manager_data->binded_profiles & BIND_PROFILE_HFP_A2DP) {
				if (s_timer_connect_a2dp == NULL) {
					UINT8 *bd = NEW(BD_ADDR_LEN);
					//app_a2dp_connect(connected_info->bd);
					memcpy(bd, param, BD_ADDR_LEN);
					r_conn_conut++;
					s_timer_connect_a2dp = FsmAddTimer(NULL, 700, 0, bd, NULL, (UINT32)bt_manager_timer_connect_a2dp);
					break;
				}
			}
			if(r_conn_conut > 3)
				r_conn_conut = 3;
		}
		if (s_bt_manager_data->binded_profiles & BIND_PROFILE_A2DP_AVRCP) {
			if (app_avrcp_get_status() == ST_APP_AVRCP_CONNECTING ||
			    app_avrcp_get_status() == ST_APP_AVRCP_CONNECTED) {
				w4_avrcp_disc = TRUE;
			}
		}
		if (w4_avrcp_disc) {
			UINT8 *bd  = (UINT8 *)NEW(BD_ADDR_LEN);
			memcpy(bd, param, BD_ADDR_LEN);
			s_bt_manager_data->timer_w4_avrcp_disc =
			    FsmAddTimer(NULL, 1000, 0, bd, NULL, (UINT32)bt_manager_wait_avrcp_cbk);
		} else
#endif
		{
			s_bt_manager_data->event_cbk(BT_EV_A2DP_DISCONNECTED, param);
		}
#ifdef CONFIG_AVRCP
		if(bt_manager_get_binded_profiles())
			app_avrcp_disconnect();
#endif
	}
	break;
	case EV_A2DP_RETURN_STATUS:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_RETURN_STATUS, param);
		break;
	case EV_A2DP_APTX_STATUS:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_APTX_STATUS, param);
		break;
	case EV_A2DP_PROFILESTAT:
		s_bt_manager_data->event_cbk(BT_EV_PROFILE_A2DPSTAT, param);
		break;
	case EV_A2DP_SET_MUTE:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_SET_MUTE, NULL);
		break;
	case EV_A2DP_SET_SAMP:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_SET_SAMP, param);
		break;
	case EV_A2DP_GET_RDEVICE_INFO:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_GET_DEV, param);
		break;
	case EV_A2DP_PLAY_STAT:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_PLAY_STAT, param);
		break;
	case EV_A2DP_OK:
		s_bt_manager_data->event_cbk(BT_EV_EVENT_OK, NULL);
		break;
	default:
		break;
	}

#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP
	porting_a2dp_event_cbk(event, param);
#endif
}
#endif

#ifdef CONFIG_AVRCP
static void hlp_mgr_avrcp_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_avrcp_cbk, event = %d, pointer of param = 0x%08x\r\n", event, param);

	switch(event) {
	case EV_AVRCP_CONN_COMPLETED:						//struct HLPAVRCPConnResultStru
		bt_manager_delete_w4_avrcp_disc_timer(param);
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_CONN_COMPLETED, param);
//#ifndef CONFIG_BIND_PROFILES
#if 1
		if(bt_manager_get_binded_profiles() && hlp_hf_get_status() < ST_HF_CONNECTED && s_timer_disconnect_a2dp == NULL)
		{
			s_timer_disconnect_a2dp = FsmAddTimer(NULL, 1000, 0, NULL, "dis_a2", (UINT32)bt_manager_timer_disconnect_a2dp);
		}
		if(!bt_manager_get_profiles_value_a2dp()) {
			bt_manager_get_notify(TRUE);
		}
#endif
		break;
	case EV_AVRCP_DISC_COMPLETED:
		bt_manager_delete_w4_avrcp_disc_timer(param);
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_DISC_COMPLETED, param);
		on_profile_state_changed(PROFILE_ID_AVRCP, PROFILE_STATE_DISCONNECTED);
		if (s_timer_notify_status != NULL) {
			bse_log_info(TAG, "s_timer_notify_status timer end\r\n");
			FSM_DelTimer(&s_timer_notify_status);
			s_timer_notify_status = NULL;
		}
		if(!bt_manager_get_profiles_value_a2dp()) {
			bt_manager_get_notify(FALSE);
		}
		break;
	case EV_AVRCP_GET_CAPS_CMF:
		if ((BOOL)param == TRUE) {
			app_avrcp_get_play_status();
			//app_avrcp_get_nowplaying_info();
		}
		break;
	case EV_AVRCP_ELEMENT_ATTRS:						//char *str
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_ELEMENT_ATTRS, param);
		break;
	case EV_AVRCP_FOLDER_LIST_INFO:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_FOLDER_LIST_INFO, param);
		break;
	case EV_AVRCP_NOW_PLAY_STATUS_IND:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_NOW_PLAY_STATUS_IND, param);
		break;
	case EV_AVRCP_ELEMENT_END:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_ELEMENT_END, NULL);
		break;
	case EV_AVRCP_UPDATA_UIDS:							//UINT16
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_UPDATA_UIDS, param);
		break;
	case EV_AVRCP_ELEMENT_ATTRS_OF_NOW_PLAYING:			//char *str
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_ELEMENT_ATTRS_OF_NOW_PLAYING, param);
		break;
	case EV_AVRCP_PLAYBACK_STATUS_CHANGED: {			//BOOL
		if (!memcmp(s_bt_manager_data->dev_model, "vivo X1w", strlen("vivo X1w"))) {
			if(bt_manager_get_profiles_value_a2dp())
            	bt_manager_get_notify(TRUE);
		} else {
		    s_bt_manager_data->event_cbk(BT_EV_AVRCP_PLAYBACK_STATUS_CHANGED, param);
			if(bt_manager_get_profiles_value_a2dp())
				bt_manager_get_notify((UINT8)param);
		}
		break;
	}
	case EV_AVRCP_PLAYBACK_POS_CHANGED:					//UINT32 pos
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_PLAYBACK_POS_CHANGED, param);
		break;
	case EV_AVRCP_NOW_PLAYING_CONTENT_CHANGED:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_NOW_PLAYING_CONTENT_CHANGED, param);
		break;
	case EV_AVRCP_BROWSING_CHANEL_COMPLETED:			//BOOL
		break;
	case EV_AVRCP_PLAYER_SETTTING_STATUS_IND:
		printf("EV_AVRCP_PLAYER_SETTTING_STATUS_IND\n");
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_PLAYER_SETTTING_STATUS_IND, param);
		break;
	case EV_AVRCP_RETURN_STATUS:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_RETURN_STATUS, param);
		break;
	case EV_AVRCP_STAT:
		s_bt_manager_data->event_cbk(BT_EV_PROFILE_AVRCPSTAT, param);
		break;
	case EV_AVRCP_PLAY_STAT:
		s_bt_manager_data->event_cbk(BT_EV_A2DP_PLAY_STAT, param);
		break;
	case EV_AVRCP_ABS_VOL_IND:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_ABS_VOL_IND, param);
		break;
	//BIP
	case EV_AVRCP_BIP_STATUS:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_BIP_STAT, param);
		break;
	case EV_AVRCP_BIP_COVERART_IMAGE:
		s_bt_manager_data->event_cbk(BT_EV_AVRCP_COVERART, param);
		break;
	default:
		break;
	}
        bse_log_api(TAG, "hlp_mgr_avrcp_cbk, event = %d, pointer of param = 0x%08x Exit\r\n", event, param);
}
#endif

#ifdef CONFIG_PBAP
static void hlp_mgr_pbap_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_pbap_cbk, event = %d, pointer of param = 0x%08x\r\n", event, param);

	switch(event) {
	case EV_PBAP_CONNECT_SUCCESS:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_CONNECT_SUCCESS, param);
		break;
	case EV_PBAP_CONNECT_FAIL:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_CONNECT_FAIL, param);
		break;
	case EV_PBAP_DISCONNECTED:
		on_profile_state_changed(PROFILE_ID_PBAP, PROFILE_STATE_DISCONNECTED);
		s_bt_manager_data->event_cbk(BT_EV_PBAP_DISCONNECTED, param);
		break;
	case EV_PBAP_PHONEBOOK_DATA_IND:
		//usleep(50000);
		s_bt_manager_data->event_cbk(BT_EV_PBAP_PHONEBOOK_DATA_IND, param);
		break;
	case EV_PBAP_CURR_CALL_PHONEBOOK_IND:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_CURR_CALL_PHONEBOOK_IND, param);
		break;
	case EV_PBAP_GET_PHONEBOOK_SUCCESS:
		//usleep(50000);
		s_bt_manager_data->event_cbk(BT_EV_PBAP_GET_PHONEBOOK_SUCCESS, param);
		break;
	case EV_PBAP_GET_PHONEBOOK_FAIL:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_GET_PHONEBOOK_FAIL, param);
		break;
	case EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS, param);
		break;
	case EV_PBAP_INDEX_DATA_IND:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_INDEX_DATA_IND, param);
		break;
	case EV_PBAP_SUPPORTED_REPOS:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_SUPPORTED_REPOS, param);
		break;
	case EV_PBAP_PHONEBOOK_RAWDATA_IND:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_PHONEBOOK_RAWDATA_IND, param);
		break;
	case EV_PBAP_PHONEBOOK_PHOTODATA_IND:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_PHONEBOOK_PHOTODATA_IND, param);
		break;
	case EV_PBAP_STAT_EV:
		s_bt_manager_data->event_cbk(BT_EV_PBAP_STAT_EVENT, param);
		break;
	default:
		break;
	}
        bse_log_api(TAG, "hlp_mgr_pbap_cbk, event = %d, pointer of param = 0x%08x Exit\r\n", event, param);
}
#endif

#ifdef CONFIG_HID
static void hlp_mgr_hid_cbk(unsigned int event, void *param)
{
	UINT32 api_event = 0xFFFF;
	bse_log_api(TAG, "hlp_mgr_hid_cbk event=%d\r\n", event);

	switch(event) {
	case EV_BTC_CONNECT_RET:
		if (param != NULL) {
			api_event = BT_EV_BTC_CONNECTED;
#ifdef CONFIG_CBP
			app_cb_screen_notify_ind(NF_CB_SCREEN_HID_CONNECT, param);
#endif
		} else {
#ifdef CONFIG_CBP
			app_cb_screen_notify_ind(NF_CB_SCREEN_HID_DISCONNECT, param);
#endif
			api_event = BT_EV_BTC_DISCONNECTED;
		}
		break;
	case EV_BTC_DISCONNECT_RET:
		api_event = BT_EV_BTC_DISCONNECTED;
#ifdef CONFIG_CBP
		app_cb_screen_notify_ind(NF_CB_SCREEN_HID_DISCONNECT, param);
#endif
		break;
	default:
		break;
	}

	s_bt_manager_data->event_cbk(api_event, param);
}
#endif

#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP
// FIXME constants should be the same as definition in porting module
#define PORTING_A2DP_EV_OPEN    0x01
#define PORTING_A2DP_EV_PLAYING 0x02
#define PORTING_A2DP_EV_SUSPEND 0x03
#define PORTING_A2DP_EV_CLOSE   0x04
#define PORTING_A2DP_EV_DATA    0x05
#define PORTING_A2DP_EV_CONFIG  0x06

/* Export the A2DP event to upper layer */
static void porting_a2dp_event_cbk(unsigned int event, void *param)
{
        if(event!=EV_A2DP_AV_DATA_IND)
             bse_log_api(TAG,"porting_a2dp_event_cbk:%d ++++",event);
	switch(event) {
	case EV_A2DP_AV_OPEN:
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_open(AUDIO_TYPE_A2DP, NULL);
#endif
		break;
	case EV_A2DP_AV_PLAYING:
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_start(AUDIO_TYPE_A2DP);
#endif
		break;
	case EV_A2DP_AV_SUSPEND:
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_pause(AUDIO_TYPE_A2DP);
#endif
		break;
	case EV_A2DP_DISCONNECTED:
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_close(AUDIO_TYPE_A2DP);
#endif
		break;
	case EV_A2DP_AV_DATA_IND: {
		struct BuffStru *buffer = (struct BuffStru *) param;
		if (buffer) {
#ifdef CONFIG_BT_AUDIO_CBK
#if (CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP == CONFIG_A2DP_DATA_SYSTEM_SUPPORT_PCM)
			bt_porting_audio_data_ind(AUDIO_TYPE_A2DP, BUFDATA(buffer)+1, DATASIZE(buffer));
#elif (CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP == CONFIG_A2DP_DATA_SYSTEM_SUPPORT_SBC)
			bt_porting_audio_data_ind(AUDIO_TYPE_A2DP, BUFDATA(buffer)+1, DATASIZE(buffer)-1);
#endif
#endif
		}
	}
	break;
	case EV_A2DP_AV_SET_CONFIG:
#ifdef CONFIG_BT_AUDIO_CBK
		bt_porting_audio_cfg(AUDIO_TYPE_A2DP, param); // param is sample rate
#endif
		break;
	default:
		break;
	}
    if(event!=EV_A2DP_AV_DATA_IND)
                 bse_log_api(TAG,"porting_a2dp_event_cbk:%d----",event);

}
#endif

#ifdef CONFIG_SPP
#ifndef CONFIG_SPP_ANDROID
#ifdef CONFIG_IAP
void bt_manager_timer_connect_iap(UINT8 * bd_addr)
{
    UINT8  iap2_client_uuid[16] = {0x00,0x00,0x00,0x00,0xDE,0xCA,0xFA,0xDE,0xDE,0xCA,0xDE,0xAF,0xDE,0xCA,0xCA,0xFE};
    if(iap_state > ST_IAP_READY) {
	s_timer_connect_iap2 = NULL;
	return;
   }

   if (bd_addr) {
        bt_spp_connect(0, bd_addr,iap2_client_uuid);
        FREE(bd_addr);
    } else {
        bt_spp_connect(0, NULL,iap2_client_uuid);
    }
    s_timer_connect_iap2 = NULL;
    bse_log_api(TAG, "bt_manager_timer_connect_spp!");
}
void bt_manager_kill_iap_conn_timer()
{
     if (s_timer_connect_iap2) {
        FSM_DelTimer(&s_timer_connect_iap2);
        s_timer_connect_iap2 = NULL;
    }
}
#endif
void bt_manager_disc_rfc_timer()
{
    UINT32 handle = app_rfc_srv_get_head_handle();
    if (handle) {
        app_rfc_server_disconnect(handle);
    }
    bse_log_api(TAG, "bt_manager_disc_rfc_timer,handle(0x%04x)",handle);

}
static void hlp_mgr_spp_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_spp_cbk, event=%d, param=0x%08x\r\n", event, param);

	switch (event) {
	case EV_APP_SPP_SERVER_STARTED:
	case EV_APP_SRV_RFC_SERVER_STARTED:
		rfc_server_info_t rfc_server_start_info;
		rfc_server_start_info = *(rfc_server_info_t *)param;
		if(is_iap2_uuid(rfc_server_start_info.rfc_srv_uuid128)) {
			iap_state = ST_IAP_READY;
		}	
		s_bt_manager_data->event_cbk(BT_EV_SPP_START, param);
		break;
	case EV_APP_SPP_SERVER_STOPPED:
	case EV_APP_SRV_RFC_SERVER_STOPPED:
		s_bt_manager_data->event_cbk(BT_EV_SPP_STOP, param);
		break;
	case EV_APP_SPP_CONNECTING:
		rfc_server_info_t rfc_server_connecting_info;
		rfc_server_connecting_info = *(rfc_server_info_t *)param;
		if(is_iap2_uuid(rfc_server_connecting_info.rfc_srv_uuid128)) {
			iap_state = ST_IAP_CONNECTING;
		}	
		break;
	case EV_APP_SPP_CONNECT_RET:
	case EV_APP_SRV_RFC_CONNECT_RET:
		rfc_connection_info_t rfc_conn_info;
		rfc_conn_info = *(rfc_connection_info_t *)param;
		if(rfc_conn_info.handle == app_get_aa_conn_handle()) {
			s_bt_manager_data->event_cbk(BT_EV_AAP_CONNECTED, param);
			iap_state = ST_IAP_CONNECTED;
		} else if (rfc_conn_info.handle == app_get_iap2_conn_handle()) {
			s_bt_manager_data->event_cbk(BT_EV_IAP2_CONNECTED, param);
		} else {
			s_bt_manager_data->event_cbk(BT_EV_SPP_CONNECTED, param);
		}
		
		break;
	case EV_APP_SPP_DISCONNECT_RET:
	case EV_APP_SRV_RFC_DISCONNECT_RET:
		rfc_connection_info_t rfc_disconn_info;
		rfc_disconn_info= *(rfc_connection_info_t *)param;
		if(rfc_disconn_info.handle == app_get_aa_conn_handle() ) {
			s_bt_manager_data->event_cbk(BT_EV_AAP_DISCONNECTED, param);
			iap_state = ST_IAP_READY;
			app_clean_aa_conn_info();
		} else if (rfc_disconn_info.handle == app_get_iap2_conn_handle()) {
			s_bt_manager_data->event_cbk(BT_EV_IAP2_DISCONNECTED, param);
			app_clean_iap2_conn_info();
		} else{
			s_bt_manager_data->event_cbk(BT_EV_SPP_DISCONNECTED, param);
		}
		break;
	case EV_APP_SPP_SEND_DATA_RET:
	case EV_APP_SRV_RFC_SEND_DATA_RET:
		//s_bt_manager_data->event_cbk(BT_EV_SPP_SEND_DATA_RET, param);
		break;
	case EV_APP_SPP_DATA_IND: 
	case EV_APP_SRV_RFC_DATA_IND:{
		struct app_spp_data_ind_stru *data_ind = (struct app_spp_data_ind_stru *)param;
		if(data_ind->handle == app_get_aa_conn_handle()) {
			s_bt_manager_data->event_cbk(BT_EV_AAP_DATA_IND, (void *)data_ind);
		} else if (data_ind->handle == app_get_iap2_conn_handle()) {
			s_bt_manager_data->event_cbk(BT_EV_IAP2_DATA_IND, (void *)data_ind);
		} else {
			s_bt_manager_data->event_cbk(BT_EV_SPP_DATA_IND, (void *)data_ind);
		}
	}
	break;
	default:
		break;
	}
}
#else
static void hlp_mgr_spp_cbk(unsigned int event, void *param)
{
	switch (event) {
	case EV_SPP_RFCOMM_CONN_RESULT:
		s_bt_manager_data->event_cbk(BT_EV_SPP_RFCOMM_CONN_RESULT, param);
		break;
	case EV_SPP_RFCOMM_LISTEN_RESULT:
		s_bt_manager_data->event_cbk(BT_EV_SPP_RFCOMM_LISTEN_RESULT, param);
		break;
	default:
		break;
	}
}
#endif
#endif

#ifdef CONFIG_PAN
static int hlp_pan_event_convert_to_AT_event(UINT8 event)
{
	int evt = -1;

	switch (event) {
	case EV_PAN_CONNECTING:
		evt = BT_EV_PAN_CONNECTING;
		break;
	case EV_PAN_CONNECTED:
		evt = BT_EV_PAN_CONNECTED;
		break;
	case EV_PAN_DISCONNECTING:
		evt = BT_EV_PAN_DISCONNECTING;
		break;
	case EV_PAN_DISCONNECTED:
		evt = BT_EV_PAN_DISCONNECTED;
		break;
	case EV_PAN_CONTROL_STATE:
		evt = BT_EV_PAN_CONTROL_STATE;
		break;
	default:
		break;
	}

	return evt;
}

static void hlp_mgr_pan_cbk(unsigned int event, void *param)
{
	int evt = hlp_pan_event_convert_to_AT_event(event);

	if (event == EV_PAN_DISCONNECTED) {
		on_profile_state_changed(PROFILE_ID_PAN, PROFILE_STATE_DISCONNECTED);
	}

	if (evt >= 0) {
		s_bt_manager_data->event_cbk(evt, param);
	}
}
#endif

#ifdef CONFIG_DUN
static int hlp_dun_event_convert_to_AT_event(UINT8 event)
{
	int evt = -1;

	switch (event) {
	case EV_DUN_CONNECTING:
		evt = BT_EV_DUN_CONNECTING;
		break;
	case EV_DUN_CONNECTED:
		evt = BT_EV_DUN_CONNECTED;
		break;
	case EV_DUN_DISCONNECTING:
		evt = BT_EV_DUN_DISCONNECTING;
		break;
	case EV_DUN_DISCONNECTED:
		evt = BT_EV_DUN_DISCONNECTED;
		break;
	default:
		break;
	}

	return evt;
}

static void hlp_mgr_dun_cbk(unsigned int event, void *param)
{
	int evt = hlp_dun_event_convert_to_AT_event(event);

	if (event == EV_DUN_DISCONNECTED) {
		on_profile_state_changed(PROFILE_ID_DUN, PROFILE_STATE_DISCONNECTED);
	}

	if (evt >= 0) {
		s_bt_manager_data->event_cbk(evt, param);
	}
}

#endif

#ifdef CONFIG_MAP
static int hlp_map_event_convert_to_AT_event(int event)
{
	int evt = -1;

	switch (event) {
	case EV_MAP_CONNECTION_EVENT:
		evt = BT_EV_MAP_CONNECTION_EVENT;
		break;

	case EV_MAP_NEW_SMS_ARRIVED:
		evt = BT_EV_MAP_NEW_SMS_ARRIVED;
		break;

	case EV_MAP_RETRIEVE_SMS_RESULT:
		evt = BT_EV_MAP_RETRIEVE_SMS_RESULT;
		break;

	case EV_MAP_GET_SMS_LIST_RESULT:
		evt = BT_EV_MAP_GET_SMS_LIST_RESULT;
		break;

	case EV_MAP_SEND_SMS_RESULT:
		evt = BT_EV_MAP_SEND_SMS_RESULT;
		break;

	case EV_MAP_DELETE_SMS_RESULT:
		evt = BT_EV_MAP_DELETE_SMS_RESULT;
		break;

	case EV_MAP_BMSG_DATA_IND:
		evt = BT_EV_MAP_BMSG_DATA_IND;
		break;
	case EV_MAP_GET_SMS_LIST_COMPLETE:
		evt = BT_EV_MAP_GET_SMS_LIST_COMPLETE;
		break;

	default:
		break;
	}

	return evt;
}

static void hlp_mgr_map_cbk(unsigned int event, void *param)
{
	UINT result = (UINT8) param;
	int evt;

	switch (event) {
	case EV_MAP_CONNECTION_EVENT:

		if (result == MAP_RESULT_DISCONNECTED) {
			on_profile_state_changed(PROFILE_ID_MAP, PROFILE_STATE_DISCONNECTED);
		}
		break;
	}

	evt = hlp_map_event_convert_to_AT_event(event);

	switch (event) {
	case EV_MAP_CONNECTION_EVENT:
	case EV_MAP_DELETE_SMS_RESULT:
	case EV_MAP_SEND_SMS_RESULT: {
		char p[5];
		memset(p, 0, 5);
		snprintf(p, sizeof(p), "%02X%02X", result, 0);
		s_bt_manager_data->event_cbk(evt, p);
	}
	break;
	case EV_MAP_GET_SMS_LIST_RESULT:
	case EV_MAP_NEW_SMS_ARRIVED:
	case EV_MAP_BMSG_DATA_IND:
	case EV_MAP_GET_SMS_LIST_COMPLETE: {
		s_bt_manager_data->event_cbk(evt, param);
	}
	break;
	}
}

#endif

#ifdef CONFIG_GATT

short g_attr_handle = 0;;
short g_conn_id = 0;
UINT8 g_write_descriptor_status = 0;
UINT8* character_data = NULL;
int character_data_size = 0;
#ifdef JZY_GATT
extern module_event_cbk_t gattcbk;
#endif

extern module_event_cbk_t ey_event_cb;

static const UINT8 s_clcast_service_uuid[16] = {0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0xc9, 0xef, 0xde, 0xfd, 0xef, 0x9f};
static const UINT8 s_clcast_read_uuid[16]    = {0x00, 0x00, 0xff, 0xe1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0xc9, 0xef, 0xde, 0xfd, 0xef, 0x9f};
static const UINT8 s_clcast_write_uuid[16]   = {0x00, 0x00, 0xff, 0xe2, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0xc9, 0xef, 0xde, 0xfd, 0xef, 0x9f};
static const UINT8 s_clcast_notify_uuid[16]  = {0x00, 0x00, 0xff, 0xe3, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0xc9, 0xef, 0xde, 0xfd, 0xef, 0x9f};
static const UINT8 s_clcast_cccd_uuid[16]    = {0x00, 0x00, 0x29, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0xc9, 0xef, 0xde, 0xfd, 0xef, 0x9f};

extern int sw_get_ble_connection_type();

int clcast_ble_is_active(void)
{
	return sw_get_ble_connection_type() == BLE_CLCAST_CONNECTION;
}

int clcast_ble_set_read_value(const uint8_t *data, uint16_t len)
{
	if (!data || len > sizeof(g_clcast_read_value)) {
		return -1;
	}

	memcpy(g_clcast_read_value, data, len);
	g_clcast_read_value_len = len;
	return 0;
}

int clcast_ble_notify(const uint8_t *data, uint16_t len)
{
	UINT16 max_payload = g_clcast_mtu > 3 ? g_clcast_mtu - 3 : 20;

	if (!clcast_ble_is_active()) {
		return -1;
	}
	if (!data || len == 0) {
		return -2;
	}
	if (!g_clcast_connected) {
		return -3;
	}
	if (!g_clcast_notify_enabled) {
		return -4;
	}
	if (!g_clcast_notify_handle) {
		return -5;
	}
	if (len > max_payload) {
		return -6;
	}

	app_gatt_server_send_notification(g_clcast_server_if, g_clcast_notify_handle, g_clcast_conn_id, len, (unsigned char *)data);
	return 0;
}

static void hlp_mgr_gatt_cbk(unsigned int event, void *param)
{
   bse_log_api(TAG,"hlp_mgr_gatt_cbk, event=%d\r\n", event);
    switch (event) {
        case EV_GATT_CLIENT_CONNECTED:
        {
            evt_param_client_connected_t *p_connected = (evt_param_client_connected_t*) param;
            bse_log_api(TAG,"conn_id(%d),connected(%d)",p_connected->conn_id,p_connected->connected);
            //s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_CONNECT_CB, param);
            //gattcbk(BT_EV_GATT_CONNECTED, param);
        }
	break;
	  case EV_GATT_CONNECTED:
		if (clcast_ble_is_active()) {
			evt_param_connected_t *p_connected = (evt_param_connected_t*) param;
			if (p_connected) {
				g_clcast_conn_id = p_connected->conn_id;
				g_clcast_connected = 1;
			}
		}
		s_bt_manager_data->event_cbk(BT_EV_GATT_CONNECTED, param);
		break;
	  case EV_GATT_DISCONNECTED:
		if (clcast_ble_is_active()) {
			g_clcast_conn_id = 0;
			g_clcast_connected = 0;
			g_clcast_notify_enabled = 0;
		}
		s_bt_manager_data->event_cbk(BT_EV_GATT_DISCONNECTED, param);
		break;			
        case EV_GATT_SERVER_REGISTERED:
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_REGISTERED, param);

		evt_param_server_registered_t *p_server_registered;
		p_server_registered= (evt_param_server_registered_t *)param;

		bse_log_api(TAG,"hlp_mgr_gatt_cbk status:%d, server_if:%d,uuid:%02x %02x %02x %02x \n", p_server_registered->status, p_server_registered->server_if,
		p_server_registered->uuid.uu[0], p_server_registered->uuid.uu[1], p_server_registered->uuid.uu[14], p_server_registered->uuid.uu[15]);
		UINT8 ec_eir_uuid[16] ={0x00, 0x00, 0xb3, 0x60, 0xd6, 0xd8, 0xc7, 0xec, 0xbd, 0xf0, 0xea, 0xb1, 0xbf, 0xc6, 0xbc, 0xbc};
		UINT8 ey_eir_uuid[16] ={0x00, 0x00, 0xaa, 0xa0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
		if(memcmp(p_server_registered->uuid.uu, ec_eir_uuid, 16) == 0) {
			bse_log_api(TAG,"hlp_mgr_gatt_cbk is ec_uuid\n");

			btgatt_db_element_t pri_element ={.uuid.uu={0x00, 0x00, 0xb3, 0x60, 0xd6, 0xd8, 0xc7, 0xec, 0xbd, 0xf0, 0xea, 0xb1, 0xbf, 0xc6, 0xbc, 0xbc}};
			pri_element.type = BTGATT_DB_PRIMARY_SERVICE;
			hlp_gatt_server_save_primary_service_to_local_ext(p_server_registered->server_if, &pri_element);

			btgatt_db_element_t ccc_element ={.uuid.uu={0x00, 0x00, 0xb3, 0x62, 0xd6, 0xd8, 0xc7, 0xec, 0xbd, 0xf0, 0xea, 0xb1, 0xbf, 0xc6, 0xbc, 0xbc}};
			ccc_element.type = BTGATT_DB_CHARACTERISTIC;
			ccc_element.properties = ATT_NOTIFY|ATT_WRITE|ATT_WRITE_NORSP;
			hlp_gatt_server_save_characteristic_to_local_ext(p_server_registered->server_if, &ccc_element);

			btgatt_db_element_t des_element ={.uuid.uu={0x00, 0x00, 0x29, 0x02, 0xd6, 0xd8, 0xc7, 0xec, 0xbd, 0xf0, 0xea, 0xb1, 0xbf, 0xc6, 0xbc, 0xbc}};
			des_element.type = BTGATT_DB_DESCRIPTOR;
			ccc_element.properties = ATT_BROADCAST;
			hlp_gatt_server_save_descriptor_to_local_ext(p_server_registered->server_if, &des_element);

			hlp_gatt_add_all_Service(p_server_registered->server_if);
		}else if (memcmp(p_server_registered->uuid.uu, ey_eir_uuid, 16) == 0) {
			btgatt_db_element_t pri_element ={.uuid.uu={0x00, 0x00, 0xaa, 0xa0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}};
			pri_element.type = BTGATT_DB_PRIMARY_SERVICE;
			hlp_gatt_server_save_primary_service_to_local_ext(p_server_registered->server_if, &pri_element);

			for(int i = 0; i < 10; i++) {
				btgatt_db_element_t ccc_element ={.uuid.uu={0x00, 0x00, 0xaa, 0xa1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}};
				ccc_element.uuid.uu[3] = 0xa1 + i;
				ccc_element.type = BTGATT_DB_CHARACTERISTIC;
				ccc_element.properties = ATT_NOTIFY|ATT_READ|ATT_WRITE|ATT_WRITE_NORSP;
				hlp_gatt_server_save_characteristic_to_local_ext(p_server_registered->server_if, &ccc_element);

				btgatt_db_element_t des_element ={.uuid.uu={0x00, 0x00, 0x29, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}};
				des_element.type = BTGATT_DB_DESCRIPTOR;
				des_element.properties = ATT_BROADCAST;
				hlp_gatt_server_save_descriptor_to_local_ext(p_server_registered->server_if, &des_element);
			}

			hlp_gatt_add_all_Service(p_server_registered->server_if);			
		}
		else if(memcmp(p_server_registered->uuid.uu, s_clcast_service_uuid, 16) == 0) {
			btgatt_db_element_t pri_element = {0};
			btgatt_db_element_t read_element = {0};
			btgatt_db_element_t write_element = {0};
			btgatt_db_element_t notify_element = {0};
			btgatt_db_element_t desc_element = {0};

			g_clcast_server_if = p_server_registered->server_if;
			g_clcast_conn_id = 0;
			g_clcast_connected = 0;
			g_clcast_read_handle = 0;
			g_clcast_write_handle = 0;
			g_clcast_notify_handle = 0;
			g_clcast_notify_enabled = 0;
			g_clcast_mtu = 23;

			memcpy(pri_element.uuid.uu, s_clcast_service_uuid, 16);
			pri_element.type = BTGATT_DB_PRIMARY_SERVICE;
			hlp_gatt_server_save_primary_service_to_local_ext(p_server_registered->server_if, &pri_element);

			memcpy(read_element.uuid.uu, s_clcast_read_uuid, 16);
			read_element.type = BTGATT_DB_CHARACTERISTIC;
			read_element.properties = ATT_READ;
			hlp_gatt_server_save_characteristic_to_local_ext(p_server_registered->server_if, &read_element);

			memcpy(write_element.uuid.uu, s_clcast_write_uuid, 16);
			write_element.type = BTGATT_DB_CHARACTERISTIC;
			write_element.properties = ATT_WRITE | ATT_WRITE_NORSP;
			hlp_gatt_server_save_characteristic_to_local_ext(p_server_registered->server_if, &write_element);

			memcpy(notify_element.uuid.uu, s_clcast_notify_uuid, 16);
			notify_element.type = BTGATT_DB_CHARACTERISTIC;
			notify_element.properties = ATT_NOTIFY;
			hlp_gatt_server_save_characteristic_to_local_ext(p_server_registered->server_if, &notify_element);

			memcpy(desc_element.uuid.uu, s_clcast_cccd_uuid, 16);
			desc_element.type = BTGATT_DB_DESCRIPTOR;
			desc_element.properties = ATT_READ | ATT_WRITE;
			hlp_gatt_server_save_descriptor_to_local_ext(p_server_registered->server_if, &desc_element);

			hlp_gatt_add_all_Service(p_server_registered->server_if);
		}
		if (!clcast_ble_is_active() && !g_ffa0_service_registered) {
            g_ffa0_service_registered = 1;

            btgatt_db_element_t new_pri = {
                .uuid.uu = {0x00,0x00, 0xFF,0xA0, 0x00,0x00, 0x10,0x00,
                            0x80,0x00, 0x00,0x80, 0x5F,0x9B, 0x34,0xFB},
                .type = BTGATT_DB_PRIMARY_SERVICE
            };
            hlp_gatt_server_save_primary_service_to_local_ext(
                p_server_registered->server_if, &new_pri);

            btgatt_db_element_t new_ch = {
                .uuid.uu = {0x00,0x00, 0xFF,0xA1, 0x00,0x00, 0x10,0x00,
                            0x80,0x00, 0x00,0x80, 0x5F,0x9B, 0x34,0xFB},
                .type = BTGATT_DB_CHARACTERISTIC,
                .properties = ATT_NOTIFY | ATT_WRITE | ATT_WRITE_NORSP
            };
            hlp_gatt_server_save_characteristic_to_local_ext(
                p_server_registered->server_if, &new_ch);

            btgatt_db_element_t new_desc = {
                .uuid.uu = {0x00,0x00, 0x29,0x02, 0x00,0x00, 0x10,0x00,
                            0x80,0x00, 0x00,0x80, 0x5F,0x9B, 0x34,0xFB},
                .type = BTGATT_DB_DESCRIPTOR
            };
            hlp_gatt_server_save_descriptor_to_local_ext(
                p_server_registered->server_if, &new_desc);

            hlp_gatt_add_all_Service(p_server_registered->server_if);
        }
			
            //gattcbk(BT_EV_GATT_SERVER_REGISTERED, param);
            break;
        case EV_GATT_SERVICE_ADDED:
            //bt_manager_data->event_cbk(BT_EV_GATT_SERVICE_ADDED, param);
            if (clcast_ble_is_active()) {
                btgatt_db_element_t *element = (btgatt_db_element_t *)param;
                if (element && element->type == BTGATT_DB_CHARACTERISTIC) {
                    if (memcmp(element->uuid.uu, s_clcast_read_uuid, 16) == 0) {
                        g_clcast_read_handle = element->attribute_handle;
                    } else if (memcmp(element->uuid.uu, s_clcast_write_uuid, 16) == 0) {
                        g_clcast_write_handle = element->attribute_handle;
                    } else if (memcmp(element->uuid.uu, s_clcast_notify_uuid, 16) == 0) {
                        g_clcast_notify_handle = element->attribute_handle;
                    }
                }
            }
            if(ey_event_cb) {
            		ey_event_cb(EV_GATT_SERVICE_ADDED, param);			
		}
            break;
        case EV_GATT_SERVICE_STOPPED:
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVICE_STOPPED, param);
            break;
        case EV_GATT_SERVICE_DELETED:
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVICE_DELETED, param);
            break;
        case EV_GATT_RESPONSE_SEND_COMPLETED:
            //s_bt_manager_data->event_cbk(BT_EV_GATT_RESPONSE_SEND_COMPLETED, param);
           // gattcbk(BT_EV_GATT_RESPONSE_SEND_COMPLETED, param);
            break;
        case EV_GATT_SERVER_READ_CHARACTERISTIC:
        {
            //gattcbk(EV_GATT_SERVER_READ_CHARACTERISTIC, param);
             if(ey_event_cb) {
            		ey_event_cb(EV_GATT_SERVER_READ_CHARACTERISTIC, param);			
			}
            if (clcast_ble_is_active()) {
                evt_param_server_read_characteristic_t *msg = (evt_param_server_read_characteristic_t*)param;
                if (msg && (msg->handle == g_clcast_read_handle || msg->handle == (g_clcast_read_handle + 1))) {
                    app_gatt_server_send_response(g_clcast_server_if, msg->conn_id, msg->trans_id, 0, msg->handle, msg->offset, 0,
                                                  g_clcast_read_value_len, g_clcast_read_value);
                }
            }
           /* evt_param_server_read_characteristic_t *msg = (evt_param_server_read_characteristic_t*)param;
            app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0,character_data_size,character_data);
            g_attr_handle = msg->handle;
            g_conn_id = msg->conn_id;
            UART_Printf("EV_GATT_SERVER_READ_CHARACTERISTIC,attr_handle(0x%04x),conn_id(0x%04x)",g_attr_handle,g_conn_id);
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_READ_CHARACTER, param);*/

        }   break;
        case EV_GATT_SERVER_READ_DESCRIPTOR:
        {
            //gattcbk(EV_GATT_SERVER_READ_DESCRIPTOR, param);

            /*UINT8 data[2] = {0x01,0x00};
            evt_param_server_read_descriptor_t *msg = (evt_param_server_read_descriptor_t*)param;
            app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset, 0,2,data);

            UART_Printf("EV_GATT_SERVER_READ_DESCRIPTOR\r\n");
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_READ_DESCRIPTOR, param);*/

        }   break;
        case EV_GATT_SERVER_WRITE_CHARACTERISTIC:
        {
           // gattcbk(EV_GATT_SERVER_WRITE_CHARACTERISTIC, param);
           if(ey_event_cb) {
            		ey_event_cb(EV_GATT_SERVER_WRITE_CHARACTERISTIC, param);			
		}

            UINT8 i = 0;
            evt_param_server_write_characteristic_t *msg = (evt_param_server_write_characteristic_t*)param;
            bse_log_api(TAG,"EV_GATT_SERVER_WRITE_CHARACTERISTIC\r\n");

		UINT8 data[2] = {0x01,0x00};
            app_gatt_server_send_response(1,msg->conn_id,msg->trans_id, 0,msg->handle,msg->offset,0,2,data);

            if (character_data != NULL) {
                FREE(character_data);
                character_data = NULL;
            }
            character_data = NEW(msg->data_len);
            character_data_size = msg->data_len;
            memcpy(character_data,msg->data,msg->data_len);
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_WRITE_CHARACTER, param);

        }   break;
        case EV_GATT_SERVER_WRITE_DESCRIPTOR:
        {
            //gattcbk(EV_GATT_SERVER_WRITE_DESCRIPTOR, param);
            UINT8 i = 0;
            evt_param_server_write_descriptor_t *msg = (evt_param_server_write_descriptor_t*)param;
            printf("EV_GATT_SERVER_WRITE_DESCRIPTOR,handle(%d)",msg->handle);
            for (i= 0;i<msg->data_len;i++) {
                printf("0x%x ",msg->data[i]);
            }
            app_gatt_server_send_response(1,msg->conn_id,msg->trans_id,0,msg->handle,0,0,msg->data_len,msg->data);

            if (msg->data[0]== 0x01) {
                g_write_descriptor_status = 1;
            } else {
                g_write_descriptor_status = 0;
            }
            if (clcast_ble_is_active() && msg->data_len > 0) {
                g_clcast_notify_enabled = (msg->data[0] & 0x01) ? 1 : 0;
            }
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_WRITE_DESCRIPTOR, param);

        }   break;
        case EV_GATT_EXECUTE_WRITE:
            break;
        case EV_GATT_NOTIFICATION_SENT:
            s_bt_manager_data->event_cbk(BT_EV_GATT_NOTIFICATION_SENT, param);
            break;
        case EV_GATT_SERVER_CONGESTION:
            break;
        case EV_GATT_SERVER_MTU_CHANGED:
        {
            if (clcast_ble_is_active()) {
                evt_param_server_mtu_changed_t *msg = (evt_param_server_mtu_changed_t *)param;
                if (msg && msg->mtu > 0) {
                    g_clcast_mtu = msg->mtu;
                }
            }
            s_bt_manager_data->event_cbk(BT_EV_GATT_SERVER_MTU_CHANGED, param);
        } break;
        case EV_GATT_SERVER_PHY_READ:
            break;
        case EV_GATT_SERVER_PHY_UPDATE:
            break;
        case EV_GATT_SERVER_CONN_UPDATE:
            break;
        case EV_GATT_ADVERTISING_SET_STARTED:
            break;
        case EV_GATT_OWN_ADDRESS_READ:
            break;
        case EV_GATT_ADVERTISING_ENABLED:
            break;
        case EV_GATT_ADVERTISING_DATA_SET:
            break;
        case EV_GATT_SCAN_RESPONSE_DATA_SET:
            break;
        case EV_GATT_ADVERTISING_PARAMETERS_UPDATED:
            break;
        case EV_GATT_PERIODIC_ADVERTISING_PARAMETERS_UPDATED:
            break;
        case EV_GATT_PERIODIC_ADVERTISING_DATA_SET:
            break;
        case EV_GATT_PERIODIC_ADVERTISING_ENABLED:
            break;
        case EV_GATT_SYNC_LOST:
            break;
        case EV_GATT_SYNC_REPORT:
            break;
        case EV_GATT_SYNC_STARTED:
            break; 
		case EV_GATT_READ_REMOTE_RSSI: {
			bt_event_indicate(BT_EV_READ_RSSI, param);
		}          
     }
 }
 #endif


#ifdef CONFIG_OPP
static void hlp_mgr_opp_cbk(unsigned int event, void *param)
{
	int evt = -1;

	switch (event) {
	case EV_OPP_PUSH_STARTED:
		evt = BT_EV_OPP_PUSH_STARTED;
		break;
	case EV_OPP_PUSH_COMPLETED:
		evt = BT_EV_OPP_PUSH_COMPLETED;
		break;
	case EV_OPP_PUSH_IN_PROGRESS:
		evt = BT_EV_OPP_PUSH_IN_PROGRESS;
		break;
	case EV_OPP_PUSH_ABORTED:
		evt = BT_EV_OPP_PUSH_ABORTED;
		break;
	case EV_OPP_PULL_STARTED:
		evt = BT_EV_OPP_PULL_STARTED;
		break;
	case EV_OPP_PULL_COMPLETED:
		evt = BT_EV_OPP_PULL_COMPLETED;
		break;
	case EV_OPP_PULL_IN_PROGRESS:
		evt = BT_EV_OPP_PULL_IN_PROGRESS;
		break;
	case EV_OPP_PULL_ABORTED:
		evt = BT_EV_OPP_PULL_ABORTED;
		break;
	}

	if (evt >= 0) {
		s_bt_manager_data->event_cbk(evt, param);
	}
}
#endif

#ifdef CONFIG_FTP
static void hlp_mgr_ftp_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_ftp_cbk, event=%d\r\n", event);

	switch (event) {
	case EV_FTP_CONNECTING:
		s_bt_manager_data->event_cbk(BT_EV_FTP_CONNECTING, param);
		break;
	case EV_FTP_DISCONNECTED:
		s_bt_manager_data->event_cbk(BT_EV_FTP_DISCONNECTED, param);
		break;
	case EV_FTP_CONNECTED:
		s_bt_manager_data->event_cbk(BT_EV_FTP_CONNECTED, param);
		break;
	case EV_FTP_CONN_CFM:
		break;
	case EV_FTP_CHANGE_FOLDER_COMPLETE:
		s_bt_manager_data->event_cbk(BT_EV_FTP_CHANGE_FOLDER_COMPLETE, param);
		break;
	case EV_FTP_DISPLAY_OBJECT_IND:
		s_bt_manager_data->event_cbk(BT_EV_FTP_DISPLAY_OBJECT_IND, (void *)param);
		break;
	case EV_FTP_DISPLAY_COMPLETE:
		s_bt_manager_data->event_cbk(BT_EV_FTP_DISPLAY_COMPLETE, (void *)param);
		break;
	case EV_FTP_PUSH_OBJECT_STATED:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PUSH_OBJECT_STATED, param);
		break;
	case EV_FTP_PUSH_IN_PROGRESS:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PUSH_IN_PROGRESS, param);
		break;
	case EV_FTP_PUSH_OBJECT_COMPLETE:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PUSH_OBJECT_COMPLETE, param);
		break;
	case EV_FTP_DELETE_OBJECT_COMPLETE:
		s_bt_manager_data->event_cbk(BT_EV_FTP_DELETE_OBJECT_COMPLETE, param);
		break;
	case EV_FTP_PULL_OBJECT_STATED:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PULL_OBJECT_STATED, param);
		break;
	case EV_FTP_PULL_IN_PROGRESS:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PULL_IN_PROGRESS, param);
		break;
	case EV_FTP_PULL_OBJECT_COMPLETE:
		s_bt_manager_data->event_cbk(BT_EV_FTP_PULL_OBJECT_COMPLETE, param);
		break;
	default:
		break;
	}
}
#endif

static void bt_manager_check_a2dp_avrcp_status(void *arg)
{
	UINT8 bd[BD_ADDR_LEN];

	s_bt_manager_data->timer_w4_avrcp_disc = NULL;

	if (arg) {
		memcpy(bd, arg, BD_ADDR_LEN);
		//FREE(arg);
	} else {
		memset(bd, 0, BD_ADDR_LEN);
	}

	if (app_avrcp_get_status() == ST_APP_AVRCP_READY &&
	    app_a2dp_get_status() == ST_A2DP_READY) {
		s_bt_manager_data->event_cbk(BT_EV_A2DP_DISCONNECTED, bd);
	}

	if (app_avrcp_get_status() == ST_APP_AVRCP_CONNECTED &&
	    app_a2dp_get_status() == ST_A2DP_READY) {
		// TODO need to disconnect the avrcp connection?
		bse_log_warning(TAG, "%s", "WARNING!!! A2DP has disconnected but avrcp still connected");
	}
}

static void bt_manager_wait_avrcp_cbk(void *arg)
{
#ifdef CONFIG_AVRCP
	bt_manager_check_a2dp_avrcp_status(arg);
#endif
}

static void bt_manager_delete_w4_avrcp_disc_timer(void *arg)
{
	BOOL connected = (app_avrcp_get_status() == ST_APP_AVRCP_CONNECTED);
	bse_log_info(TAG, "avrcp %s.", (connected ? "connected" : "disconnect"));

	if (s_bt_manager_data->timer_w4_avrcp_disc != NULL) {
		FsmDelTimer2(s_bt_manager_data->timer_w4_avrcp_disc);
		s_bt_manager_data->timer_w4_avrcp_disc = NULL;

		bt_manager_check_a2dp_avrcp_status(arg);
	}
}

#ifdef CONFIG_CBP
static void hlp_mgr_cb_screen_cbk(unsigned int event, void *param)
{
	bse_log_api(TAG, "hlp_mgr_cb_screen_cbk, event=%d, param=%08x\r\n", event, param);

	switch (event) {
	case EV_BTC_CB_STARTED:
		break;
	case EV_BTC_CB_STOPPED:
		s_bt_manager_data->event_cbk(BT_EV_BTC_CB_STOPPED, NULL);
		break;
	case EV_BTC_CB_SUCCESS:
		s_bt_manager_data->event_cbk(BT_EV_BTC_CB_STARTED, NULL);
		break;
	case EV_BTC_CB_SPP_CONNECT_REQ:
		s_bt_manager_data->event_cbk(BT_EV_BTC_CB_SPP_CONNECT_REQ, param);
		break;
	case EV_BTC_CB_SPP_DISCONNECT_REQ:
		s_bt_manager_data->event_cbk(BT_EV_BTC_CB_SPP_DISCONNECT_REQ, NULL);
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_ANCS_CLIENT
static int hlp_ancs_event_convert_to_AT_event(UINT8 event)
{
	int evt = -1;

	switch (event) {
	case EV_ANCS_IOS_NOTIFICATION:
		evt = BT_EV_ANCS_IOS_NOTIFICATION;
		break;
	case EV_ANCS_CONNECTION_STATE:
		evt = BT_EV_ANCS_CONNECTION_STATE;
		break;
	case EV_ANCS_NOTIFICATION_ATTRIBUTE:
		evt = BT_EV_ANCS_NOTIFICATION_ATTRIBUTE;
		break;
	case EV_ANCS_APP_ATTRIBUTE:
		evt = BT_EV_ANCS_APP_ATTRIBUTE;
		break;
	default:
		break;
	}

	return evt;
}

static void hlp_mgr_ancs_cbk(unsigned int event, void *param)
{
	int evt = hlp_ancs_event_convert_to_AT_event(event);

	if (evt >= 0) {
		s_bt_manager_data->event_cbk(evt, param);
	}
}
#endif
void bt_audio_track_switch(int val)
{
#ifdef CONFIG_BT_AUDIO_CBK
	bt_porting_audio_switch(AUDIO_TYPE_A2DP, val);
#endif
}

void bt_audio_recoder_switch(int val)
{
#ifdef CONFIG_BT_AUDIO_CBK
	bt_porting_audio_switch(AUDIO_TYPE_SCO, val);
#endif
}

#ifdef CONFIG_MGR_STATIC
#define SERVICE_FILE_PATH "/etc/brlink/bt_service.cfg"
#define MAX_SERVICE_NUM 14

typedef struct service_stru {
	unsigned int service_id;
	char service_str[10];
} service_stru_t;
static service_stru_t s_ss[MAX_SERVICE_NUM] = {
	{PROFILE_ID_HFP, "HFP"},
	{PROFILE_ID_A2DP, "A2DP"},
	{PROFILE_ID_AVRCP, "AVRCP"},
	{PROFILE_ID_PBAP, "PBAP"},
	{PROFILE_ID_HID, "HID"},
	{PROFILE_ID_CBP, "CBP"},
	{PROFILE_ID_SPP, "SPP"},
	{PROFILE_ID_FTP, "FTP"},
	{PROFILE_ID_DUN, "DUN"},
	{PROFILE_ID_GATT, "GATT"},
	{PROFILE_ID_OPP, "OPP"},
	{PROFILE_ID_MAP, "MAP"},
	{PROFILE_ID_PAN, "PAN"},
	{PROFILE_ID_ANCS, "ANCS"},
};
static void load_service_config()
{
	int i = 0;
	int len = 0;
	int val_len = 16;
	UINT8 *val = NEW(val_len);

	for(i = 0; i < MAX_SERVICE_NUM; i++) {
		len = sizeof(s_ss[i].service_str);
		bse_log_verbose(TAG, "s_ss[%d] len is %d, str is %s", i, len, s_ss[i].service_str);
		//GetLocalProp
		memset(val, 0, val_len);
		GetProp("service", s_ss[i].service_str, val, &val_len, SERVICE_FILE_PATH);
		if(memcmp(val, "n", 1) == 0) {
			bse_log_verbose(TAG, "val is %s, while disabled %s service", val, s_ss[i].service_str);
			s_bt_manager_data->disabled_service |= (1 << s_ss[i].service_id);
		}

	}

	free(val);
	bse_log_api(TAG, "get disabled_service is %x", s_bt_manager_data->disabled_service);
}
#endif /* CONFIG_MGR_STATIC */

#ifdef CONFIG_MGR_DYNAMIC
static int disconnect_profile_if_exist(int profile_id)
{
	s_bt_manager_data->local_profile_flag |= (1 << profile_id);
	bse_log_api(TAG, "%s, profile_id is %d", __func__, profile_id);
	switch (profile_id) {
#ifdef CONFIG_HFP
	case PROFILE_ID_HFP:
		//FIXME HFP Stop will do disconnect if it was connected.
		if (s_auto_conn_timer) {
			FSM_DelTimer(&s_auto_conn_timer);
			s_auto_conn_timer = NULL;
		}
		return -1;
#endif
#ifdef CONFIG_A2DP
	case PROFILE_ID_A2DP:
		//FIXME A2dp Stop will do disconnect if it was connected.
		return -1;
#endif
#ifdef CONFIG_AVRCP
	case PROFILE_ID_AVRCP:
#if 0  //no manager for disable avrcp
		if (app_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
			app_avrcp_disconnect();
			break;
		}
#endif
		return -1;
#endif
#ifdef CONFIG_PBAP
	case PROFILE_ID_PBAP:
		if(hlp_pbap_get_status() >= ST_PBAP_CONNECTED) {
			if(hlp_pbap_get_status() == ST_PBAP_GETTING) {
				app_pbap_abort_phonebook();
			}
			app_pbap_stop();
			break;
		}
		return -1;
#endif
#ifdef CONFIG_HID
	case PROFILE_ID_HID:
		if(app_hid_get_status() >= ST_APP_HID_CONNECTING) {
			app_hid_disconnect(NULL);
			//break;
		}
		return -1;

#endif
#ifdef CONFIG_CBP
	case PROFILE_ID_CBP:
		//FIXME CBP Stop will do disconnect if it was connected.
		return -1;
#endif
#ifdef CONFIG_SPP
	case PROFILE_ID_SPP:
		/*FIXME*/
		app_spp_disconnect_all();
		return -1;
#endif
#ifdef CONFIG_FTP
	case PROFILE_ID_FTP:
		if(app_ftp_get_status() >= ST_FTP_CONNECTED) {
			app_ftp_disconnect();
			break;
		}
		return -1;
#endif
#ifdef CONFIG_DUN
	case PROFILE_ID_DUN:
		if(app_dun_get_status() >= ST_DUN_CONNECTED) {
			/*FIXME
			break;
			*/
			return -1;
		}
		return -1;
#endif
#ifdef CONFIG_GATT
	case PROFILE_ID_GATT:
		//FIXME
		return -1;
#endif
#ifdef CONFIG_OPP
	case PROFILE_ID_OPP:
		//FIXME
		return -1;
#endif
#ifdef CONFIG_MAP
	case PROFILE_ID_MAP:
		/*FIXME CBP Stop will do disconnect if it was connected.*/
		/*FIXME*/
		return -1;
#endif
#ifdef CONFIG_PAN
	case PROFILE_ID_PAN:
		if(app_pan_get_status() >= ST_PAN_CONNECTED) {
			//FIXME
			app_pan_disconnect();
			return -1;
		}
		return -1;
#endif

	}
	return 0;
}

/*
struct profile_mgr{
    int pfofile_id;
    int pfofile_status;
};
*/

void bt_manager_profile_disconnect_ind(int profile_id)
{
	bse_log_api(TAG, "bt_manager_profile_disconnect_ind profile_id is %d", profile_id);
	if(s_bt_manager_data->local_profile_flag & (1 << profile_id)) {
		s_bt_manager_data->local_profile_flag &= (0 << profile_id);
		if(g_profiles[profile_id]->stop != NULL) {
			g_profiles[profile_id]->stop();
		}
		if(g_profiles[profile_id]->done != NULL) {
			g_profiles[profile_id]->done();
		}
		s_bt_manager_data->local_profile_status &= (0 << profile_id);
		s_bt_manager_data->event_cbk(BT_EV_PROFILE_DISABLED, profile_id);
	}
}

void bt_manager_profile_operation(int profile_id, int param)
{
	bse_log_api(TAG, "%s-->%d", __func__, __LINE__);
	if(profile_id < PROFILE_ID_HFP && profile_id > PROFILE_ID_PAN) {
		return ;
	}
	if(param == PROFILE_ENABLE) {
		bse_log_info(TAG, "set profile id [%d]-->[ENABLE]", profile_id);
		if (g_profiles[profile_id]->init != NULL) {
			g_profiles[profile_id]->init(profile_event_indcate);
		}
		if(g_profiles[profile_id]->start != NULL) {
			g_profiles[profile_id]->start();
		}
		s_bt_manager_data->local_profile_status |= (1 << profile_id);
		s_bt_manager_data->event_cbk(BT_EV_PROFILE_ENABLED, profile_id);
	} else if(param == PROFILE_DISABLE) {
		if(disconnect_profile_if_exist(profile_id) == 0) {
			//TODO
		} else {
			bse_log_info(TAG, "set profile id [%d]-->[DISABLE]", profile_id);
			bt_manager_profile_disconnect_ind(profile_id);
		}
	}

}

void bt_sync_profile_state()
{
	s_bt_manager_data->event_cbk(BT_EV_PROFILE_STATUS, s_bt_manager_data->local_profile_status);
}

#endif /* CONFIG_MGR_DYNAMIC */

