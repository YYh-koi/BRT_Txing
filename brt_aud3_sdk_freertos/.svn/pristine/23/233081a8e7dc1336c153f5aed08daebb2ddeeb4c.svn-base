/**
  * This file defines functions to execute the gatt procedure is Stack task thread.
  */

#ifndef _APP_GATT_FSM_H__
#define _APP_GATT_FSM_H__

#include "bt_gatt.h"
#include "bt_gatt_types.h"
#include "btdatatype.h"

enum {
	FSM_EV_CMD_GATT_TEST,
	FSM_EV_CMD_GATT_CLIENT_GET_DEVICE_TYPE,
	FSM_EV_CMD_GATT_CLIENT_REGISTER_APP,
	FSM_EV_CMD_GATT_CLIENT_UNREGISTER_APP,
	FSM_EV_CMD_GATT_CLIENT_CONNECT,
	FSM_EV_CMD_GATT_CLIENT_DISCONNECT,
	FSM_EV_CMD_GATT_CLIENT_SET_PREFERRED_PHY,
	FSM_EV_CMD_GATT_CLIENT_READ_PHY,
	FSM_EV_CMD_GATT_CLIENT_REFRESH,
	FSM_EV_CMD_GATT_CLIENT_SEARCH_SERVICE,
	FSM_EV_CMD_GATT_CLIENT_DISCOVER_SERVICE_BY_UUID,
	FSM_EV_CMD_GATT_CLIENT_GET_GATT_DB,
	FSM_EV_CMD_GATT_CLIENT_READ_CHARACTERISTIC,
	FSM_EV_CMD_GATT_CLIENT_READ_USING_CHARACTERISTIC_UUID,
	FSM_EV_CMD_GATT_CLIENT_READ_DESCRIPTOR,
	FSM_EV_CMD_GATT_CLIENT_WRITE_CHARACTERISTIC,
	FSM_EV_CMD_GATT_CLIENT_WRITE_DESCRIPTOR,
	FSM_EV_CMD_GATT_CLIENT_EXECUTE_WRITE,
	FSM_EV_CMD_GATT_CLIENT_REGISTER_FOR_NOTIFICATIONS,
	FSM_EV_CMD_GATT_CLIENT_READ_REMOTE_RSSI,
	FSM_EV_CMD_GATT_CLIENT_CONFIGURE_MTU,
	FSM_EV_CMD_GATT_CONNECTION_PARAMETER_UPDATE,
	FSM_EV_CMD_GATT_SERVER_REGISTER_APP,
	FSM_EV_CMD_GATT_SERVER_UNREGISTER_APP,
	FSM_EV_CMD_GATT_SERVER_CONNECT,
	FSM_EV_CMD_GATT_SERVER_DISCONNECT,
	FSM_EV_CMD_GATT_SERVER_SET_PREFERRED_PHY,
	FSM_EV_CMD_GATT_SERVER_READ_PHY,
	FSM_EV_CMD_GATT_SERVER_ADD_SERVICE,
	FSM_EV_CMD_GATT_SERVER_STOP_SERVICE,
	FSM_EV_CMD_GATT_SERVER_DELETE_SERVICE,
	FSM_EV_CMD_GATT_SERVER_SEND_INDICATION,
	FSM_EV_CMD_GATT_SERVER_SEND_NOTIFICATION,
	FSM_EV_CMD_GATT_SERVER_SEND_RESPONSE,
	FSM_EV_CMD_GATT_ADVM_START_ADVERTISING_SET,
	FSM_EV_CMD_GATT_ADVM_GETOWNADDRESS,
	FSM_EV_CMD_GATT_ADVM_STOPADVERTISINGSET,
	FSM_EV_CMD_GATT_ADVM_ENABLEADVERTISINGSET,
	FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_DATA,
	FSM_EV_CMD_GATT_ADVM_SET_SCAN_RESPONSE_DATA,
	FSM_EV_CMD_GATT_ADVM_SET_ADVERTISING_PARAMETERS,
	FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_PARAMETERS,
	FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_DATA,
	FSM_EV_CMD_GATT_ADVM_SET_PERIODIC_ADVERTISING_ENABLE,
	FSM_EV_CMD_GATT_SCAN_PERIODIC_STARTSYNC,
	FSM_EV_CMD_GATT_SCAN_PERIODIC_STOPSYNC,
	FSM_EV_CMD_GATT_SCAN_REGISTER_SCANNER,
	FSM_EV_CMD_GATT_SCAN_UNREGISTER_SCANNER,
	FSM_EV_CMD_GATT_SCAN_CLIENT_SCAN,
	FSM_EV_CMD_GATT_SCAN_SET_SCAN_PARAMETERS,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ADD,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_DELETE,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_ADD,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_CLEAR_ALL,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_DELETE,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_CLEAR,
	FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ENABLE,
	FSM_EV_CMD_GATT_CLIENT_CONFIG_BATCH_SCAN_STORAGE,
	FSM_EV_CMD_GATT_CLIENT_START_BATCH_SCAN,
	FSM_EV_CMD_GATT_CLIENT_STOP_BATCH_SCAN,
	FSM_EV_CMD_GATT_CLIENT_READ_SCAN_REPORTS,
};

