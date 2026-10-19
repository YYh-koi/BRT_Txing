/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		bt_power_manager.c
*
*	Description:
*
*	Create Date:		2014/11/05
***********************************************************************/
#include "btdatatype.h"
#include "bt_power_manager.h"
#include "profile.h"
#include "internal_gap.h"
#include "internal_hf.h"
#include "internal_a2dp.h"
#include "internal_avrcp.h"
#include "internal_pbap.h"
#include "profiles_def.h"
#include "transport_tl.h"
#include "auto_at_internal.h"
#ifdef CONFIG_CHECK_LICENSE
#include "auto_license.h"
#endif
#include "bt_functions.h"
#include "brt_trace.h"
 #include <pthread.h>
#include "bt_manager.h"

#define POWER_MGR_DBG(fmt, ...) bse_log_info("bse_state", fmt, ## __VA_ARGS__)
#define CANCAL_DISCOVERY_AND_SET_INVISIABLE_TIMEOUT 100
#define DISCONNECT_PROFILE_TIMEOUT 3000
extern BOOL g_bt_opened;
enum tag_bt_state {
    BT_STATE_NONE,
    BT_STATE_ENABLING,
	BT_STATE_ENABLING_EX,
    BT_STATE_ENABLED,
    BT_STATE_DISABLING,
    BT_STATE_DISABLING_HFP,
    BT_STATE_DISABLING_A2DP,
    BT_STATE_DISABLING_AVRCP,
    BT_STATE_DISABLING_PBAP,
    BT_STATE_DISABLING_HID,
    /* TODO: need disconnect CBP separately or with HID together?*/
    /*BT_STATE_DISABLING_CBP,*/
    BT_STATE_DISABLING_SPP,
    BT_STATE_DISABLING_FTP,
    BT_STATE_DISABLING_DUN,
    BT_STATE_DISABLING_GATT,
    BT_STATE_DISABLING_OPP,
    BT_STATE_DISABLING_MAP,
    BT_STATE_DISABLING_PAN,
    BT_STATE_DISABLING_ACL,
    BT_STATE_DISABLING_HCI,
    BT_STATE_DISABLED,
    BT_STATE_COUNT
};

typedef struct tag_bt_state_management {
	int                 state;
	SYSTEM_LOCK         lock;
	int                 expected_state;
	struct FsmTimer     *wait_for_timer;
	UINT8               hci_enabled;
	UINT8               is_disabling;
} bt_state_management_t;

//static bt_state_management_t s_bt_state_management;

bt_state_management_t s_bt_state_management;

typedef void (*bt_action_cb)();
typedef struct tag_bt_pm_state_machine {
	int state;
	int state_transmit_to;
	bt_action_cb action;
} bt_pm_state_machine_t;

static bt_pm_state_machine_t s_sm[BT_STATE_COUNT] = {
	{BT_STATE_NONE,              BT_STATE_NONE,                 NULL},
	{BT_STATE_ENABLING,          BT_STATE_ENABLED,              bt_action_on_bluetooth_enabling},
	//{BT_STATE_ENABLING_EX,       BT_STATE_ENABLED,              bt_action_on_bluetooth_enabling_ex},
	{BT_STATE_ENABLED,           BT_STATE_NONE,                 bt_action_on_bluetooth_enabled},
	{BT_STATE_DISABLING,         BT_STATE_DISABLING_HFP,        bt_action_on_bluetooth_disabling},
	{BT_STATE_DISABLING_HFP,     BT_STATE_DISABLING_A2DP,       bt_disable_hfp},
	{BT_STATE_DISABLING_A2DP,    BT_STATE_DISABLING_AVRCP,      bt_disable_a2dp},
	{BT_STATE_DISABLING_AVRCP,   BT_STATE_DISABLING_PBAP,       bt_disable_avrcp},
	{BT_STATE_DISABLING_PBAP,    BT_STATE_DISABLING_GATT,       bt_disable_pbap},
	{BT_STATE_DISABLING_GATT,    BT_STATE_DISABLING_ACL,        bt_disable_gatt},
	{BT_STATE_DISABLING_ACL,     BT_STATE_DISABLING_HCI,        bt_disable_acl},
	{BT_STATE_DISABLING_HCI,     BT_STATE_DISABLED,             bt_disable_hci},
	{BT_STATE_DISABLED,          BT_STATE_NONE,                 bt_action_on_bluetooth_disabled}
};

