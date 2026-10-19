#include "auto_gatt_event.h"

static module_event_cbk_t s_event_cbk;

static void gatt_event_indication(uint32_t event, void *param)
{
	if (s_event_cbk != NULL) {
		s_event_cbk((PROFILE_ID_GATT << 8) |event, param);
	}
}

static void client_registered_cb(uint8_t status, uint8_t client_if, bt_uuid_t uuid)
{
	evt_param_client_registered_t param;
	param.status = status;
	param.client_if = client_if;
	param.uuid = uuid;
	gatt_event_indication(EV_GATT_CLIENT_REGISTERED, (void *) &param);
}

static void scanner_registered_cb(uint8_t status, int scan_id, bt_uuid_t uuid)
{
	evt_param_scanner_registered_t param;
	param.status = status;
	param.scan_id = scan_id;
	param.uuid = uuid;
	gatt_event_indication(EV_GATT_SCANNER_REGISTERED, (void *) &param);
}

static void scan_result_cb(uint8_t event_type, uint8_t address_type, bt_bdaddr_t address, uint8_t primary_phy, uint8_t secondary_phy, uint8_t advertising_sid, uint8_t tx_power, uint8_t rssi, uint16_t periodic_adv_int, uint16_t adv_data_len, uint8_t * adv_data)
{
	evt_param_scan_result_t param;
	param.event_type = event_type;
	param.address_type = address_type;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.primary_phy = primary_phy;
	param.secondary_phy = secondary_phy;
	param.advertising_sid = advertising_sid;
	param.tx_power = tx_power;
	param.rssi = rssi;
	param.periodic_adv_int = periodic_adv_int;
	param.adv_data_len = adv_data_len;
	param.adv_data = adv_data;
	gatt_event_indication(EV_GATT_SCAN_RESULT, (void *) &param);
}

static void connected_cb(uint8_t client_if, uint16_t conn_id, uint8_t status, bt_bdaddr_t address)
{
	evt_param_connected_t param;
	param.client_if = client_if;
	param.conn_id = conn_id;
	param.status = status;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	gatt_event_indication(EV_GATT_CONNECTED, (void *) &param);
}

static void disconnected_cb(uint8_t client_if, uint16_t conn_id, uint8_t status, bt_bdaddr_t address)
{
	evt_param_disconnected_t param;
	param.client_if = client_if;
	param.conn_id = conn_id;
	param.status = status;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	gatt_event_indication(EV_GATT_DISCONNECTED, (void *) &param);
}

static void read_characteristic_cb(uint16_t conn_id, uint8_t status, uint16_t handle, uint16_t data_len, uint8_t * data)
{
	evt_param_read_characteristic_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.handle = handle;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_READ_CHARACTERISTIC, (void *) &param);
}

static void write_characteristic_cb(uint16_t conn_id, uint8_t status, uint16_t handle)
{
	evt_param_write_characteristic_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.handle = handle;
	gatt_event_indication(EV_GATT_WRITE_CHARACTERISTIC, (void *) &param);
}

static void execute_completed_cb(uint16_t conn_id, uint8_t status)
{
	evt_param_execute_completed_t param;
	param.conn_id = conn_id;
	param.status = status;
	gatt_event_indication(EV_GATT_EXECUTE_COMPLETED, (void *) &param);
}

static void search_completed_cb(uint16_t conn_id, uint8_t status)
{
	evt_param_search_completed_t param;
	param.conn_id = conn_id;
	param.status = status;
	gatt_event_indication(EV_GATT_SEARCH_COMPLETED, (void *) &param);
}

static void read_descriptor_cb(uint16_t conn_id, uint8_t status, uint16_t handle, uint16_t data_len, uint8_t * data)
{
	evt_param_read_descriptor_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.handle = handle;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_READ_DESCRIPTOR, (void *) &param);
}

static void write_descriptor_cb(uint16_t conn_id, uint8_t status, uint16_t handle)
{
	evt_param_write_descriptor_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.handle = handle;
	gatt_event_indication(EV_GATT_WRITE_DESCRIPTOR, (void *) &param);
}

static void notify_cb(uint16_t conn_id, bt_bdaddr_t address, uint16_t handle, uint8_t is_notify, uint16_t data_len, uint8_t * data)
{
	evt_param_notify_t param;
	param.conn_id = conn_id;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.handle = handle;
	param.is_notify = is_notify;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_NOTIFY, (void *) &param);
}

