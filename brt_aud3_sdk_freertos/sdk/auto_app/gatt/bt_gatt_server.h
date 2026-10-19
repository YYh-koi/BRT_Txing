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


#ifndef ANDROID_INCLUDE_BT_GATT_SERVER_H
#define ANDROID_INCLUDE_BT_GATT_SERVER_H

//#include "bt_bluedroid_types.h"

#include "bt_gatt_types.h"

#include "bt_common_types.h"


#define BTGATT_MAX_ATTR_LEN 600

/** GATT value type used in response to remote read requests */
typedef struct {
	uint8_t           value[BTGATT_MAX_ATTR_LEN];
	uint16_t          handle;
	uint16_t          offset;
	uint16_t          len;
	uint8_t           auth_req;
} btgatt_value_t;

/** GATT remote read request response type */
typedef union {
	btgatt_value_t attr_value;
	uint16_t            handle;
} btgatt_response_t;

typedef void (*server_registered_callback)(uint8_t status, uint8_t server_if, bt_uuid_t uuid);

typedef void (*client_connected_callback)(bt_bdaddr_t address, uint8_t connected, uint16_t conn_id, uint8_t server_if);

typedef void (*service_added_callback)(btgatt_db_element_t * element);//uint8_t status, uint8_t server_if, uint16_t srv_handle,uint8_t is_completed, uint16_t svc_data_len, uint8_t * svc_data);

typedef void (*service_stopped_callback)(uint8_t status, uint8_t server_if, uint16_t srvc_handle);

typedef void (*service_deleted_callback)(uint8_t status, uint8_t server_if, uint16_t srvc_handle);

typedef void (*response_send_completed_callback)(uint8_t status, uint16_t attr_handle);

typedef void (*server_read_characteristic_callback)(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, uint8_t is_long);

typedef void (*server_read_descriptor_callback)(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, uint8_t is_long);

typedef void (*server_write_characteristic_callback)(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, int length, uint8_t need_rsp, uint8_t is_prep, uint16_t data_len, uint8_t * data);

typedef void (*server_write_descriptor_callback)(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, int length, uint8_t need_rsp, uint8_t is_prep, uint16_t data_len, uint8_t * data);

typedef void (*execute_write_callback)(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint8_t exec_write);

typedef void (*notification_sent_callback)(uint16_t conn_id, uint8_t status);

typedef void (*server_congestion_callback)(uint16_t conn_id, uint8_t congested);

typedef void (*server_mtu_changed_callback)(uint16_t conn_id, int mtu);

typedef void (*server_phy_read_callback)(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status);

typedef void (*server_phy_update_callback)(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status);

typedef void (*server_conn_update_callback)(uint16_t conn_id, int interval, int latency, int timeout, uint8_t status);

typedef void (*advertising_set_started_callback)(int reg_id, int advertiser_id, uint8_t tx_power, uint8_t status);

typedef void (*own_address_read_callback)(int advertiser_id, uint8_t address_type, bt_bdaddr_t address);

typedef void (*advertising_enabled_callback)(int advertiser_id, uint8_t enable, uint8_t status);

typedef void (*advertising_data_set_callback)(int advertiser_id, uint8_t status);

typedef void (*scan_response_data_set_callback)(int advertiser_id, uint8_t status);

typedef void (*advertising_parameters_updated_callback)(int advertiser_id, uint8_t tx_power, uint8_t status);

typedef void (*periodic_advertising_parameters_updated_callback)(int advertiser_id, uint8_t status);

typedef void (*periodic_advertising_data_set_callback)(int advertiser_id, uint8_t status);

typedef void (*periodic_advertising_enabled_callback)(int advertiser_id, uint8_t enable, uint8_t status);

typedef void (*sync_lost_callback)(uint16_t sync_handle);

typedef void (*sync_report_callback)(uint16_t conn_id, uint8_t tx_power, uint8_t rssi, uint8_t data_status, uint16_t data_len, uint8_t * data);

typedef void (*sync_started_callback)(uint8_t reg_id, uint8_t status, uint16_t sync_handle, uint8_t sid, uint8_t address_type, bt_bdaddr_t address, uint8_t phy, uint16_t interval);


typedef struct {
	server_registered_callback									server_registered;
	client_connected_callback									client_connected;
	service_added_callback										service_added;
	service_stopped_callback									service_stopped;
	service_deleted_callback									service_deleted;
	response_send_completed_callback							response_send_completed;
	server_read_characteristic_callback							server_read_characteristic;
	server_read_descriptor_callback								server_read_descriptor;
	server_write_characteristic_callback						server_write_characteristic;
	server_write_descriptor_callback							server_write_descriptor;
	execute_write_callback										execute_write;
	notification_sent_callback									notification_sent;
	server_congestion_callback									server_congestion;
	server_mtu_changed_callback									server_mtu_changed;
	server_phy_read_callback									server_phy_read;
	server_phy_update_callback									server_phy_update;
	server_conn_update_callback									server_conn_update;
	advertising_set_started_callback							advertising_set_started;
	own_address_read_callback									own_address_read;
	advertising_enabled_callback								advertising_enabled;
	advertising_data_set_callback								advertising_data_set;
	scan_response_data_set_callback								scan_response_data_set;
	advertising_parameters_updated_callback						advertising_parameters_updated;
	periodic_advertising_parameters_updated_callback			periodic_advertising_parameters_updated;
	periodic_advertising_data_set_callback						periodic_advertising_data_set;
	periodic_advertising_enabled_callback						periodic_advertising_enabled;
	sync_lost_callback											sync_lost;
	sync_report_callback										sync_report;
	sync_started_callback										sync_started;
} btgatt_server_callbacks_t;

#endif /* ANDROID_INCLUDE_BT_GATT_CLIENT_H */