#define CASE_RETURN_STR(const) case const: return #const;

static char *get_state_string(int state)
{
	switch (state) {
		CASE_RETURN_STR(BT_STATE_NONE);
		CASE_RETURN_STR(BT_STATE_ENABLING);
		CASE_RETURN_STR(BT_STATE_ENABLING_EX);
		CASE_RETURN_STR(BT_STATE_ENABLED);
		CASE_RETURN_STR(BT_STATE_DISABLING);
		CASE_RETURN_STR(BT_STATE_DISABLING_HFP);
		CASE_RETURN_STR(BT_STATE_DISABLING_A2DP);
		CASE_RETURN_STR(BT_STATE_DISABLING_AVRCP);
		CASE_RETURN_STR(BT_STATE_DISABLING_PBAP);
		CASE_RETURN_STR(BT_STATE_DISABLING_HID);
		CASE_RETURN_STR(BT_STATE_DISABLING_SPP);
		CASE_RETURN_STR(BT_STATE_DISABLING_FTP);
		CASE_RETURN_STR(BT_STATE_DISABLING_DUN);
		CASE_RETURN_STR(BT_STATE_DISABLING_GATT);
		CASE_RETURN_STR(BT_STATE_DISABLING_OPP);
		CASE_RETURN_STR(BT_STATE_DISABLING_MAP);
		CASE_RETURN_STR(BT_STATE_DISABLING_PAN);
		CASE_RETURN_STR(BT_STATE_DISABLING_ACL);
		CASE_RETURN_STR(BT_STATE_DISABLING_HCI);
		CASE_RETURN_STR(BT_STATE_DISABLED);
	default:
		return "Unknown state.";
	}
}

void bt_power_init()
{
	memset(&s_bt_state_management, 0, sizeof(bt_state_management_t));
	s_bt_state_management.lock = CREATE_CRITICAL(NULL);
	s_bt_state_management.state = BT_STATE_NONE;
	s_bt_state_management.expected_state = BT_STATE_NONE;
	s_bt_state_management.hci_enabled = 1;
}

void bt_power_done()
{
	s_bt_state_management.state = BT_STATE_DISABLED;
	s_bt_state_management.expected_state = BT_STATE_DISABLED;
	s_bt_state_management.hci_enabled = 0;
	FREE_CRITICAL(s_bt_state_management.lock);
}

#if 0
void bt_w_crash_event_func()
{
	POWER_MGR_DBG("%s start", __FUNCTION__);
	//PBAP"
	if(hlp_pbap_get_status() == ST_PBAP_GETTING )
	{
		bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+PBDATA=E: Download complete %d", hlp_pbap_get_storage() != 1 ? 2 : 1);
	}
	bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+PBSTAT=1");
	//HFP
	bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+HFPSTAT=1");
	//A2DP
	bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+A2DPSTAT=1");
	//AVRCP
	bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+AVRCPSTAT=1");
	//GAP
	bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+DEVSTAT=0");

	s_bt_state_management.state = BT_STATE_DISABLED;

	POWER_MGR_DBG("%s start", __FUNCTION__);
}
#endif

void bt_enable_bluetooth()
{
	if (s_bt_state_management.state == BT_STATE_ENABLED) {
		bt_event_indicate(BT_EV_APP_PWR_STAT, (void *)1);
		bt_event_indicate(BT_EV_APP_VERSION, NULL);
		bt_event_indicate(BT_EV_SET_PROFILE_CONFIG, (void *)1192);
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)1);
		return ;
	}
	bt_event_indicate(BT_EV_EVENT_OK, NULL);
	/*if (s_bt_state_management.state != BT_STATE_NONE && s_bt_state_management.state != BT_STATE_DISABLED) {
		return;
	}*/
	//treate BT_STATE_NONE as power off, wait stack done complete!!! add by cjs
	if (s_bt_state_management.state != BT_STATE_NONE/* && s_bt_state_management.state != BT_STATE_DISABLED*/) {
		return;
	}

	on_bluetooth_state_changed(BT_STATE_ENABLING);
}

