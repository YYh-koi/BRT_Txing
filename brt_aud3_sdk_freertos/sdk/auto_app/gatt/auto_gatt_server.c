#include "bt_gatt.h"
#include "att_ui.h"
#include "gap_ui.h"
#include "auto_gatt_server.h"
#include "auto_gatt_fsm.h"
#include "internal_gatt_fsm_server.h"


#define TAG "bse_gatt_server"
#include "brt_trace.h"

void app_gatt_server_init()
{
}

void app_gatt_server_done()
{
}



bt_status_t app_gatt_server_register_app(bt_uuid_t app_uuid)
{
	fsm_msg_gatt_server_register_app_t *msg = (fsm_msg_gatt_server_register_app_t *) NEW(sizeof(fsm_msg_gatt_server_register_app_t));
	msg->app_uuid = app_uuid;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_REGISTER_APP, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_unregister_app(unsigned char server_if)
{
	fsm_msg_gatt_server_unregister_app_t *msg = (fsm_msg_gatt_server_unregister_app_t *) NEW(sizeof(fsm_msg_gatt_server_unregister_app_t));
	msg->server_if = server_if;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_UNREGISTER_APP, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_connect(unsigned char server_if, bt_bdaddr_t address, unsigned char is_direct, int transport)
{
	fsm_msg_gatt_server_connect_t *msg = (fsm_msg_gatt_server_connect_t *) NEW(sizeof(fsm_msg_gatt_server_connect_t));
	msg->server_if = server_if;
	msg->address = address;
	msg->is_direct = is_direct;
	msg->transport = transport;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_CONNECT, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_disconnect(unsigned char server_if, bt_bdaddr_t address, short conn_id)
{
	fsm_msg_gatt_server_disconnect_t *msg = (fsm_msg_gatt_server_disconnect_t *) NEW(sizeof(fsm_msg_gatt_server_disconnect_t));
	msg->server_if = server_if;
	msg->address = address;
	msg->conn_id = conn_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_DISCONNECT, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_set_preferred_phy(unsigned char server_if, short conn_id, unsigned char tx_phy, unsigned char rx_phy, unsigned char phy_options)
{
	fsm_msg_gatt_server_set_preferred_phy_t *msg = (fsm_msg_gatt_server_set_preferred_phy_t *) NEW(sizeof(fsm_msg_gatt_server_set_preferred_phy_t));
	msg->server_if = server_if;
	msg->conn_id = conn_id;
	msg->tx_phy = tx_phy;
	msg->rx_phy = rx_phy;
	msg->phy_options = phy_options;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_SET_PREFERRED_PHY, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_read_phy(unsigned char client_if, short conn_id)
{
	fsm_msg_gatt_server_read_phy_t *msg = (fsm_msg_gatt_server_read_phy_t *) NEW(sizeof(fsm_msg_gatt_server_read_phy_t));
	msg->client_if = client_if;
	msg->conn_id = conn_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_READ_PHY, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_add_service(unsigned char server_if, unsigned char is_completed, short svc_data_len, unsigned char * svc_data)
{
	fsm_msg_gatt_server_add_service_t *msg = (fsm_msg_gatt_server_add_service_t *) NEW(sizeof(fsm_msg_gatt_server_add_service_t));
	msg->server_if = server_if;
	msg->is_completed = is_completed;
	msg->svc_data_len = svc_data_len;
	if (msg->svc_data_len > 0) {
		msg->svc_data = (unsigned char *) NEW(msg->svc_data_len);

		if (msg->svc_data) {
			memcpy(msg->svc_data, svc_data, msg->svc_data_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_ADD_SERVICE, (void *)msg);
	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_stop_service(unsigned char server_if, int svc_handle)
{
	fsm_msg_gatt_server_stop_service_t *msg = (fsm_msg_gatt_server_stop_service_t *) NEW(sizeof(fsm_msg_gatt_server_stop_service_t));
	msg->server_if = server_if;
	msg->svc_handle = svc_handle;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_STOP_SERVICE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_delete_service(unsigned char server_if, int svc_handle)
{
	fsm_msg_gatt_server_delete_service_t *msg = (fsm_msg_gatt_server_delete_service_t *) NEW(sizeof(fsm_msg_gatt_server_delete_service_t));
	msg->server_if = server_if;
	msg->svc_handle = svc_handle;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_DELETE_SERVICE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_send_indication(unsigned char server_if, short attr_handle, short conn_id, short val_len, unsigned char * val)
{
	fsm_msg_gatt_server_send_indication_t *msg = (fsm_msg_gatt_server_send_indication_t *) NEW(sizeof(fsm_msg_gatt_server_send_indication_t));
	msg->server_if = server_if;
	msg->attr_handle = attr_handle;
	msg->conn_id = conn_id;
	msg->val_len = val_len;
	if (msg->val_len > 0) {
		msg->val = (unsigned char *) NEW(msg->val_len);

		if (msg->val) {
			memcpy(msg->val, val, msg->val_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_SEND_INDICATION, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_send_notification(unsigned char server_if, short attr_handle, short conn_id, short val_len, unsigned char * val)
{
	fsm_msg_gatt_server_send_notification_t *msg = (fsm_msg_gatt_server_send_notification_t *) NEW(sizeof(fsm_msg_gatt_server_send_notification_t));
	msg->server_if = server_if;
	msg->attr_handle = attr_handle;
	msg->conn_id = conn_id;
	msg->val_len = val_len;
	if (msg->val_len > 0) {
		msg->val = (unsigned char *) NEW(msg->val_len);

		if (msg->val) {
			memcpy(msg->val, val, msg->val_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_SEND_NOTIFICATION, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_server_send_response(unsigned char server_if, short conn_id, short trans_id, int status, short handle, short offset, unsigned char auth_req, short val_len, unsigned char * val)
{
	fsm_msg_gatt_server_send_response_t *msg = (fsm_msg_gatt_server_send_response_t *) NEW(sizeof(fsm_msg_gatt_server_send_response_t));
	msg->server_if = server_if;
	msg->conn_id = conn_id;
	msg->trans_id = trans_id;
	msg->status = status;
	msg->handle = handle;
	msg->offset = offset;
	msg->val_len = val_len;
	if (msg->val_len > 0) {
		msg->val = (unsigned char *) NEW(msg->val_len);

		if (msg->val) {
			memcpy(msg->val, val, msg->val_len);
		}
	}
	msg->auth_req = auth_req;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_SERVER_SEND_RESPONSE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_add(unsigned char client_if, unsigned char filter_type, unsigned char filt_index, int company_id, int company_id_mask, bt_uuid_t uuid, bt_uuid_t uuid_mask, bt_bdaddr_t address, unsigned char addr_type, short data_len, unsigned char * data, short mask_len, unsigned char * mask, short name_len, unsigned char * name)
{
	fsm_msg_gatt_client_scan_filter_add_t *msg = (fsm_msg_gatt_client_scan_filter_add_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_add_t));
	msg->client_if = client_if;
	msg->filter_type = filter_type;
	msg->filt_index = filt_index;
	msg->company_id = company_id;
	msg->company_id_mask = company_id_mask;
	msg->uuid = uuid;
	msg->uuid_mask = uuid_mask;
	msg->address = address;
	msg->addr_type = addr_type;
	msg->data_len = data_len;
	if (msg->data_len > 0) {
		msg->data = (unsigned char *) NEW(msg->data_len);

		if (msg->data) {
			memcpy(msg->data, data, msg->data_len);
		}
	}
	msg->mask_len = mask_len;
	if (msg->mask_len > 0) {
		msg->mask = (unsigned char *) NEW(msg->mask_len);

		if (msg->mask) {
			memcpy(msg->mask, mask, msg->mask_len);
		}
	}
	msg->name_len = name_len;
	if (msg->name_len > 0) {
		msg->name = (unsigned char *) NEW(msg->name_len);

		if (msg->name) {
			memcpy(msg->name, name, msg->name_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ADD, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_delete(unsigned char client_if, unsigned char filter_type, unsigned char filt_index, int company_id, int company_id_mask, bt_uuid_t uuid, bt_uuid_t uuid_mask, bt_bdaddr_t address, unsigned char addr_type, short data_len, unsigned char * data, short mask_len, unsigned char * mask, short name_len, unsigned char * name)
{
	fsm_msg_gatt_client_scan_filter_delete_t *msg = (fsm_msg_gatt_client_scan_filter_delete_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_delete_t));
	msg->client_if = client_if;
	msg->filter_type = filter_type;
	msg->filt_index = filt_index;
	msg->company_id = company_id;
	msg->company_id_mask = company_id_mask;
	msg->uuid = uuid;
	msg->uuid_mask = uuid_mask;
	msg->address = address;
	msg->addr_type = addr_type;
	msg->data_len = data_len;
	if (msg->data_len > 0) {
		msg->data = (unsigned char *) NEW(msg->data_len);

		if (msg->data) {
			memcpy(msg->data, data, msg->data_len);
		}
	}
	msg->mask_len = mask_len;
	if (msg->mask_len > 0) {
		msg->mask = (unsigned char *) NEW(msg->mask_len);

		if (msg->mask) {
			memcpy(msg->mask, mask, msg->mask_len);
		}
	}
	msg->name_len = name_len;
	if (msg->name_len > 0) {
		msg->name = (unsigned char *) NEW(msg->name_len);

		if (msg->name) {
			memcpy(msg->name, name, msg->name_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_DELETE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_param_add(short filter_params_len, unsigned char * filter_params)
{
	fsm_msg_gatt_client_scan_filter_param_add_t *msg = (fsm_msg_gatt_client_scan_filter_param_add_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_param_add_t));
	msg->filter_params_len = filter_params_len;
	if (msg->filter_params_len > 0) {
		msg->filter_params = (unsigned char *) NEW(msg->filter_params_len);

		if (msg->filter_params) {
			memcpy(msg->filter_params, filter_params, msg->filter_params_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_ADD, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_param_clear_all(unsigned char client_if)
{
	fsm_msg_gatt_client_scan_filter_param_clear_all_t *msg = (fsm_msg_gatt_client_scan_filter_param_clear_all_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_param_clear_all_t));
	msg->client_if = client_if;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_CLEAR_ALL, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_param_delete(unsigned char client_if, unsigned char filt_index)
{
	fsm_msg_gatt_client_scan_filter_param_delete_t *msg = (fsm_msg_gatt_client_scan_filter_param_delete_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_param_delete_t));
	msg->client_if = client_if;
	msg->filt_index = filt_index;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_PARAM_DELETE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_clear(unsigned char client_if, unsigned char filt_index)
{
	fsm_msg_gatt_client_scan_filter_clear_t *msg = (fsm_msg_gatt_client_scan_filter_clear_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_clear_t));
	msg->client_if = client_if;
	msg->filt_index = filt_index;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_CLEAR, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_scan_filter_enable(unsigned char client_if, unsigned char enable)
{
	fsm_msg_gatt_client_scan_filter_enable_t *msg = (fsm_msg_gatt_client_scan_filter_enable_t *) NEW(sizeof(fsm_msg_gatt_client_scan_filter_enable_t));
	msg->client_if = client_if;
	msg->enable = enable;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SCAN_FILTER_ENABLE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_config_batch_scan_storage(unsigned char client_if, unsigned char max_full_reports_percent, unsigned char max_truncated_reports_percent, unsigned char notify_threshold_percent)
{
	fsm_msg_gatt_client_config_batch_scan_storage_t *msg = (fsm_msg_gatt_client_config_batch_scan_storage_t *) NEW(sizeof(fsm_msg_gatt_client_config_batch_scan_storage_t));
	msg->client_if = client_if;
	msg->max_full_reports_percent = max_full_reports_percent;
	msg->max_truncated_reports_percent = max_truncated_reports_percent;
	msg->notify_threshold_percent = notify_threshold_percent;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_CONFIG_BATCH_SCAN_STORAGE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_start_batch_scan(int client_if, unsigned char scan_mode, int scan_interval_unit, int scan_window_unit, int address_type, int discard_rule)
{
	fsm_msg_gatt_client_start_batch_scan_t *msg = (fsm_msg_gatt_client_start_batch_scan_t *) NEW(sizeof(fsm_msg_gatt_client_start_batch_scan_t));
	msg->client_if = client_if;
	msg->scan_mode = scan_mode;
	msg->scan_interval_unit = scan_interval_unit;
	msg->scan_window_unit = scan_window_unit;
	msg->address_type = address_type;
	msg->discard_rule = discard_rule;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_START_BATCH_SCAN, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_stop_batch_scan(unsigned char client_if)
{
	fsm_msg_gatt_client_stop_batch_scan_t *msg = (fsm_msg_gatt_client_stop_batch_scan_t *) NEW(sizeof(fsm_msg_gatt_client_stop_batch_scan_t));
	msg->client_if = client_if;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_STOP_BATCH_SCAN, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_scan_reports(unsigned char client_if, unsigned char scan_type)
{
	fsm_msg_gatt_client_read_scan_reports_t *msg = (fsm_msg_gatt_client_read_scan_reports_t *) NEW(sizeof(fsm_msg_gatt_client_read_scan_reports_t));
	msg->client_if = client_if;
	msg->scan_type = scan_type;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_SCAN_REPORTS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

void app_gatt_server_on_device_connected(struct HCI_AddressStru *bd, UINT16 acl_hdl)
{
	hlp_gatt_server_on_device_connected(bd, acl_hdl);
}

void app_gatt_server_on_device_disconnected(struct HCI_AddressStru *bd, UINT16 acl_hdl)
{
	hlp_gatt_server_on_device_disconnected(bd, acl_hdl);
}
