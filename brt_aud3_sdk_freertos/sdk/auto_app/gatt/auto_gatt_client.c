#include "bt_gatt.h"
#include "att_ui.h"
#include "gap_ui.h"
#include "auto_gatt_client.h"
#include "auto_gatt_fsm.h"
#include "internal_gatt_fsm_client.h"

void app_gatt_client_init()
{
}

void app_gatt_client_done()
{
}

bt_status_t app_gatt_test(int cmd)
{
	fsm_msg_gatt_test_t *msg = (fsm_msg_gatt_test_t *) NEW(sizeof(fsm_msg_gatt_test_t));
	msg->cmd = cmd;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_TEST, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_get_device_type(bt_bdaddr_t address)
{
	fsm_msg_gatt_client_get_device_type_t *msg = (fsm_msg_gatt_client_get_device_type_t *) NEW(sizeof(fsm_msg_gatt_client_get_device_type_t));
	msg->address = address;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_GET_DEVICE_TYPE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_register_app(bt_uuid_t uuid)
{
	fsm_msg_gatt_client_register_app_t *msg = (fsm_msg_gatt_client_register_app_t *) NEW(sizeof(fsm_msg_gatt_client_register_app_t));
	msg->uuid = uuid;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_REGISTER_APP, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_unregister_app(unsigned char client_if)
{
	fsm_msg_gatt_client_unregister_app_t *msg = (fsm_msg_gatt_client_unregister_app_t *) NEW(sizeof(fsm_msg_gatt_client_unregister_app_t));
	msg->client_if = client_if;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_UNREGISTER_APP, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_connect(unsigned char client_if, bt_bdaddr_t address, unsigned char is_direct, int transport, unsigned char opportunistic, int initiating_phys)
{
	fsm_msg_gatt_client_connect_t *msg = (fsm_msg_gatt_client_connect_t *) NEW(sizeof(fsm_msg_gatt_client_connect_t));
	msg->client_if = client_if;
	msg->address = address;
	msg->is_direct = is_direct;
	msg->transport = transport;
	msg->opportunistic = opportunistic;
	msg->initiating_phys = initiating_phys;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_CONNECT, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_disconnect(unsigned char client_if, bt_bdaddr_t address, short conn_id)
{
	fsm_msg_gatt_client_disconnect_t *msg = (fsm_msg_gatt_client_disconnect_t *) NEW(sizeof(fsm_msg_gatt_client_disconnect_t));
	msg->client_if = client_if;
	msg->address = address;
	msg->conn_id = conn_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_DISCONNECT, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_set_preferred_phy(unsigned char client_if, short conn_id, unsigned char tx_phy, unsigned char rx_phy, unsigned char phy_options)
{
	fsm_msg_gatt_client_set_preferred_phy_t *msg = (fsm_msg_gatt_client_set_preferred_phy_t *) NEW(sizeof(fsm_msg_gatt_client_set_preferred_phy_t));
	msg->client_if = client_if;
	msg->conn_id = conn_id;
	msg->tx_phy = tx_phy;
	msg->rx_phy = rx_phy;
	msg->phy_options = phy_options;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SET_PREFERRED_PHY, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_phy(unsigned char client_if, short conn_id)
{
	fsm_msg_gatt_client_read_phy_t *msg = (fsm_msg_gatt_client_read_phy_t *) NEW(sizeof(fsm_msg_gatt_client_read_phy_t));
	msg->client_if = client_if;
	msg->conn_id = conn_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_PHY, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_refresh(unsigned char client_if, bt_bdaddr_t address)
{
	fsm_msg_gatt_client_refresh_t *msg = (fsm_msg_gatt_client_refresh_t *) NEW(sizeof(fsm_msg_gatt_client_refresh_t));
	msg->client_if = client_if;
	msg->address = address;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_REFRESH, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_search_service(short conn_id, unsigned char search_all, bt_uuid_t service_uuid)
{
	fsm_msg_gatt_client_search_service_t *msg = (fsm_msg_gatt_client_search_service_t *) NEW(sizeof(fsm_msg_gatt_client_search_service_t));
	msg->conn_id = conn_id;
	msg->search_all = search_all;
	msg->service_uuid = service_uuid;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_SEARCH_SERVICE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_discover_service_by_uuid(short conn_id, bt_uuid_t service_uuid)
{
	fsm_msg_gatt_client_discover_service_by_uuid_t *msg = (fsm_msg_gatt_client_discover_service_by_uuid_t *) NEW(sizeof(fsm_msg_gatt_client_discover_service_by_uuid_t));
	msg->conn_id = conn_id;
	msg->service_uuid = service_uuid;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_DISCOVER_SERVICE_BY_UUID, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_get_gatt_db(short conn_id)
{
	fsm_msg_gatt_client_get_gatt_db_t *msg = (fsm_msg_gatt_client_get_gatt_db_t *) NEW(sizeof(fsm_msg_gatt_client_get_gatt_db_t));
	msg->conn_id = conn_id;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_GET_GATT_DB, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_characteristic(short conn_id, short handle, unsigned char auth_req)
{
	fsm_msg_gatt_client_read_characteristic_t *msg = (fsm_msg_gatt_client_read_characteristic_t *) NEW(sizeof(fsm_msg_gatt_client_read_characteristic_t));
	msg->conn_id = conn_id;
	msg->handle = handle;
	msg->auth_req = auth_req;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_CHARACTERISTIC, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_using_characteristic_uuid(short conn_id, bt_uuid_t uuid, int s_handle, int e_handle, unsigned char auth_req)
{
	fsm_msg_gatt_client_read_using_characteristic_uuid_t *msg = (fsm_msg_gatt_client_read_using_characteristic_uuid_t *) NEW(sizeof(fsm_msg_gatt_client_read_using_characteristic_uuid_t));
	msg->conn_id = conn_id;
	msg->uuid = uuid;
	msg->s_handle = s_handle;
	msg->e_handle = e_handle;
	msg->auth_req = auth_req;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_USING_CHARACTERISTIC_UUID, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_descriptor(short conn_id, short handle, unsigned char auth_req)
{
	fsm_msg_gatt_client_read_descriptor_t *msg = (fsm_msg_gatt_client_read_descriptor_t *) NEW(sizeof(fsm_msg_gatt_client_read_descriptor_t));
	msg->conn_id = conn_id;
	msg->handle = handle;
	msg->auth_req = auth_req;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_DESCRIPTOR, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_write_characteristic(short conn_id, short handle, int write_type, unsigned char auth_req, short value_len, unsigned char * value)
{
	fsm_msg_gatt_client_write_characteristic_t *msg = (fsm_msg_gatt_client_write_characteristic_t *) NEW(sizeof(fsm_msg_gatt_client_write_characteristic_t));
	msg->conn_id = conn_id;
	msg->handle = handle;
	msg->write_type = write_type;
	msg->auth_req = auth_req;
	msg->value_len = value_len;
	if (msg->value_len > 0) {
		msg->value = (unsigned char *) NEW(msg->value_len);

		if (msg->value) {
			memcpy(msg->value, value, msg->value_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_WRITE_CHARACTERISTIC, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_write_descriptor(short conn_id, short handle, unsigned char auth_req, short value_len, unsigned char * value)
{
	fsm_msg_gatt_client_write_descriptor_t *msg = (fsm_msg_gatt_client_write_descriptor_t *) NEW(sizeof(fsm_msg_gatt_client_write_descriptor_t));
	msg->conn_id = conn_id;
	msg->handle = handle;
	msg->auth_req = auth_req;
	msg->value_len = value_len;
	if (msg->value_len > 0) {
		msg->value = (unsigned char *) NEW(msg->value_len);

		if (msg->value) {
			memcpy(msg->value, value, msg->value_len);
		}
	}
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_WRITE_DESCRIPTOR, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_execute_write(short conn_id, unsigned char execute)
{
	fsm_msg_gatt_client_execute_write_t *msg = (fsm_msg_gatt_client_execute_write_t *) NEW(sizeof(fsm_msg_gatt_client_execute_write_t));
	msg->conn_id = conn_id;
	msg->execute = execute;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_EXECUTE_WRITE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_register_for_notifications(unsigned char client_if, bt_bdaddr_t address, short handle, unsigned char enable)
{
	fsm_msg_gatt_client_register_for_notifications_t *msg = (fsm_msg_gatt_client_register_for_notifications_t *) NEW(sizeof(fsm_msg_gatt_client_register_for_notifications_t));
	msg->client_if = client_if;
	msg->address = address;
	msg->handle = handle;
	msg->enable = enable;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_REGISTER_FOR_NOTIFICATIONS, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_read_remote_rssi(unsigned char client_if, bt_bdaddr_t address)
{
	fsm_msg_gatt_client_read_remote_rssi_t *msg = (fsm_msg_gatt_client_read_remote_rssi_t *) NEW(sizeof(fsm_msg_gatt_client_read_remote_rssi_t));
	msg->client_if = client_if;
	msg->address = address;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_READ_REMOTE_RSSI, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_client_configure_mtu(short conn_id, int mtu)
{
	fsm_msg_gatt_client_configure_mtu_t *msg = (fsm_msg_gatt_client_configure_mtu_t *) NEW(sizeof(fsm_msg_gatt_client_configure_mtu_t));
	msg->conn_id = conn_id;
	msg->mtu = mtu;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CLIENT_CONFIGURE_MTU, (void *)msg);

	return BT_STATUS_SUCCESS;
}

bt_status_t app_gatt_connection_parameter_update(unsigned char client_if, bt_bdaddr_t address, int min_interval, int max_interval, int latency, int timeout)
{
	fsm_msg_gatt_connection_parameter_update_t *msg = (fsm_msg_gatt_connection_parameter_update_t *) NEW(sizeof(fsm_msg_gatt_connection_parameter_update_t));
	msg->client_if = client_if;
	msg->address = address;
	msg->min_interval = min_interval;
	msg->max_interval = max_interval;
	msg->latency = latency;
	msg->timeout = timeout;
	gatt_fsm_send_msg(FSM_EV_CMD_GATT_CONNECTION_PARAMETER_UPDATE, (void *)msg);

	return BT_STATUS_SUCCESS;
}

void app_gatt_client_on_device_connected(struct HCI_AddressStru *addr, UINT16 acl_hdl)
{
	hlp_gatt_fsm_client_on_device_connected(addr, acl_hdl);
}

void app_gatt_client_on_device_disconnected(struct HCI_AddressStru *addr, UINT16 acl_hdl)
{
	hlp_gatt_fsm_client_on_device_disconnected(addr, acl_hdl);
}