void bt_enable_bluetooth_ex()
{
	if (s_bt_state_management.state == BT_STATE_ENABLED) {
		bt_event_indicate(BT_EV_APP_PWR_STAT, (void *)1);
		bt_event_indicate(BT_EV_APP_VERSION, NULL);
		bt_event_indicate(BT_EV_SET_PROFILE_CONFIG, (void *)1192);
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)1);
		return ;
	}
	bt_event_indicate(BT_EV_EVENT_OK, NULL);
	/*if (s_bt_state_management.state != BT_STATE_NONE && s_bt_state_management.state != BT_STATE_DISABLED) {
		return;
	}*/
	//treate BT_STATE_NONE as power off, wait stack done complete!!! add by cjs
	if (s_bt_state_management.state != BT_STATE_NONE/* && s_bt_state_management.state != BT_STATE_DISABLED*/) {
		return;
	}

	on_bluetooth_state_changed(BT_STATE_ENABLING_EX);
}

int is_bluetooth_enabled()
{
	return s_bt_state_management.state == BT_STATE_ENABLED ? TRUE : FALSE;
}

void bt_time_disable_bluetooth()
{
	on_bluetooth_state_changed(BT_STATE_DISABLING);
}

void bt_disable_bluetooth()
{
	app_gap_scan_mode_control(BT_SET_SCAN_MODE_TYPE_SCAN, TRUE);
	printf("s_bt_state_management.state = %d\n", s_bt_state_management.state);
	if (s_bt_state_management.state == BT_STATE_NONE) {
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)0);
		return;
	}

	if (s_bt_state_management.state != BT_STATE_ENABLED && s_bt_state_management.state != BT_STATE_ENABLING_EX) {
		return;
	}
	g_bt_opened = FALSE;
	if(hlp_hf_get_status() == ST_HF_CONNECTING)
	{
		FsmAddTimer(NULL, 2000, 0, NULL, NULL, (UINT32)bt_time_disable_bluetooth);//1s
	} else {
		on_bluetooth_state_changed(BT_STATE_DISABLING);
	}
}

void bt_get_bluetooth_state()
{
    if (s_bt_state_management.state == BT_STATE_ENABLED) {
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)1);
	}else{
        bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)0);
	}
}
#if 0
static void bt_local_info_init()
{
	bt_dev_get_pairied_device();
	bt_dev_get_local_bd();
#ifdef CONFIG_MGR_DYNAMIC
	bt_sync_profile_state();
#endif
	char name[1];
	memset(name, 0, 1);
	bt_dev_operate_local_name(name);
}
#endif
int bt_get_power_state()
{
	if(s_bt_state_management.state == BT_STATE_ENABLED)
	{
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)1);
		return 1;
	} else {
		bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, 0);
              return 0;
        }
}

void check_power_on_thread(void *param)
{
    
    POWER_MGR_DBG("start check bt state!");
    //bt_enable_bluetooth();
    //usleep(6000*1000);//sleep 5s
    if (is_bluetooth_enabled())
    {
        POWER_MGR_DBG("%s, open bt success", __FUNCTION__);
        return;
    }
    POWER_MGR_DBG("%s, open bt fail,restart brlinkd", __FUNCTION__);
    exit(0);
}