static void register_for_notifications_cb(uint16_t conn_id, uint8_t status, uint8_t registered, uint16_t handle)
{
	evt_param_register_for_notifications_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.registered = registered;
	param.handle = handle;
	gatt_event_indication(EV_GATT_REGISTER_FOR_NOTIFICATIONS, (void *) &param);
}

static void read_remote_rssi_cb(uint8_t client_if, bt_bdaddr_t address, uint8_t rssi, uint8_t status)
{
	printf("%s, rssi = %d, status = %x\r\n",__func__, rssi, status);
	evt_param_read_remote_rssi_t param;
	param.client_if = client_if;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.rssi = rssi;
	param.status = status;
	gatt_event_indication(EV_GATT_READ_REMOTE_RSSI, (void *) &param);
}

static void configure_mtu_cb(uint16_t conn_id, uint8_t status, int mtu)
{
	evt_param_configure_mtu_t param;
	param.conn_id = conn_id;
	param.status = status;
	param.mtu = mtu;
	gatt_event_indication(EV_GATT_CONFIGURE_MTU, (void *) &param);
}

static void scan_filter_config_cb(uint8_t action, uint8_t status, uint8_t client_if, uint8_t filter_type, int available_space)
{
	evt_param_scan_filter_config_t param;
	param.action = action;
	param.status = status;
	param.client_if = client_if;
	param.filter_type = filter_type;
	param.available_space = available_space;
	gatt_event_indication(EV_GATT_SCAN_FILTER_CONFIG, (void *) &param);
}

static void scan_filter_params_configured_cb(uint8_t action, uint8_t status, uint8_t client_if, int available_space)
{
	evt_param_scan_filter_params_configured_t param;
	param.action = action;
	param.status = status;
	param.client_if = client_if;
	param.available_space = available_space;
	gatt_event_indication(EV_GATT_SCAN_FILTER_PARAMS_CONFIGURED, (void *) &param);
}

static void scan_filter_enable_disabled_cb(uint8_t action, uint8_t status, uint8_t client_if)
{
	evt_param_scan_filter_enable_disabled_t param;
	param.action = action;
	param.status = status;
	param.client_if = client_if;
	gatt_event_indication(EV_GATT_SCAN_FILTER_ENABLE_DISABLED, (void *) &param);
}

static void client_congestion_cb(uint16_t conn_id, uint8_t congested)
{
	evt_param_client_congestion_t param;
	param.conn_id = conn_id;
	param.congested = congested;
	gatt_event_indication(EV_GATT_CLIENT_CONGESTION, (void *) &param);
}

static void batch_scan_storage_configured_cb(uint8_t status, uint8_t client_if)
{
	evt_param_batch_scan_storage_configured_t param;
	param.status = status;
	param.client_if = client_if;
	gatt_event_indication(EV_GATT_BATCH_SCAN_STORAGE_CONFIGURED, (void *) &param);
}

static void batch_scan_start_stopped_cb(uint8_t action, uint8_t status, uint8_t client_if)
{
	evt_param_batch_scan_start_stopped_t param;
	param.action = action;
	param.status = status;
	param.client_if = client_if;
	gatt_event_indication(EV_GATT_BATCH_SCAN_START_STOPPED, (void *) &param);
}

#if 0
typedef struct
{
    uint8_t  client_if;
    uint8_t  filt_index;
    uint8_t  advertiser_state;
    uint8_t  advertiser_info_present;
    uint8_t  addr_type;
    uint8_t  tx_power;
    int8_t  rssi_value;
    uint16_t time_stamp;
    bt_bdaddr_t bd_addr;
    uint8_t  adv_pkt_len;
    uint8_t  *p_adv_pkt_data;
    uint8_t  scan_rsp_len;
    uint8_t  *p_scan_rsp_data;
} btgatt_track_adv_info_t;

struct AdvertiseParameters {
  uint16_t advertising_event_properties;
  uint32_t min_interval;
  uint32_t max_interval;
  uint8_t channel_map;
  int8_t tx_power;
  uint8_t primary_advertising_phy;
  uint8_t secondary_advertising_phy;
  uint8_t scan_request_notification_enable;
};

struct PeriodicAdvertisingParameters {
  uint8_t enable;
  uint16_t min_interval;
  uint16_t max_interval;
  uint16_t periodic_advertising_properties;
};


