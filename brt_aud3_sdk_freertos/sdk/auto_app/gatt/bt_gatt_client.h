/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef ANDROID_INCLUDE_BT_GATT_CLIENT_H
#define ANDROID_INCLUDE_BT_GATT_CLIENT_H

//#include "bt_bluedroid_types.h"
#include "bt_gatt_types.h"
#include "bt_common_types.h"


/**
 * Buffer sizes for maximum attribute length and maximum read/write
 * operation buffer size.
 */
#define BTGATT_MAX_ATTR_LEN 600

/** Buffer type for unformatted reads/writes */
typedef struct {
	uint8_t             value[BTGATT_MAX_ATTR_LEN];
	uint16_t            len;
} btgatt_unformatted_value_t;

/** Parameters for GATT read operations */
typedef struct {
	uint16_t			handle;
	btgatt_unformatted_value_t value;
	uint16_t            value_type;
	uint8_t             status;
} btgatt_read_params_t;

/** Parameters for GATT write operations */
typedef struct {
	btgatt_srvc_id_t    srvc_id;
	btgatt_gatt_id_t    char_id;
	btgatt_gatt_id_t    descr_id;
	uint8_t             status;
} btgatt_write_params_t;

/** Attribute change notification parameters */
typedef struct {
	uint8_t             value[BTGATT_MAX_ATTR_LEN];
	bt_bdaddr_t         bda;
	uint16_t            handle;
	uint16_t            len;
	uint8_t             is_notify;
} btgatt_notify_params_t;

typedef struct {
	uint8_t  client_if;
	uint8_t  action;
	uint8_t  filt_index;
	uint16_t feat_seln;
	uint16_t list_logic_type;
	uint8_t  filt_logic_type;
	uint8_t  rssi_high_thres;
	uint8_t  rssi_low_thres;
	uint8_t  dely_mode;
	uint16_t found_timeout;
	uint16_t lost_timeout;
	uint8_t  found_timeout_cnt;
	uint16_t  num_of_tracking_entries;
} btgatt_filt_param_setup_t;

typedef struct {
	bt_bdaddr_t        *bda1;
	bt_uuid_t          *uuid1;
	uint16_t            u1;
	uint16_t            u2;
	uint16_t            u3;
	uint16_t            u4;
	uint16_t            u5;
} btgatt_test_params_t;

/* BT GATT client error codes */
typedef enum {
    BT_GATTC_COMMAND_SUCCESS = 0,    /* 0  Command succeeded                 */
    BT_GATTC_COMMAND_STARTED,        /* 1  Command started OK.               */
    BT_GATTC_COMMAND_BUSY,           /* 2  Device busy with another command  */
    BT_GATTC_COMMAND_STORED,         /* 3 request is stored in control block */
    BT_GATTC_NO_RESOURCES,           /* 4  No resources to issue command     */
    BT_GATTC_MODE_UNSUPPORTED,       /* 5  Request for 1 or more unsupported modes */
    BT_GATTC_ILLEGAL_VALUE,          /* 6  Illegal command /parameter value  */
    BT_GATTC_INCORRECT_STATE,        /* 7  Device in wrong state for request  */
    BT_GATTC_UNKNOWN_ADDR,           /* 8  Unknown remote BD address         */
    BT_GATTC_DEVICE_TIMEOUT,         /* 9  Device timeout                    */
    BT_GATTC_INVALID_CONTROLLER_OUTPUT,/* 10  An incorrect value was received from HCI */
    BT_GATTC_SECURITY_ERROR,          /* 11 Authorization or security failure or not authorized  */
    BT_GATTC_DELAYED_ENCRYPTION_CHECK, /*12 Delayed encryption check */
    BT_GATTC_ERR_PROCESSING           /* 12 Generic error                     */
} btgattc_error_t;

typedef void (*client_registered_callback)(uint8_t status, uint8_t client_if, bt_uuid_t uuid);

typedef void (*scanner_registered_callback)(uint8_t status, int scan_id, bt_uuid_t uuid);

typedef void (*scan_result_callback)(uint8_t event_type, uint8_t address_type, bt_bdaddr_t address, uint8_t primary_phy, uint8_t secondary_phy, uint8_t advertising_sid, uint8_t tx_power, uint8_t rssi, uint16_t periodic_adv_int, uint16_t adv_data_len, uint8_t * adv_data);

typedef void (*connected_callback)(uint8_t client_if, uint16_t conn_id, uint8_t status, bt_bdaddr_t address);

typedef void (*disconnected_callback)(uint8_t client_if, uint16_t conn_id, uint8_t status, bt_bdaddr_t address);