static void on_bluetooth_state_changed(int state)
{
	int i;
	int changed = 1;
	POWER_MGR_DBG("%s(%s -> %s).", __FUNCTION__,
	              get_state_string(s_bt_state_management.state), get_state_string(state));

	if (s_bt_state_management.state == state) {
		POWER_MGR_DBG("%s the same state. return!", __FUNCTION__);
		return;
	}

	ENTER_CRITICAL(s_bt_state_management.lock);

	if (state == BT_STATE_ENABLED) {
		//bt_local_info_init();
#ifdef CONFIG_CHECK_LICENSE
		FsmAddTimer(NULL, 0, 0, NULL, NULL, (UINT32)app_start_check_license);
#endif
	}

	for(i=0; i<BT_STATE_COUNT; i++) {
		if (s_sm[i].state_transmit_to == state) {
			if (s_bt_state_management.state != s_sm[i].state) {
				changed = 0;
			}
			break;
		}
	}

	POWER_MGR_DBG("changed %d.", changed);
	if (s_bt_state_management.state == BT_STATE_DISABLED && state == BT_STATE_NONE) {
		s_bt_state_management.state = state;
		POWER_MGR_DBG("power off end.");
		LEAVE_CRITICAL(s_bt_state_management.lock);
		return;
	}
        
	if (changed) {
		s_bt_state_management.state = state;
		LEAVE_CRITICAL(s_bt_state_management.lock);
	} else {
		LEAVE_CRITICAL(s_bt_state_management.lock);
		return;
	}

	for(i=0; i<BT_STATE_COUNT; i++) {
		if (s_sm[i].state == state && s_sm[i].action != NULL) {
			s_sm[i].action();
			break;
		}
	}
}

UINT8 bt_bluetooth_is_enabled()
{
	return s_bt_state_management.hci_enabled;
}

UINT8 bt_bluetooth_is_disabling()
{
	return s_bt_state_management.is_disabling;
}

static void bt_enable_timeout_callback(void *arg)
{
	int state = BT_STATE_NONE;
	POWER_MGR_DBG("%s,expected state:%s.", __FUNCTION__, get_state_string(state));
	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.expected_state = BT_STATE_NONE;
	s_bt_state_management.wait_for_timer = NULL;
	LEAVE_CRITICAL(s_bt_state_management.lock);

	//If time out , change state to BT_STATE_NONE
	on_bluetooth_state_changed(state);
}
static void bt_disable_timeout_callback(void *arg)
{
	int state = s_bt_state_management.expected_state;
	POWER_MGR_DBG("%s,expected state:%s.", __FUNCTION__, get_state_string(state));

	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.expected_state = BT_STATE_NONE;
	s_bt_state_management.wait_for_timer = NULL;
	LEAVE_CRITICAL(s_bt_state_management.lock);

	// Continue to change the expected state
	on_bluetooth_state_changed(state);
}

static void bt_disable_guard_state(int state, int timeout)
{
	bt_guard_state(state, timeout, bt_disable_timeout_callback);
}

static void bt_guard_state(int state, int timeout, UserTimerFunc *func)
{
	struct FsmTimer *timer = FsmAddTimer(NULL, timeout, 0, NULL, "bt_guard", (UINT32) func);

	POWER_MGR_DBG("%s(%s, %d).", __FUNCTION__, get_state_string(state), timeout);

	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.expected_state = state;
	s_bt_state_management.wait_for_timer = timer;
	LEAVE_CRITICAL(s_bt_state_management.lock);
}

static void bt_disable_delete_timer_if_exist()
{
	struct FsmTimer *timer = NULL;

	ENTER_CRITICAL(s_bt_state_management.lock);
	POWER_MGR_DBG("%s(expected_state %s current %s).", __FUNCTION__, get_state_string(s_bt_state_management.expected_state), get_state_string(s_bt_state_management.state));
	if (s_bt_state_management.state != BT_STATE_NONE) {//fixme, if BT_STATE_NONE need delete timer
		s_bt_state_management.expected_state = BT_STATE_NONE;
		timer = s_bt_state_management.wait_for_timer;
		s_bt_state_management.wait_for_timer = NULL;
	}
	LEAVE_CRITICAL(s_bt_state_management.lock);

	if (timer != NULL) {
		POWER_MGR_DBG("%s(timer %08x).", __FUNCTION__, timer);
		FSM_DelTimer(&timer);
	}
}