typedef void (*batchscan_reports_callback)(int client_if, int status,
                                           int report_format, int num_records,
                                           std::vector<uint8_t> data);

typedef void (*batchscan_threshold_callback)(int client_if);

typedef void (*track_adv_event_callback)(
    btgatt_track_adv_info_t *p_track_adv_info);

typedef void (*scan_result_callback)(uint16_t event_type, uint8_t addr_type,
                                     bt_bdaddr_t *bda, uint8_t primary_phy,
                                     uint8_t secondary_phy,
                                     uint8_t advertising_sid, int8_t tx_power,
                                     int8_t rssi, uint16_t periodic_adv_int,
                                     std::vector<uint8_t> adv_data);
#endif

static void batch_scan_reports_cb(uint8_t status, int scanner_id, uint8_t report_type, int num_records, uint16_t record_data_len, uint8_t * record_data)
{
	evt_param_batch_scan_reports_t param;
	param.status = status;
	param.scanner_id = scanner_id;
	param.report_type = report_type;
	param.num_records = num_records;
	param.record_data_len = record_data_len;
	param.record_data = record_data;
	gatt_event_indication(EV_GATT_BATCH_SCAN_REPORTS, (void *) &param);
}

static void batch_scan_threshold_crossed_cb(uint8_t client_if)
{
	evt_param_batch_scan_threshold_crossed_t param;
	param.client_if = client_if;
	gatt_event_indication(EV_GATT_BATCH_SCAN_THRESHOLD_CROSSED, (void *) &param);
}

static void track_adv_found_lost_cb(uint8_t client_if, uint16_t adv_pkt_len, uint8_t * adv_pkt, uint16_t scan_rsp_len, uint8_t * scan_rsp, uint8_t filt_index, uint8_t adv_state, uint8_t adv_info_present, bt_bdaddr_t address, uint8_t addr_type, uint8_t tx_power, uint16_t rssi_value, uint16_t time_stamp)
{
	evt_param_track_adv_found_lost_t param;
	param.client_if = client_if;
	param.adv_pkt_len = adv_pkt_len;
	param.adv_pkt = adv_pkt;
	param.scan_rsp_len = scan_rsp_len;
	param.scan_rsp = scan_rsp;
	param.filt_index = filt_index;
	param.adv_state = adv_state;
	param.adv_info_present = adv_info_present;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.addr_type = addr_type;
	param.tx_power = tx_power;
	param.rssi_value = rssi_value;
	param.time_stamp = time_stamp;
	gatt_event_indication(EV_GATT_TRACK_ADV_FOUND_LOST, (void *) &param);
}

static void scan_param_setup_completed_cb(uint8_t status, int scanner_id)
{
	evt_param_scan_param_setup_completed_t param;
	param.status = status;
	param.scanner_id = scanner_id;
	gatt_event_indication(EV_GATT_SCAN_PARAM_SETUP_COMPLETED, (void *) &param);
}

static void get_gatt_db_cb(uint16_t conn_id, uint8_t is_completed, uint16_t svc_data_len, uint8_t * svc_data)
{
	evt_param_get_gatt_db_t param;
	param.conn_id = conn_id;
	param.is_completed = is_completed;
	param.svc_data_len = svc_data_len;
	param.svc_data = svc_data;
	gatt_event_indication(EV_GATT_GET_GATT_DB, (void *) &param);
}

static void client_phy_read_cb(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status)
{
	evt_param_client_phy_read_t param;
	param.conn_id = conn_id;
	param.tx_phy = tx_phy;
	param.rx_phy = rx_phy;
	param.status = status;
	gatt_event_indication(EV_GATT_CLIENT_PHY_READ, (void *) &param);
}

static void client_phy_update_cb(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status)
{
	evt_param_client_phy_update_t param;
	param.conn_id = conn_id;
	param.tx_phy = tx_phy;
	param.rx_phy = rx_phy;
	param.status = status;
	gatt_event_indication(EV_GATT_CLIENT_PHY_UPDATE, (void *) &param);
}

static void client_conn_update_cb(uint16_t conn_id, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status)
{
	evt_param_client_conn_update_t param;
	param.conn_id = conn_id;
	param.interval = interval;
	param.latency = latency;
	param.timeout = timeout;
	param.status = status;
	gatt_event_indication(EV_GATT_CLIENT_CONN_UPDATE, (void *) &param);
}