typedef struct {
	int cmd;
} fsm_msg_gatt_test_t;

typedef struct {
	bt_bdaddr_t address;
} fsm_msg_gatt_client_get_device_type_t;

typedef struct {
	bt_uuid_t uuid;
} fsm_msg_gatt_client_register_app_t;

typedef struct {
	uint8_t client_if;
} fsm_msg_gatt_client_unregister_app_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
	uint8_t is_direct;
	int transport;
	uint8_t opportunistic;
	int initiating_phys;
} fsm_msg_gatt_client_connect_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
	uint16_t conn_id;
} fsm_msg_gatt_client_disconnect_t;

typedef struct {
	uint8_t client_if;
	uint16_t conn_id;
	uint8_t tx_phy;
	uint8_t rx_phy;
	uint8_t phy_options;
} fsm_msg_gatt_client_set_preferred_phy_t;

typedef struct {
	uint8_t client_if;
	uint16_t conn_id;
} fsm_msg_gatt_client_read_phy_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
} fsm_msg_gatt_client_refresh_t;

typedef struct {
	uint16_t conn_id;
	uint8_t search_all;
	bt_uuid_t service_uuid;
} fsm_msg_gatt_client_search_service_t;

typedef struct {
	uint16_t conn_id;
	bt_uuid_t service_uuid;
} fsm_msg_gatt_client_discover_service_by_uuid_t;

typedef struct {
	uint16_t conn_id;
} fsm_msg_gatt_client_get_gatt_db_t;

typedef struct {
	uint16_t conn_id;
	uint16_t handle;
	uint8_t auth_req;
} fsm_msg_gatt_client_read_characteristic_t;

typedef struct {
	uint16_t conn_id;
	bt_uuid_t uuid;
	int s_handle;
	int e_handle;
	uint8_t auth_req;
} fsm_msg_gatt_client_read_using_characteristic_uuid_t;

typedef struct {
	uint16_t conn_id;
	uint16_t handle;
	uint8_t auth_req;
} fsm_msg_gatt_client_read_descriptor_t;

typedef struct {
	uint16_t conn_id;
	uint16_t handle;
	int write_type;
	uint8_t auth_req;
	uint16_t value_len;
	uint8_t * value;
} fsm_msg_gatt_client_write_characteristic_t;

typedef struct {
	uint16_t conn_id;
	uint16_t handle;
	uint8_t auth_req;
	uint16_t value_len;
	uint8_t * value;
} fsm_msg_gatt_client_write_descriptor_t;

typedef struct {
	uint16_t conn_id;
	uint8_t execute;
} fsm_msg_gatt_client_execute_write_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
	uint16_t handle;
	uint8_t enable;
} fsm_msg_gatt_client_register_for_notifications_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
} fsm_msg_gatt_client_read_remote_rssi_t;