static void bt_action_on_bluetooth_enabling()
{
	s_bt_state_management.hci_enabled = 1;

	POWER_MGR_DBG("app_tl_init");
	app_tl_init();
	bt_init();
	POWER_MGR_DBG("bt_init");
	//FsmAddTimer(NULL, 5000, 0, NULL, "reopen", (UINT32) check_power_on_thread);
}
#if 0
static void bt_action_on_bluetooth_enabling_ex()
{
	s_bt_state_management.hci_enabled = 1;

	//POWER_MGR_DBG("app_tl_init");
	//app_tl_init();
	bt_init();
	POWER_MGR_DBG("bt_init");
	//FsmAddTimer(NULL, 5000, 0, NULL, "reopen", (UINT32) check_power_on_thread);
}
#endif
static void bt_action_on_bluetooth_disabling()
{
	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.is_disabling = 1;
	LEAVE_CRITICAL(s_bt_state_management.lock);

	hlp_gap_set_visiability(0);
	// stop inquiry
	if (app_gap_is_discovering() == TRUE) {
		app_gap_cancel_discovery();
	}
	// wait some ms for complete the ops(set_visibility and cancel_discovery.
	bt_disable_guard_state(BT_STATE_DISABLING_HFP, CANCAL_DISCOVERY_AND_SET_INVISIABLE_TIMEOUT);
}

void bt_action_on_bluetooth_enabled()
{
	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.is_disabling = 0;
	LEAVE_CRITICAL(s_bt_state_management.lock);

	bt_disable_delete_timer_if_exist();
	hlp_gap_set_visiability(1);
	g_bt_opened = TRUE;
	bt_event_indicate(BT_EV_APP_PWR_STAT, (void *)1);
	bt_event_indicate(BT_EV_APP_VERSION, NULL);
	bt_event_indicate(BT_EV_SET_PROFILE_CONFIG, (void *)65535);
	bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)1);
	bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, (void *)3);
	app_ancs_start();
	//g_bt_opened = TRUE;
	
}

static void bt_action_on_bluetooth_disabled()
{
    POWER_MGR_DBG("%s", __FUNCTION__);
	ENTER_CRITICAL(s_bt_state_management.lock);
	s_bt_state_management.is_disabling = 0;
	LEAVE_CRITICAL(s_bt_state_management.lock);

	request_bt_done();
}

/* Make sure the HFP connection is disconnected during bt_disable_bluetooth */
static void bt_disable_hfp()
{
	bt_disable_delete_timer_if_exist();

	if (hlp_hf_get_status() >= ST_HF_CONNECTING) {
		bt_disable_guard_state(BT_STATE_DISABLING_A2DP, DISCONNECT_PROFILE_TIMEOUT);
		app_hf_disconnect();
	} else {
		on_bluetooth_state_changed(BT_STATE_DISABLING_A2DP);
	}
}

/* Make sure the A2DP connection is disconnected during bt_disable_bluetooth */
static void bt_disable_a2dp()
{
	bt_disable_delete_timer_if_exist();

	if (hlp_a2dp_get_status() >= ST_A2DP_CONNECTING) {
		bt_disable_guard_state(BT_STATE_DISABLING_AVRCP, DISCONNECT_PROFILE_TIMEOUT);
		/* this function is called when user is turnning off BT. The State of BT is not in STATE_ON.
		 * The service is not try to disconnect A2DP at this time.  */
		//if (!bt_manager_hfp_a2dp_is_binded()) {
			app_a2dp_disconnect();
		//}
	} else {
		on_bluetooth_state_changed(BT_STATE_DISABLING_AVRCP);
	}
}

/* Make sure the AVRCP connection is disconnected during bt_disable_bluetooth */
static void bt_disable_avrcp()
{
	bt_disable_delete_timer_if_exist();

#ifdef CONFIG_AVRCP
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTING) {
		bt_disable_guard_state(BT_STATE_DISABLING_PBAP, DISCONNECT_PROFILE_TIMEOUT);
        app_avrcp_disconnect();
	} else
#endif
	{
		on_bluetooth_state_changed(BT_STATE_DISABLING_PBAP);
	}
}

/* Make sure the PBAP connection is disconnected during bt_disable_bluetooth */
static void bt_disable_pbap()
{
	bt_disable_delete_timer_if_exist();

#ifdef CONFIG_PBAP
	if(hlp_pbap_get_handle() != NULL) {
		bt_disable_guard_state(BT_STATE_DISABLING_GATT, DISCONNECT_PROFILE_TIMEOUT);
		app_pbap_stop();
	} else
#endif
	{
		on_bluetooth_state_changed(BT_STATE_DISABLING_GATT);
	}
}