static void server_registered_cb(uint8_t status, uint8_t server_if, bt_uuid_t uuid)
{
	evt_param_server_registered_t param;
	param.status = status;
	param.server_if = server_if;
	param.uuid = uuid;
	gatt_event_indication(EV_GATT_SERVER_REGISTERED, (void *) &param);
}

static void client_connected_cb(bt_bdaddr_t address, uint8_t connected, uint16_t conn_id, uint8_t server_if)
{
	evt_param_client_connected_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.connected = connected;
	param.conn_id = conn_id;
	param.server_if = server_if;
	gatt_event_indication(EV_GATT_CLIENT_CONNECTED, (void *) &param);
}

static void service_added_cb(btgatt_db_element_t  * element)//uint8_t status, uint8_t server_if,uint16_t srv_handle, uint8_t is_completed, uint16_t svc_data_len, uint8_t * svc_data)
{
    //evt_param_service_added_t param;
    //param.status = status;
    //param.server_if = server_if;
    //param.srv_handle = srv_handle;
    //param.is_completed = is_completed;
    //param.svc_data_len = svc_data_len;
    //param.svc_data = svc_data;
    gatt_event_indication(EV_GATT_SERVICE_ADDED, (void *) element);
}

static void service_stopped_cb(uint8_t status, uint8_t server_if, uint16_t srvc_handle)
{
	evt_param_service_stopped_t param;
	param.status = status;
	param.server_if = server_if;
	param.srvc_handle = srvc_handle;
	gatt_event_indication(EV_GATT_SERVICE_STOPPED, (void *) &param);
}

static void service_deleted_cb(uint8_t status, uint8_t server_if, uint16_t srvc_handle)
{
	evt_param_service_deleted_t param;
	param.status = status;
	param.server_if = server_if;
	param.srvc_handle = srvc_handle;
	gatt_event_indication(EV_GATT_SERVICE_DELETED, (void *) &param);
}

static void response_send_completed_cb(uint8_t status, uint16_t attr_handle)
{
	evt_param_response_send_completed_t param;
	param.status = status;
	param.attr_handle = attr_handle;
	gatt_event_indication(EV_GATT_RESPONSE_SEND_COMPLETED, (void *) &param);
}

static void server_read_characteristic_cb(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, uint8_t is_long)
{
	evt_param_server_read_characteristic_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.conn_id = conn_id;
	param.trans_id = trans_id;
	param.handle = handle;
	param.offset = offset;
	param.is_long = is_long;
	gatt_event_indication(EV_GATT_SERVER_READ_CHARACTERISTIC, (void *) &param);
}

static void server_read_descriptor_cb(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, uint8_t is_long)
{
	evt_param_server_read_descriptor_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.conn_id = conn_id;
	param.trans_id = trans_id;
	param.handle = handle;
	param.offset = offset;
	param.is_long = is_long;
	gatt_event_indication(EV_GATT_SERVER_READ_DESCRIPTOR, (void *) &param);
}

static void server_write_characteristic_cb(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, int length, uint8_t need_rsp, uint8_t is_prep, uint16_t data_len, uint8_t * data)
{
	evt_param_server_write_characteristic_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.conn_id = conn_id;
	param.trans_id = trans_id;
	param.handle = handle;
	param.offset = offset;
	param.length = length;
	param.need_rsp = need_rsp;
	param.is_prep = is_prep;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_SERVER_WRITE_CHARACTERISTIC, (void *) &param);
}

static void server_write_descriptor_cb(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t handle, uint16_t offset, int length, uint8_t need_rsp, uint8_t is_prep, uint16_t data_len, uint8_t * data)
{
	evt_param_server_write_descriptor_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.conn_id = conn_id;
	param.trans_id = trans_id;
	param.handle = handle;
	param.offset = offset;
	param.length = length;
	param.need_rsp = need_rsp;
	param.is_prep = is_prep;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_SERVER_WRITE_DESCRIPTOR, (void *) &param);
}

static void execute_write_cb(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint8_t exec_write)
{
	evt_param_execute_write_t param;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.conn_id = conn_id;
	param.trans_id = trans_id;
	param.exec_write = exec_write;
	gatt_event_indication(EV_GATT_EXECUTE_WRITE, (void *) &param);
}

static void notification_sent_cb(uint16_t conn_id, uint8_t status)
{
	evt_param_notification_sent_t param;
	param.conn_id = conn_id;
	param.status = status;
	gatt_event_indication(EV_GATT_NOTIFICATION_SENT, (void *) &param);
}