typedef struct {
	uint16_t conn_id;
	int mtu;
} fsm_msg_gatt_client_configure_mtu_t;

typedef struct {
	uint8_t client_if;
	bt_bdaddr_t address;
	int min_interval;
	int max_interval;
	int latency;
	int timeout;
} fsm_msg_gatt_connection_parameter_update_t;

typedef struct {
	bt_uuid_t app_uuid;
} fsm_msg_gatt_server_register_app_t;

typedef struct {
	uint8_t server_if;
} fsm_msg_gatt_server_unregister_app_t;

typedef struct {
	uint8_t server_if;
	bt_bdaddr_t address;
	uint8_t is_direct;
	int transport;
} fsm_msg_gatt_server_connect_t;

typedef struct {
	uint8_t server_if;
	bt_bdaddr_t address;
	uint16_t conn_id;
} fsm_msg_gatt_server_disconnect_t;

typedef struct {
	uint8_t server_if;
	uint16_t conn_id;
	uint8_t tx_phy;
	uint8_t rx_phy;
	uint8_t phy_options;
} fsm_msg_gatt_server_set_preferred_phy_t;

typedef struct {
	uint8_t client_if;
	uint16_t conn_id;
} fsm_msg_gatt_server_read_phy_t;

typedef struct {
	uint8_t server_if;
	uint8_t is_completed;
	uint16_t svc_data_len;
	uint8_t * svc_data;
} fsm_msg_gatt_server_add_service_t;

typedef struct {
	uint8_t server_if;
	uint16_t svc_handle;
} fsm_msg_gatt_server_stop_service_t;

typedef struct {
	uint8_t server_if;
	uint16_t svc_handle;
} fsm_msg_gatt_server_delete_service_t;

typedef struct {
	uint8_t server_if;
	uint16_t attr_handle;
	uint16_t conn_id;
	uint16_t val_len;
	uint8_t * val;
} fsm_msg_gatt_server_send_indication_t;

typedef struct {
	uint8_t server_if;
	uint16_t attr_handle;
	uint16_t conn_id;
	uint16_t val_len;
	uint8_t * val;
} fsm_msg_gatt_server_send_notification_t;

typedef struct {
	uint8_t server_if;
	uint16_t conn_id;
	uint16_t trans_id;
	int status;
	uint16_t handle;
	uint16_t offset;
	uint8_t auth_req;
	uint16_t val_len;
	uint8_t * val;
} fsm_msg_gatt_server_send_response_t;

typedef struct {
	int duration;
	int max_ext_adv_events;
	int reg_id;
	uint16_t param_len;
	uint8_t * param;
	uint16_t adv_data_len;
	uint8_t * adv_data;
	uint16_t scan_resp_data_len;
	uint8_t * scan_resp_data;
	uint16_t periodic_adv_data_len;
	uint8_t * period_adv_data;
	uint16_t periodic_data_len;
	uint8_t * periodic_data;
} fsm_msg_gatt_advm_start_advertising_set_t;

typedef struct {
	int advertiser_id;
} fsm_msg_gatt_advm_getownaddress_t;

typedef struct {
	int advertiser_id;
} fsm_msg_gatt_advm_stopadvertisingset_t;

typedef struct {
	int advertiser_id;
	uint8_t enable;
	int duration;
	int max_ext_adv_events;
} fsm_msg_gatt_advm_enableadvertisingset_t;

typedef struct {
	int advertiser_id;
	uint16_t data_len;
	uint8_t * data;
} fsm_msg_gatt_advm_set_advertising_data_t;

typedef struct {
	int advertiser_id;
	uint16_t data_len;
	uint8_t * data;
} fsm_msg_gatt_advm_set_scan_response_data_t;

typedef struct {
	int advertiser_id;
	uint16_t param_len;
	uint8_t * param;
} fsm_msg_gatt_advm_set_advertising_parameters_t;

