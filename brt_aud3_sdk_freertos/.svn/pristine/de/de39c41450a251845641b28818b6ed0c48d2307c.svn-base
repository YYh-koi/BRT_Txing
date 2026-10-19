#include <stdio.h>
#include <string.h>
#include "at_event_gatt_DTTMode.h"
#include "at_process_gatt_DTTMode.h"
#include "bt_event.h"
#include "auto_gatt.h"
#include "auto_gatt_client.h"
#include "att_ui.h"

extern gatt_data_transport_t *get_gatt_DTTMode_data(); // at_process_gatt_DTTMode.c

int bt_event_gatt_DTTMode_indicate(unsigned int event_code, void *param, char *event_buffer, int AT_len)
{
	evt_param_notify_t *notif_data = (evt_param_notify_t *) param;
	int cur_len;
	short event_len;
	gatt_data_transport_t * DTTMode = get_gatt_DTTMode_data();

	if (notif_data == NULL || DTTMode == NULL || !(DTTMode->is_DTTMode)) {
		return -1;
	}

	if (event_code == BT_EV_GATT_NOTIFY) {
		snprintf(event_buffer, AT_len, "L%02X%02X%02X%02X%02X%02X",
		         notif_data->address.address[5], notif_data->address.address[4], notif_data->address.address[3],
		         notif_data->address.address[2], notif_data->address.address[1], notif_data->address.address[0]);
		snprintf(event_buffer + strlen(event_buffer),
		         AT_len - strlen(event_buffer), "%02X", notif_data->dataLen);
		cur_len = strlen(event_buffer);
		event_len = AT_len - cur_len - 1;
		event_len = notif_data->dataLen > event_len ? event_len : notif_data->dataLen;
		memcpy(event_buffer + cur_len, notif_data->data, event_len);

		return cur_len + event_len;
	} else if (event_code == BT_EV_GATT_CLIENT_REGISTERED) {
		evt_param_client_registered_t *p_client_registered = (evt_param_client_registered_t*) param;
		memset(event_buffer, 0, AT_len);
		DTTMode->client_if = p_client_registered->client_if;
		app_gatt_client_connect(DTTMode->client_if, DTTMode->address, 1, 0);

		return 0;
	} else if (event_code == BT_EV_GATT_CONNECTED) {
		evt_param_connected_t *p_connected = (evt_param_connected_t*) param;
		short service_handle = 0x1B;
		short char_handle = 0x1C;
		short descr_handle = 0x1D;
		short service_uuid = 0xFF00;
		short char_uuid = 0xFF01;
		short descr_uuid = 0x2902;
		struct ATT_UUIDStru out;
		ATT_UUID_SetU2(&out, service_uuid);
		bt_uuid_t service_uuid16;
		memcpy(&service_uuid16, &out.u, 16);

		ATT_UUID_SetU2(&out, char_uuid);
		bt_uuid_t char_uuid16;
		memcpy(&char_uuid16, &out.u, 16);

		ATT_UUID_SetU2(&out, descr_uuid);
		bt_uuid_t descr_uuid16;
		memcpy(&descr_uuid16, &out.u, 16);
		unsigned char value[2] = {2, 0};

		if (p_connected == NULL) {
			return 0;
		}

		snprintf(event_buffer, AT_len, "KE%02X%02X%02X%02X%02X%02X%02X",
		         (unsigned char) p_connected->status,
		         DTTMode->address.address[5],
		         DTTMode->address.address[4],
		         DTTMode->address.address[3],
		         DTTMode->address.address[2],
		         DTTMode->address.address[1],
		         DTTMode->address.address[0]);
		DTTMode->conn_id = p_connected->connId;
		app_gatt_client_write_descriptor(DTTMode->conn_id, /*service_type*/1, service_handle, service_uuid16,
		                                 char_handle, char_uuid16, descr_handle, descr_uuid16, /*XXX write_type*/0, /*XXX auth_req*/0, 2, value);

		return 0;
	} else if (event_code == BT_EV_GATT_DISCONNECTED) {
		evt_param_disconnected_t *p_disconnected = (evt_param_disconnected_t *) param;

		if (p_disconnected == NULL) {
			return 0;
		}

		snprintf(event_buffer, AT_len, "KF%02X%02X%02X%02X%02X%02X%02X",
		         (unsigned char) p_disconnected->status,
		         DTTMode->address.address[5],
		         DTTMode->address.address[4],
		         DTTMode->address.address[3],
		         DTTMode->address.address[2],
		         DTTMode->address.address[1],
		         DTTMode->address.address[0]);
		DTTMode->conn_id = p_disconnected->connId;
		app_gatt_client_unregister_client(DTTMode->client_if);
		memset(DTTMode, 0, sizeof(gatt_data_transport_t));

		return 0;
	} else if (event_code == BT_EV_GATT_WRITE_CHARACTERISTIC) {
		memset(event_buffer, 0, AT_len);
		return 0;
	}

	return -1;
}