static void server_congestion_cb(uint16_t conn_id, uint8_t congested)
{
	evt_param_server_congestion_t param;
	param.conn_id = conn_id;
	param.congested = congested;
	gatt_event_indication(EV_GATT_SERVER_CONGESTION, (void *) &param);
}

static void server_mtu_changed_cb(uint16_t conn_id, int mtu)
{
	evt_param_server_mtu_changed_t param;
	param.conn_id = conn_id;
	param.mtu = mtu;
	gatt_event_indication(EV_GATT_SERVER_MTU_CHANGED, (void *) &param);
}

static void server_phy_read_cb(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status)
{
	evt_param_server_phy_read_t param;
	param.conn_id = conn_id;
	param.tx_phy = tx_phy;
	param.rx_phy = rx_phy;
	param.status = status;
	gatt_event_indication(EV_GATT_SERVER_PHY_READ, (void *) &param);
}

static void server_phy_update_cb(uint16_t conn_id, uint8_t tx_phy, uint8_t rx_phy, uint8_t status)
{
	evt_param_server_phy_update_t param;
	param.conn_id = conn_id;
	param.tx_phy = tx_phy;
	param.rx_phy = rx_phy;
	param.status = status;
	gatt_event_indication(EV_GATT_SERVER_PHY_UPDATE, (void *) &param);
}

static void server_conn_update_cb(uint16_t conn_id, int interval, int latency, int timeout, uint8_t status)
{
	evt_param_server_conn_update_t param;
	param.conn_id = conn_id;
	param.interval = interval;
	param.latency = latency;
	param.timeout = timeout;
	param.status = status;
	gatt_event_indication(EV_GATT_SERVER_CONN_UPDATE, (void *) &param);
}

static void advertising_set_started_cb(int reg_id, int advertiser_id, uint8_t tx_power, uint8_t status)
{
	evt_param_advertising_set_started_t param;
	param.reg_id = reg_id;
	param.advertiser_id = advertiser_id;
	param.tx_power = tx_power;
	param.status = status;
	gatt_event_indication(EV_GATT_ADVERTISING_SET_STARTED, (void *) &param);
}

static void own_address_read_cb(int advertiser_id, uint8_t address_type, bt_bdaddr_t address)
{
	evt_param_own_address_read_t param;
	param.advertiser_id = advertiser_id;
	param.address_type = address_type;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	gatt_event_indication(EV_GATT_OWN_ADDRESS_READ, (void *) &param);
}

static void advertising_enabled_cb(int advertiser_id, uint8_t enable, uint8_t status)
{
	evt_param_advertising_enabled_t param;
	param.advertiser_id = advertiser_id;
	param.enable = enable;
	param.status = status;
	gatt_event_indication(EV_GATT_ADVERTISING_ENABLED, (void *) &param);
}

static void advertising_data_set_cb(int advertiser_id, uint8_t status)
{
	evt_param_advertising_data_set_t param;
	param.advertiser_id = advertiser_id;
	param.status = status;
	gatt_event_indication(EV_GATT_ADVERTISING_DATA_SET, (void *) &param);
}

static void scan_response_data_set_cb(int advertiser_id, uint8_t status)
{
	evt_param_scan_response_data_set_t param;
	param.advertiser_id = advertiser_id;
	param.status = status;
	gatt_event_indication(EV_GATT_SCAN_RESPONSE_DATA_SET, (void *) &param);
}

static void advertising_parameters_updated_cb(int advertiser_id, uint8_t tx_power, uint8_t status)
{
	evt_param_advertising_parameters_updated_t param;
	param.advertiser_id = advertiser_id;
	param.tx_power = tx_power;
	param.status = status;
	gatt_event_indication(EV_GATT_ADVERTISING_PARAMETERS_UPDATED, (void *) &param);
}

static void periodic_advertising_parameters_updated_cb(int advertiser_id, uint8_t status)
{
	evt_param_periodic_advertising_parameters_updated_t param;
	param.advertiser_id = advertiser_id;
	param.status = status;
	gatt_event_indication(EV_GATT_PERIODIC_ADVERTISING_PARAMETERS_UPDATED, (void *) &param);
}