static void bt_disable_gatt()
{
	int conn = 0;
	bt_disable_delete_timer_if_exist();
#ifdef CONFIG_GATT
	// FIXME
	conn = app_gatt_client_done();
#endif

	if (conn) {
		bt_disable_guard_state(BT_STATE_DISABLING_ACL, DISCONNECT_PROFILE_TIMEOUT);
	} else {
		on_bluetooth_state_changed(BT_STATE_DISABLING_ACL);
	}
}

static void bt_disable_acl()
{
	bt_disable_delete_timer_if_exist();

	if (hlp_gap_disconnect_acl_if_exist()) {
		bt_disable_guard_state(BT_STATE_DISABLING_HCI, DISCONNECT_PROFILE_TIMEOUT);
	} else {
		on_bluetooth_state_changed(BT_STATE_DISABLING_HCI);
	}
}

/* Make sure the HCI data path is closed during bt_disable_bluetooth */
static void bt_disable_hci()
{
	bt_disable_delete_timer_if_exist();

	s_bt_state_management.hci_enabled = 0;
	on_bluetooth_state_changed(BT_STATE_DISABLED);
}

static void on_bt_done_finished(BOOL result)
{
	POWER_MGR_DBG("%s", __FUNCTION__);
        
	on_bluetooth_state_changed(BT_STATE_NONE);        
	bt_event_indicate(BT_EV_OTHER_LOCAL_STATE, 0);
#ifndef BT_POWER_CONFIG
	app_tl_done();
#endif
	g_bt_opened = FALSE;
#ifdef CONFIG_MEMORY_LEAK_DETECT
	mem_info_out(1);
#endif
	BTOSDepDone();
}
static void request_bt_done_callback(void *arg)
{
	POWER_MGR_DBG("%s", __FUNCTION__);
	bt_done(on_bt_done_finished);
}

static void request_bt_done()
{
	//FIXME, done stack after disconnect complete
	struct FsmTimer * ft;
	ft = FsmAddTimer(NULL, 1000, 0, NULL, "btdone", (UINT32) request_bt_done_callback);
	POWER_MGR_DBG("request_bt_done ft=0x%04x", ft); 
}

// hfp -> a2dp -> avrcp -> pbap -> gatt -> map -> dun -> pan -> ftp
void on_profile_state_changed(int profile, int state)
{
	POWER_MGR_DBG("%s(%d, %d).", __FUNCTION__, profile, state);
	// on_bluetooth_state_changed(BT_STATE_ENABLED);
	// -> on_profile_state_changed(PROFILE_ID_GAP, PROFILE_STATE_CONNECTED);
	if (state == PROFILE_STATE_CONNECTED) {
		switch (profile) {
		case PROFILE_ID_GAP:
			on_bluetooth_state_changed(BT_STATE_ENABLED);
			break;
		}
	} else if (state == PROFILE_STATE_DISCONNECTED) {
#ifdef CONFIG_MGR_DYNAMIC
		bt_manager_profile_disconnect_ind(profile);
#endif
		switch (profile) {
		case PROFILE_ID_HFP:
			on_bluetooth_state_changed(BT_STATE_DISABLING_A2DP);
			break;
		case PROFILE_ID_A2DP:
			on_bluetooth_state_changed(BT_STATE_DISABLING_AVRCP);
			break;
		case PROFILE_ID_AVRCP:
			on_bluetooth_state_changed(BT_STATE_DISABLING_PBAP);  //lkz debug
			break;
		case PROFILE_ID_PBAP:
			on_bluetooth_state_changed(BT_STATE_DISABLING_GATT);
			break;
		case PROFILE_ID_GATT:
			on_bluetooth_state_changed(BT_STATE_DISABLING_ACL);
			break;
		case PROFILE_ID_ACL:
			on_bluetooth_state_changed(BT_STATE_DISABLING_HCI);
			break;
		}
	}
}


BOOL get_g_bt_opened_status()
{
	return g_bt_opened;
}