typedef struct {
	int advertiser_id;
	uint16_t param_len;
	uint8_t * param;
} fsm_msg_gatt_advm_set_periodic_advertising_parameters_t;

typedef struct {
	int advertiser_id;
	uint16_t data_len;
	uint8_t * data;
} fsm_msg_gatt_advm_set_periodic_advertising_data_t;

typedef struct {
	int advertiser_id;
	uint8_t enable;
} fsm_msg_gatt_advm_set_periodic_advertising_enable_t;

typedef struct {
	int sid;
	bt_bdaddr_t address;
	int skip;
	int timeout;
	int reg_id;
} fsm_msg_gatt_scan_periodic_startsync_t;

typedef struct {
	int sync_handle;
} fsm_msg_gatt_scan_periodic_stopsync_t;

typedef struct {
	bt_uuid_t app_uuid;
} fsm_msg_gatt_scan_register_scanner_t;

typedef struct {
	int scanner_id;
} fsm_msg_gatt_scan_unregister_scanner_t;

typedef struct {
	uint8_t start;
} fsm_msg_gatt_scan_client_scan_t;

typedef struct {
	uint8_t client_if;
	int scan_interval;
	int scan_window;
} fsm_msg_gatt_scan_set_scan_parameters_t;

typedef struct {
	uint8_t client_if;
	uint8_t filter_type;
	uint8_t filt_index;
	int company_id;
	int company_id_mask;
	bt_uuid_t uuid;
	bt_uuid_t uuid_mask;
	bt_bdaddr_t address;
	uint8_t addr_type;
	uint16_t data_len;
	uint8_t * data;
	uint16_t mask_len;
	uint8_t * mask;
	uint16_t name_len;
	uint8_t * name;
} fsm_msg_gatt_client_scan_filter_add_t;

typedef struct {
	uint8_t client_if;
	uint8_t filter_type;
	uint8_t filt_index;
	int company_id;
	int company_id_mask;
	bt_uuid_t uuid;
	bt_uuid_t uuid_mask;
	bt_bdaddr_t address;
	uint8_t addr_type;
	uint16_t data_len;
	uint8_t * data;
	uint16_t mask_len;
	uint8_t * mask;
	uint16_t name_len;
	uint8_t * name;
} fsm_msg_gatt_client_scan_filter_delete_t;

typedef struct {
	uint16_t filter_params_len;
	uint8_t * filter_params;
} fsm_msg_gatt_client_scan_filter_param_add_t;

typedef struct {
	uint8_t client_if;
} fsm_msg_gatt_client_scan_filter_param_clear_all_t;

typedef struct {
	uint8_t client_if;
	uint8_t filt_index;
} fsm_msg_gatt_client_scan_filter_param_delete_t;

typedef struct {
	uint8_t client_if;
	uint8_t filt_index;
} fsm_msg_gatt_client_scan_filter_clear_t;

typedef struct {
	uint8_t client_if;
	uint8_t enable;
} fsm_msg_gatt_client_scan_filter_enable_t;

typedef struct {
	uint8_t client_if;
	uint8_t max_full_reports_percent;
	uint8_t max_truncated_reports_percent;
	uint8_t notify_threshold_percent;
} fsm_msg_gatt_client_config_batch_scan_storage_t;

typedef struct {
	int client_if;
	uint8_t scan_mode;
	int scan_interval_unit;
	int scan_window_unit;
	int address_type;
	int discard_rule;
} fsm_msg_gatt_client_start_batch_scan_t;

typedef struct {
	uint8_t client_if;
} fsm_msg_gatt_client_stop_batch_scan_t;

typedef struct {
	uint8_t client_if;
	uint8_t scan_type;
} fsm_msg_gatt_client_read_scan_reports_t;

void gatt_fsm_init(btgatt_callbacks_t *cbks);
int gatt_fsm_done();
void gatt_fsm_send_msg(uint8_t evt, void *param);

#endif  /*_APP_GATT_FSM_H__*/
