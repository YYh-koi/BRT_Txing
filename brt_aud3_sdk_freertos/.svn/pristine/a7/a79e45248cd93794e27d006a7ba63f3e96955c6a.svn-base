/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		bt_power_manager.h
*
*	Description:
*
*
*	Create Date:		2014/11/05
***********************************************************************/

#ifndef _BT_POWER_MANAGER_H__
#define _BT_POWER_MANAGER_H__

#include "btdatatype.h"
#include "fsm.h"

enum tag_profile_state {
    PROFILE_STATE_DISCONNECTED,
    PROFILE_STATE_CONNECTED
};

void bt_power_init();
void bt_power_done();
void bt_enable_bluetooth();
void bt_enable_bluetooth_ex();
void bt_disable_bluetooth();
void on_profile_state_changed(int profile, int state);
UINT8 bt_bluetooth_is_enabled();
UINT8 bt_bluetooth_is_disabling();
BOOL get_g_bt_opened_status();
void bt_w_crash_event_func();

static void on_bluetooth_state_changed(int state);
static void bt_disable_delete_timer_if_exist();
static void bt_guard_state(int state, int timeout, UserTimerFunc *func);
static void bt_disable_guard_state(int state, int timeout);

static void bt_action_on_bluetooth_enabling();
static void bt_action_on_bluetooth_enabling_ex();
static void bt_action_on_bluetooth_disabling();
void bt_action_on_bluetooth_enabled();
static void bt_action_on_bluetooth_disabled();
static void bt_disable_hfp();
static void bt_disable_a2dp();
static void bt_disable_avrcp();
static void bt_disable_pbap();
static void bt_disable_gatt();
static void bt_disable_acl();
static void bt_disable_hci();
static void request_bt_done();

#endif /* _BT_POWER_MANAGER_H__ */