static void periodic_advertising_data_set_cb(int advertiser_id, uint8_t status)
{
	evt_param_periodic_advertising_data_set_t param;
	param.advertiser_id = advertiser_id;
	param.status = status;
	gatt_event_indication(EV_GATT_PERIODIC_ADVERTISING_DATA_SET, (void *) &param);
}

static void periodic_advertising_enabled_cb(int advertiser_id, uint8_t enable, uint8_t status)
{
	evt_param_periodic_advertising_enabled_t param;
	param.advertiser_id = advertiser_id;
	param.enable = enable;
	param.status = status;
	gatt_event_indication(EV_GATT_PERIODIC_ADVERTISING_ENABLED, (void *) &param);
}

static void sync_lost_cb(uint16_t sync_handle)
{
	evt_param_sync_lost_t param;
	param.sync_handle = sync_handle;
	gatt_event_indication(EV_GATT_SYNC_LOST, (void *) &param);
}

static void sync_report_cb(uint16_t conn_id, uint8_t tx_power, uint8_t rssi, uint8_t data_status, uint16_t data_len, uint8_t * data)
{
	evt_param_sync_report_t param;
	param.conn_id = conn_id;
	param.tx_power = tx_power;
	param.rssi = rssi;
	param.data_status = data_status;
	param.data_len = data_len;
	param.data = data;
	gatt_event_indication(EV_GATT_SYNC_REPORT, (void *) &param);
}

static void sync_started_cb(uint8_t reg_id, uint8_t status, uint16_t sync_handle, uint8_t sid, uint8_t address_type, bt_bdaddr_t address, uint8_t phy, uint16_t interval)
{
	evt_param_sync_started_t param;
	param.reg_id = reg_id;
	param.status = status;
	param.sync_handle = sync_handle;
	param.sid = sid;
	param.address_type = address_type;
	memcpy(param.address.address, address.address, BD_ADDR_LEN);
	param.phy = phy;
	param.interval = interval;
	gatt_event_indication(EV_GATT_SYNC_STARTED, (void *) &param);
}

static const btgatt_client_callbacks_t s_gatt_client_callbacks = {
	client_registered_cb,
	scanner_registered_cb,
	scan_result_cb,
	connected_cb,
	disconnected_cb,
	read_characteristic_cb,
	write_characteristic_cb,
	execute_completed_cb,
	search_completed_cb,
	read_descriptor_cb,
	write_descriptor_cb,
	notify_cb,
	register_for_notifications_cb,
	read_remote_rssi_cb,
	configure_mtu_cb,
	scan_filter_config_cb,
	scan_filter_params_configured_cb,
	scan_filter_enable_disabled_cb,
	client_congestion_cb,
	batch_scan_storage_configured_cb,
	batch_scan_start_stopped_cb,
	batch_scan_reports_cb,
	batch_scan_threshold_crossed_cb,
	track_adv_found_lost_cb,
	scan_param_setup_completed_cb,
	get_gatt_db_cb,
	client_phy_read_cb,
	client_phy_update_cb,
	client_conn_update_cb,
};

static const btgatt_server_callbacks_t s_gatt_server_callbacks = {
	server_registered_cb,
	client_connected_cb,
	service_added_cb,
	service_stopped_cb,
	service_deleted_cb,
	response_send_completed_cb,
	server_read_characteristic_cb,
	server_read_descriptor_cb,
	server_write_characteristic_cb,
	server_write_descriptor_cb,
	execute_write_cb,
	notification_sent_cb,
	server_congestion_cb,
	server_mtu_changed_cb,
	server_phy_read_cb,
	server_phy_update_cb,
	server_conn_update_cb,
	advertising_set_started_cb,
	own_address_read_cb,
	advertising_enabled_cb,
	advertising_data_set_cb,
	scan_response_data_set_cb,
	advertising_parameters_updated_cb,
	periodic_advertising_parameters_updated_cb,
	periodic_advertising_data_set_cb,
	periodic_advertising_enabled_cb,
	sync_lost_cb,
	sync_report_cb,
	sync_started_cb,
};

static btgatt_callbacks_t s_callbacks = {
	sizeof(btgatt_callbacks_t),
	&s_gatt_client_callbacks,
	&s_gatt_server_callbacks,
};


btgatt_callbacks_t*app_gatt_event_init(module_event_cbk_t event_cbk)
{
	s_event_cbk = event_cbk;

	return &s_callbacks;
}

void app_gatt_event_done()
{
}