typedef void (*read_characteristic_callback)(uint16_t conn_id, uint8_t status, uint16_t handle, uint16_t data_len, uint8_t * data);

typedef void (*write_characteristic_callback)(uint16_t conn_id, uint8_t status, uint16_t handle);

typedef void (*execute_completed_callback)(uint16_t conn_id, uint8_t status);

typedef void (*search_completed_callback)(uint16_t conn_id, uint8_t status);

typedef void (*read_descriptor_callback)(uint16_t conn_id, uint8_t status, uint16_t handle, uint16_t data_len, uint8_t * data);

typedef void (*write_descriptor_callback)(uint16_t conn_id, uint8_t status, uint16_t handle);

typedef void (*notify_callback)(uint16_t conn_id, bt_bdaddr_t address, uint16_t handle, uint8_t is_notify, uint16_t data_len, uint8_t * data);

typedef void (*register_for_notifications_callback)(uint16_t conn_id, uint8_t status, uint8_t registered, uint16_t handle);

typedef void (*read_remote_rssi_callback)(uint8_t client_if, bt_bdaddr_t address, uint8_t rssi, uint8_t status);

typedef void (*configure_mtu_callback)(uint16_t conn_id, uint8_t status, int mtu);

typedef void (*scan_filter_config_callback)(uint8_t action, uint8_t status, uint8_t client_if, uint8_t filter_type, int available_space);

typedef void (*scan_filter_params_configured_callback)(uint8_t action, uint8_t status, uint8_t client_if, int available_space);

typedef void (*scan_filter_enable_disabled_callback)(uint8_t action, uint8_t status, uint8_t client_if);

typedef void (*client_congestion_callback)(uint16_t conn_id, uint8_t congested);

typedef void (*batch_scan_storage_configured_callback)(uint8_t status, uint8_t client_if);

typedef void (*batch_scan_start_stopped_callback)(uint8_t action, uint8_t status, uint8_t client_if);

typedef void (*batch_scan_reports_callback)(uint8_t status, int scanner_id, uint8_t report_type, int num_records, uint16_t record_data_len, uint8_t * record_data);

typedef void (*batch_scan_threshold_crossed_callback)(uint8_t client_if);

typedef void (*track_adv_found_lost_callback)(uint8_t client_if, uint16_t adv_pkt_len, uint8_t * adv_pkt, uint16_t scan_rsp_len, uint8_t * scan_rsp, uint8_t filt_index, uint8_t adv_state, uint8_t adv_info_present, bt_bdaddr_t address, uint8_t addr_type, uint8_t tx_power, uint16_t rssi_value, uint16_t time_stamp);

typedef void (*scan_param_setup_completed_callback)(uint8_t status, int scanner_id);

typedef void (*get_gatt_db_callback)(uint16_t conn_id, uint8_t is_completed, uint16_t svc_data_len, uint8_t * svc_data);

typedef void (*client_phy_read_callback)(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status);

typedef void (*client_phy_update_callback)(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status);

typedef void (*client_conn_update_callback)(uint16_t conn_id, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status);

/** BT-GATT Client callback structure. */

typedef struct {
	client_registered_callback						client_registered;
	scanner_registered_callback						scanner_registered;
	scan_result_callback							scan_result;
	connected_callback								connected;
	disconnected_callback							disconnected;
	read_characteristic_callback					read_characteristic;
	write_characteristic_callback					write_characteristic;
	execute_completed_callback						execute_completed;
	search_completed_callback						search_completed;
	read_descriptor_callback						read_descriptor;
	write_descriptor_callback						write_descriptor;
	notify_callback									notify;
	register_for_notifications_callback				register_for_notifications;
	read_remote_rssi_callback						read_remote_rssi;
	configure_mtu_callback							configure_mtu;
	scan_filter_config_callback						scan_filter_config;
	scan_filter_params_configured_callback			scan_filter_params_configured;
	scan_filter_enable_disabled_callback			scan_filter_enable_disabled;
	client_congestion_callback						client_congestion;
	batch_scan_storage_configured_callback			batch_scan_storage_configured;
	batch_scan_start_stopped_callback				batch_scan_start_stopped;
	batch_scan_reports_callback						batch_scan_reports;
	batch_scan_threshold_crossed_callback			batch_scan_threshold_crossed;
	track_adv_found_lost_callback					track_adv_found_lost;
	scan_param_setup_completed_callback				scan_param_setup_completed;
	get_gatt_db_callback							get_gatt_db;
	client_phy_read_callback						client_phy_read;
	client_phy_update_callback						client_phy_update;
	client_conn_update_callback						client_conn_update;
} btgatt_client_callbacks_t;


#endif /* ANDROID_INCLUDE_BT_GATT_CLIENT_H */
