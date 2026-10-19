#include <stdio.h>
#include "at_event_gatt.h"
#include "at_event_gatt_DTTMode.h"
#include "auto_commfunc.h"
#include "bt_event.h"
#include "auto_gatt_event.h"
#define TAG "at_event_gatt"
#include "brt_trace.h"

#define MAX_GATT_DEVICE_LEN (62)

/**
  * process the event related with GATT and convert it to AT event string.
  * If the At event is length-based(raw data), return the total event length. otherwise return 0.
  */
UINT8 at_event_gatt_indicate(unsigned int event_code, void *param, char *event_buffer, int AT_len)
{
#ifdef CONFIG_GATT

	int len = bt_event_gatt_DTTMode_indicate(event_code, param, event_buffer, AT_len);

	if (len > 0) {
		return len;
	} else if (len == 0) {
		return 0;
	}

	switch (event_code) {
	case BT_EV_GATT_CLIENT_REGISTERED: {
		evt_param_client_registered_t *p_client_registered = (evt_param_client_registered_t*) param;

		unsigned char status;
		unsigned char client_if;
		bt_uuid_t Uuid;

		if (p_client_registered == NULL) {
			return 0;
		}

		status = p_client_registered->status;
		client_if = p_client_registered->client_if;
		Uuid = p_client_registered->Uuid;
		snprintf(event_buffer, AT_len, "KA%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         status, client_if,
		         Uuid.uu[15], Uuid.uu[14], Uuid.uu[13], Uuid.uu[12],
		         Uuid.uu[11], Uuid.uu[10], Uuid.uu[9], Uuid.uu[8],
		         Uuid.uu[7], Uuid.uu[6], Uuid.uu[5], Uuid.uu[4],
		         Uuid.uu[3], Uuid.uu[2], Uuid.uu[1], Uuid.uu[0]);
		break;
	}
	case BT_EV_GATT_SCAN_RESULT: {
		evt_param_scan_result_t *p_scan_result = (evt_param_scan_result_t*) param;

		bt_bdaddr_t address;
		unsigned char rssi;
		short adv_dataLen;
		unsigned char * adv_data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_scan_result == NULL) {
			return 0;
		}

		address = p_scan_result->address;
		rssi = p_scan_result->rssi;
		adv_dataLen = p_scan_result->adv_dataLen;
		adv_data = p_scan_result->adv_data;
		raw_bytes_to_str_bytes(adv_data, adv_dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(adv_data, adv_dataLen, (unsigned char *) convert_str, &convert_len);
		adv_dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KB%02X%02X%02X%02X%02X%02X%02X%04X%s",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , rssi, adv_dataLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_SCAN_ADDRESS_NAME: {
		evt_param_scan_address_name_t *p_scan_address_name = (evt_param_scan_address_name_t*) param;

		bt_bdaddr_t address;
		short nameLen;
		unsigned char * name;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_scan_address_name == NULL) {
			return 0;
		}

		address = p_scan_address_name->address;
		nameLen = p_scan_address_name->nameLen;
		name = p_scan_address_name->name;
		raw_bytes_to_str_bytes(name, nameLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(name, nameLen, (unsigned char *) convert_str, &convert_len);
		nameLen = convert_len;

		snprintf(event_buffer, AT_len, "KC%02X%02X%02X%02X%02X%02X%04X%s",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , nameLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_NOTIFY: {
		evt_param_notify_t *p_notify = (evt_param_notify_t*) param;

		short connId;
		bt_bdaddr_t address;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		unsigned char isNotify;
		short dataLen;
		unsigned char * data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_notify == NULL) {
			return 0;
		}

		connId = p_notify->connId;
		address = p_notify->address;
		srvcType = p_notify->srvcType;
		srvcInstId = p_notify->srvcInstId;
		srvcUuid = p_notify->srvcUuid;
		charInstId = p_notify->charInstId;
		charUuid = p_notify->charUuid;
		isNotify = p_notify->isNotify;
		dataLen = p_notify->dataLen;
		data = p_notify->data;
		raw_bytes_to_str_bytes(data, dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(data, dataLen, (unsigned char *) convert_str, &convert_len);
		dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KD%04X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%s",
		         connId,
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , srvcType, srvcInstId,
		         srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , isNotify, dataLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_CONNECTED: {
		evt_param_connected_t *p_connected = (evt_param_connected_t*) param;

		unsigned char client_if;
		short connId;
		unsigned char status;
		bt_bdaddr_t address;

		if (p_connected == NULL) {
			return 0;
		}

		client_if = p_connected->client_if;
		connId = p_connected->connId;
		status = p_connected->status;
		address = p_connected->address;
		snprintf(event_buffer, AT_len, "KE%02X%04X%02X%02X%02X%02X%02X%02X%02X",
		         client_if, connId, status,
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		        );
		break;
	}
	case BT_EV_GATT_DISCONNECTED: {
		evt_param_disconnected_t *p_disconnected = (evt_param_disconnected_t*) param;

		unsigned char client_if;
		short connId;
		unsigned char status;
		bt_bdaddr_t address;

		if (p_disconnected == NULL) {
			return 0;
		}

		client_if = p_disconnected->client_if;
		connId = p_disconnected->connId;
		status = p_disconnected->status;
		address = p_disconnected->address;
		snprintf(event_buffer, AT_len, "KF%02X%04X%02X%02X%02X%02X%02X%02X%02X",
		         client_if, connId, status,
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		        );
		break;
	}
	case BT_EV_GATT_SEARCH_COMPLETED: {
		evt_param_search_completed_t *p_search_completed = (evt_param_search_completed_t*) param;

		short connId;
		unsigned char status;

		if (p_search_completed == NULL) {
			return 0;
		}

		connId = p_search_completed->connId;
		status = p_search_completed->status;
		snprintf(event_buffer, AT_len, "KG%04X%02X",
		         connId, status);
		break;
	}
	case BT_EV_GATT_SEARCH_RESULT: {
		evt_param_search_result_t *p_search_result = (evt_param_search_result_t*) param;

		short connId;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;

		if (p_search_result == NULL) {
			return 0;
		}

		connId = p_search_result->connId;
		srvcType = p_search_result->srvcType;
		srvcInstId = p_search_result->srvcInstId;
		srvcUuid = p_search_result->srvcUuid;
		snprintf(event_buffer, AT_len, "KH%04X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, srvcType, srvcInstId,
		         srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		        );
		break;
	}
	case BT_EV_GATT_GET_CHARACTERISTIC: {
		evt_param_get_characteristic_t *p_get_characteristic = (evt_param_get_characteristic_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		unsigned char charProp;

		if (p_get_characteristic == NULL) {
			return 0;
		}

		connId = p_get_characteristic->connId;
		status = p_get_characteristic->status;
		srvcType = p_get_characteristic->srvcType;
		srvcInstId = p_get_characteristic->srvcInstId;
		srvcUuid = p_get_characteristic->srvcUuid;
		charInstId = p_get_characteristic->charInstId;
		charUuid = p_get_characteristic->charUuid;
		charProp = p_get_characteristic->charProp;
		snprintf(event_buffer, AT_len, "KI%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , charProp);
		break;
	}
	case BT_EV_GATT_GET_DESCRIPTOR: {
		evt_param_get_descriptor_t *p_get_descriptor = (evt_param_get_descriptor_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		short descrInstId;
		bt_uuid_t descrUuid;

		if (p_get_descriptor == NULL) {
			return 0;
		}

		connId = p_get_descriptor->connId;
		status = p_get_descriptor->status;
		srvcType = p_get_descriptor->srvcType;
		srvcInstId = p_get_descriptor->srvcInstId;
		srvcUuid = p_get_descriptor->srvcUuid;
		charInstId = p_get_descriptor->charInstId;
		charUuid = p_get_descriptor->charUuid;
		descrInstId = p_get_descriptor->descrInstId;
		descrUuid = p_get_descriptor->descrUuid;
		snprintf(event_buffer, AT_len, "KJ%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , descrInstId,
		         descrUuid.uu[15], descrUuid.uu[14], descrUuid.uu[13], descrUuid.uu[12],
		         descrUuid.uu[11], descrUuid.uu[10], descrUuid.uu[9], descrUuid.uu[8],
		         descrUuid.uu[7], descrUuid.uu[6], descrUuid.uu[5], descrUuid.uu[4],
		         descrUuid.uu[3], descrUuid.uu[2], descrUuid.uu[1], descrUuid.uu[0]);
		break;
	}
	case BT_EV_GATT_GET_INCLUDED_SERVICE: {
		evt_param_get_included_service_t *p_get_included_service = (evt_param_get_included_service_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		unsigned char inclSrvcType;
		short inclSrvcInstId;
		bt_uuid_t inclSrvcUuid;

		if (p_get_included_service == NULL) {
			return 0;
		}

		connId = p_get_included_service->connId;
		status = p_get_included_service->status;
		srvcType = p_get_included_service->srvcType;
		srvcInstId = p_get_included_service->srvcInstId;
		srvcUuid = p_get_included_service->srvcUuid;
		inclSrvcType = p_get_included_service->inclSrvcType;
		inclSrvcInstId = p_get_included_service->inclSrvcInstId;
		inclSrvcUuid = p_get_included_service->inclSrvcUuid;
		snprintf(event_buffer, AT_len, "KK%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , inclSrvcType, inclSrvcInstId,
		         inclSrvcUuid.uu[15], inclSrvcUuid.uu[14], inclSrvcUuid.uu[13], inclSrvcUuid.uu[12],
		         inclSrvcUuid.uu[11], inclSrvcUuid.uu[10], inclSrvcUuid.uu[9], inclSrvcUuid.uu[8],
		         inclSrvcUuid.uu[7], inclSrvcUuid.uu[6], inclSrvcUuid.uu[5], inclSrvcUuid.uu[4],
		         inclSrvcUuid.uu[3], inclSrvcUuid.uu[2], inclSrvcUuid.uu[1], inclSrvcUuid.uu[0]);
		break;
	}
	case BT_EV_GATT_REGISTER_FOR_NOTIFICATIONS: {
		evt_param_register_for_notifications_t *p_register_for_notifications = (evt_param_register_for_notifications_t*) param;

		short connId;
		unsigned char status;
		unsigned char registered;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;

		if (p_register_for_notifications == NULL) {
			return 0;
		}

		connId = p_register_for_notifications->connId;
		status = p_register_for_notifications->status;
		registered = p_register_for_notifications->registered;
		srvcType = p_register_for_notifications->srvcType;
		srvcInstId = p_register_for_notifications->srvcInstId;
		srvcUuid = p_register_for_notifications->srvcUuid;
		charInstId = p_register_for_notifications->charInstId;
		charUuid = p_register_for_notifications->charUuid;
		snprintf(event_buffer, AT_len, "KL%04X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, registered, srvcType
		         , srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]);
		break;
	}
	case BT_EV_GATT_READ_CHARACTERISTIC: {
		evt_param_read_characteristic_t *p_read_characteristic = (evt_param_read_characteristic_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		unsigned char charType;
		short dataLen;
		unsigned char * data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_read_characteristic == NULL) {
			return 0;
		}

		connId = p_read_characteristic->connId;
		status = p_read_characteristic->status;
		srvcType = p_read_characteristic->srvcType;
		srvcInstId = p_read_characteristic->srvcInstId;
		srvcUuid = p_read_characteristic->srvcUuid;
		charInstId = p_read_characteristic->charInstId;
		charUuid = p_read_characteristic->charUuid;
		charType = p_read_characteristic->charType;
		dataLen = p_read_characteristic->dataLen;
		data = p_read_characteristic->data;
		raw_bytes_to_str_bytes(data, dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(data, dataLen, (unsigned char *) convert_str, &convert_len);
		dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KM%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%s",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , charType, dataLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_WRITE_CHARACTERISTIC: {
		evt_param_write_characteristic_t *p_write_characteristic = (evt_param_write_characteristic_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;

		if (p_write_characteristic == NULL) {
			return 0;
		}

		connId = p_write_characteristic->connId;
		status = p_write_characteristic->status;
		srvcType = p_write_characteristic->srvcType;
		srvcInstId = p_write_characteristic->srvcInstId;
		srvcUuid = p_write_characteristic->srvcUuid;
		charInstId = p_write_characteristic->charInstId;
		charUuid = p_write_characteristic->charUuid;
		snprintf(event_buffer, AT_len, "KN%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]);
		break;
	}
	case BT_EV_GATT_EXECUTE_COMPLETED: {
		evt_param_execute_completed_t *p_execute_completed = (evt_param_execute_completed_t*) param;

		short connId;
		unsigned char status;

		if (p_execute_completed == NULL) {
			return 0;
		}

		connId = p_execute_completed->connId;
		status = p_execute_completed->status;
		snprintf(event_buffer, AT_len, "KO%04X%02X",
		         connId, status);
		break;
	}
	case BT_EV_GATT_READ_DESCRIPTOR: {
		evt_param_read_descriptor_t *p_read_descriptor = (evt_param_read_descriptor_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		short descrInstId;
		bt_uuid_t descrUuid;
		unsigned char charType;
		short dataLen;
		unsigned char * data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_read_descriptor == NULL) {
			return 0;
		}

		connId = p_read_descriptor->connId;
		status = p_read_descriptor->status;
		srvcType = p_read_descriptor->srvcType;
		srvcInstId = p_read_descriptor->srvcInstId;
		srvcUuid = p_read_descriptor->srvcUuid;
		charInstId = p_read_descriptor->charInstId;
		charUuid = p_read_descriptor->charUuid;
		descrInstId = p_read_descriptor->descrInstId;
		descrUuid = p_read_descriptor->descrUuid;
		charType = p_read_descriptor->charType;
		dataLen = p_read_descriptor->dataLen;
		data = p_read_descriptor->data;
		raw_bytes_to_str_bytes(data, dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(data, dataLen, (unsigned char *) convert_str, &convert_len);
		dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KP%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%s",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , descrInstId,
		         descrUuid.uu[15], descrUuid.uu[14], descrUuid.uu[13], descrUuid.uu[12],
		         descrUuid.uu[11], descrUuid.uu[10], descrUuid.uu[9], descrUuid.uu[8],
		         descrUuid.uu[7], descrUuid.uu[6], descrUuid.uu[5], descrUuid.uu[4],
		         descrUuid.uu[3], descrUuid.uu[2], descrUuid.uu[1], descrUuid.uu[0]
		         , charType, dataLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_WRITE_DESCRIPTOR: {
		evt_param_write_descriptor_t *p_write_descriptor = (evt_param_write_descriptor_t*) param;

		short connId;
		unsigned char status;
		unsigned char srvcType;
		short srvcInstId;
		bt_uuid_t srvcUuid;
		short charInstId;
		bt_uuid_t charUuid;
		short descrInstId;
		bt_uuid_t descrUuid;

		if (p_write_descriptor == NULL) {
			return 0;
		}

		connId = p_write_descriptor->connId;
		status = p_write_descriptor->status;
		srvcType = p_write_descriptor->srvcType;
		srvcInstId = p_write_descriptor->srvcInstId;
		srvcUuid = p_write_descriptor->srvcUuid;
		charInstId = p_write_descriptor->charInstId;
		charUuid = p_write_descriptor->charUuid;
		descrInstId = p_write_descriptor->descrInstId;
		descrUuid = p_write_descriptor->descrUuid;
		snprintf(event_buffer, AT_len, "KQ%04X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         connId, status, srvcType, srvcInstId
		         , srvcUuid.uu[15], srvcUuid.uu[14], srvcUuid.uu[13], srvcUuid.uu[12],
		         srvcUuid.uu[11], srvcUuid.uu[10], srvcUuid.uu[9], srvcUuid.uu[8],
		         srvcUuid.uu[7], srvcUuid.uu[6], srvcUuid.uu[5], srvcUuid.uu[4],
		         srvcUuid.uu[3], srvcUuid.uu[2], srvcUuid.uu[1], srvcUuid.uu[0]
		         , charInstId,
		         charUuid.uu[15], charUuid.uu[14], charUuid.uu[13], charUuid.uu[12],
		         charUuid.uu[11], charUuid.uu[10], charUuid.uu[9], charUuid.uu[8],
		         charUuid.uu[7], charUuid.uu[6], charUuid.uu[5], charUuid.uu[4],
		         charUuid.uu[3], charUuid.uu[2], charUuid.uu[1], charUuid.uu[0]
		         , descrInstId,
		         descrUuid.uu[15], descrUuid.uu[14], descrUuid.uu[13], descrUuid.uu[12],
		         descrUuid.uu[11], descrUuid.uu[10], descrUuid.uu[9], descrUuid.uu[8],
		         descrUuid.uu[7], descrUuid.uu[6], descrUuid.uu[5], descrUuid.uu[4],
		         descrUuid.uu[3], descrUuid.uu[2], descrUuid.uu[1], descrUuid.uu[0]);
		break;
	}
	case BT_EV_GATT_READ_REMOTE_RSSI: {
		evt_param_read_remote_rssi_t *p_read_remote_rssi = (evt_param_read_remote_rssi_t*) param;

		unsigned char client_if;
		bt_bdaddr_t address;
		unsigned char rssi;
		unsigned char status;

		if (p_read_remote_rssi == NULL) {
			return 0;
		}

		client_if = p_read_remote_rssi->client_if;
		address = p_read_remote_rssi->address;
		rssi = p_read_remote_rssi->rssi;
		status = p_read_remote_rssi->status;
		snprintf(event_buffer, AT_len, "KR%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         client_if,
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , rssi, status);
		break;
	}
	case BT_EV_GATT_RECEIVE_DATA_FROM_CENTRAL: {
		evt_param_receive_data_from_central_t *p_receive_data_from_central = (evt_param_receive_data_from_central_t*) param;

		short connId;
		bt_bdaddr_t address;
		short dataLen;
		unsigned char * data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_receive_data_from_central == NULL) {
			return 0;
		}

		connId = p_receive_data_from_central->connId;
		address = p_receive_data_from_central->address;
		dataLen = p_receive_data_from_central->dataLen;
		data = p_receive_data_from_central->data;
		raw_bytes_to_str_bytes(data, dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(data, dataLen, (unsigned char *) convert_str, &convert_len);
		dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KS%04X%02X%02X%02X%02X%02X%02X%04X%s",
		         connId,
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , dataLen, convert_str);
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_CONFIGURE_MTU: {
		evt_param_configure_mtu_t *p_configure_mtu = (evt_param_configure_mtu_t*) param;

		int conn_id;
		unsigned char status;
		int mtu;

		if (p_configure_mtu == NULL) {
			return 0;
		}

		conn_id = p_configure_mtu->conn_id;
		status = p_configure_mtu->status;
		mtu = p_configure_mtu->mtu;
		snprintf(event_buffer, AT_len, "KT%08X%02X%08X",
		         conn_id, status, mtu);
		break;
	}
	case BT_EV_GATT_ADVERTISE_CALLBACK: {
		evt_param_advertise_callback_t *p_advertise_callback = (evt_param_advertise_callback_t*) param;

		unsigned char status;
		unsigned char client_if;

		if (p_advertise_callback == NULL) {
			return 0;
		}

		status = p_advertise_callback->status;
		client_if = p_advertise_callback->client_if;
		snprintf(event_buffer, AT_len, "KZA%02X%02X",
		         status, client_if);
		break;
	}
	case BT_EV_GATT_SCAN_FILTER_CONFIG: {
		evt_param_scan_filter_config_t *p_scan_filter_config = (evt_param_scan_filter_config_t*) param;

		int action;
		int client_if;
		unsigned char status;
		int filt_type;
		int avbl_space;

		if (p_scan_filter_config == NULL) {
			return 0;
		}

		action = p_scan_filter_config->action;
		client_if = p_scan_filter_config->client_if;
		status = p_scan_filter_config->status;
		filt_type = p_scan_filter_config->filt_type;
		avbl_space = p_scan_filter_config->avbl_space;
		snprintf(event_buffer, AT_len, "KZB%08X%08X%02X%08X%08X",
		         action, client_if, status, filt_type
		         , avbl_space
		        );
		break;
	}
	case BT_EV_GATT_SCAN_FILTER_PARAMS_CONFIGURED: {
		evt_param_scan_filter_params_configured_t *p_scan_filter_params_configured = (evt_param_scan_filter_params_configured_t*) param;

		int action;
		int client_if;
		unsigned char status;
		int avbl_space;

		if (p_scan_filter_params_configured == NULL) {
			return 0;
		}

		action = p_scan_filter_params_configured->action;
		client_if = p_scan_filter_params_configured->client_if;
		status = p_scan_filter_params_configured->status;
		avbl_space = p_scan_filter_params_configured->avbl_space;
		snprintf(event_buffer, AT_len, "KZC%08X%08X%02X%08X",
		         action, client_if, status, avbl_space
		        );
		break;
	}
	case BT_EV_GATT_SCAN_FILTER_ENABLE_DISABLED: {
		evt_param_scan_filter_enable_disabled_t *p_scan_filter_enable_disabled = (evt_param_scan_filter_enable_disabled_t*) param;

		int action;
		int client_if;
		unsigned char status;

		if (p_scan_filter_enable_disabled == NULL) {
			return 0;
		}

		action = p_scan_filter_enable_disabled->action;
		client_if = p_scan_filter_enable_disabled->client_if;
		status = p_scan_filter_enable_disabled->status;
		snprintf(event_buffer, AT_len, "KZD%08X%08X%02X",
		         action, client_if, status);
		break;
	}
	case BT_EV_GATT_ADVERTISE_INSTANCE_ENABLED: {
		evt_param_advertise_instance_enabled_t *p_advertise_instance_enabled = (evt_param_advertise_instance_enabled_t*) param;

		int client_if;
		unsigned char status;

		if (p_advertise_instance_enabled == NULL) {
			return 0;
		}

		client_if = p_advertise_instance_enabled->client_if;
		status = p_advertise_instance_enabled->status;
		snprintf(event_buffer, AT_len, "KZE%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_ADVERTISE_DATA_UPDATED: {
		evt_param_advertise_data_updated_t *p_advertise_data_updated = (evt_param_advertise_data_updated_t*) param;

		int client_if;
		unsigned char status;

		if (p_advertise_data_updated == NULL) {
			return 0;
		}

		client_if = p_advertise_data_updated->client_if;
		status = p_advertise_data_updated->status;
		snprintf(event_buffer, AT_len, "KZF%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_ADVERTISE_DATA_SET: {
		evt_param_advertise_data_set_t *p_advertise_data_set = (evt_param_advertise_data_set_t*) param;

		int client_if;
		unsigned char status;

		if (p_advertise_data_set == NULL) {
			return 0;
		}

		client_if = p_advertise_data_set->client_if;
		status = p_advertise_data_set->status;
		snprintf(event_buffer, AT_len, "KZG%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_ADVERTISE_INSTANCE_DISABLED: {
		evt_param_advertise_instance_disabled_t *p_advertise_instance_disabled = (evt_param_advertise_instance_disabled_t*) param;

		int client_if;
		unsigned char status;

		if (p_advertise_instance_disabled == NULL) {
			return 0;
		}

		client_if = p_advertise_instance_disabled->client_if;
		status = p_advertise_instance_disabled->status;
		snprintf(event_buffer, AT_len, "KZH%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_CLIENT_CONGESTION: {
		evt_param_client_congestion_t *p_client_congestion = (evt_param_client_congestion_t*) param;

		int conn_id;
		unsigned char congested;

		if (p_client_congestion == NULL) {
			return 0;
		}

		conn_id = p_client_congestion->conn_id;
		congested = p_client_congestion->congested;
		snprintf(event_buffer, AT_len, "KZI%08X%02X",
		         conn_id, congested);
		break;
	}
	case BT_EV_GATT_BATCH_SCAN_STORAGE_CONFIGURED: {
		evt_param_batch_scan_storage_configured_t *p_batch_scan_storage_configured = (evt_param_batch_scan_storage_configured_t*) param;

		int client_if;
		unsigned char status;

		if (p_batch_scan_storage_configured == NULL) {
			return 0;
		}

		client_if = p_batch_scan_storage_configured->client_if;
		status = p_batch_scan_storage_configured->status;
		snprintf(event_buffer, AT_len, "KZJ%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_BATCH_SCAN_START_STOPPED: {
		evt_param_batch_scan_start_stopped_t *p_batch_scan_start_stopped = (evt_param_batch_scan_start_stopped_t*) param;

		int startstop_action;
		int client_if;
		unsigned char status;

		if (p_batch_scan_start_stopped == NULL) {
			return 0;
		}

		startstop_action = p_batch_scan_start_stopped->startstop_action;
		client_if = p_batch_scan_start_stopped->client_if;
		status = p_batch_scan_start_stopped->status;
		snprintf(event_buffer, AT_len, "KZK%08X%08X%02X",
		         startstop_action, client_if, status);
		break;
	}
	case BT_EV_GATT_BATCH_SCAN_REPORTS: {
		evt_param_batch_scan_reports_t *p_batch_scan_reports = (evt_param_batch_scan_reports_t*) param;

		int client_if;
		unsigned char status;
		int report_format;
		int num_records;
		short rep_dataLen;
		unsigned char * rep_data;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_batch_scan_reports == NULL) {
			return 0;
		}

		client_if = p_batch_scan_reports->client_if;
		status = p_batch_scan_reports->status;
		report_format = p_batch_scan_reports->report_format;
		num_records = p_batch_scan_reports->num_records;
		rep_dataLen = p_batch_scan_reports->rep_dataLen;
		rep_data = p_batch_scan_reports->rep_data;
		raw_bytes_to_str_bytes(rep_data, rep_dataLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(rep_data, rep_dataLen, (unsigned char *) convert_str, &convert_len);
		rep_dataLen = convert_len;

		snprintf(event_buffer, AT_len, "KZL%08X%02X%08X%08X%04X%s",
		         client_if, status, report_format, num_records
		         , rep_dataLen
		         , convert_str
		        );
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_BATCH_SCAN_THRESHOLD_CROSSED: {
		evt_param_batch_scan_threshold_crossed_t *p_batch_scan_threshold_crossed = (evt_param_batch_scan_threshold_crossed_t*) param;

		int client_if;

		if (p_batch_scan_threshold_crossed == NULL) {
			return 0;
		}

		client_if = p_batch_scan_threshold_crossed->client_if;
		snprintf(event_buffer, AT_len, "KZM%08X",
		         client_if);
		break;
	}
	case BT_EV_GATT_SCAN_PARAM_SETUP_COMPLETED: {
		evt_param_scan_param_setup_completed_t *p_scan_param_setup_completed = (evt_param_scan_param_setup_completed_t*) param;

		int client_if;
		unsigned char status;

		if (p_scan_param_setup_completed == NULL) {
			return 0;
		}

		client_if = p_scan_param_setup_completed->client_if;
		status = p_scan_param_setup_completed->status;
		snprintf(event_buffer, AT_len, "KZN%08X%02X",
		         client_if, status);
		break;
	}
	case BT_EV_GATT_SERVER_REGISTERED: {
		evt_param_server_registered_t *p_server_registered = (evt_param_server_registered_t*) param;

		unsigned char status;
		unsigned char server_if;
		bt_uuid_t uuid;

		if (p_server_registered == NULL) {
			return 0;
		}

		status = p_server_registered->status;
		server_if = p_server_registered->server_if;
		uuid = p_server_registered->uuid;
		snprintf(event_buffer, AT_len, "JA%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		         status, server_if,
		         uuid.uu[15], uuid.uu[14], uuid.uu[13], uuid.uu[12],
		         uuid.uu[11], uuid.uu[10], uuid.uu[9], uuid.uu[8],
		         uuid.uu[7], uuid.uu[6], uuid.uu[5], uuid.uu[4],
		         uuid.uu[3], uuid.uu[2], uuid.uu[1], uuid.uu[0]);
		break;
	}
	case BT_EV_GATT_CLIENT_CONNECTED: {
		evt_param_client_connected_t *p_client_connected = (evt_param_client_connected_t*) param;

		bt_bdaddr_t address;
		unsigned char connected;
		short conn_id;
		unsigned char server_if;

		if (p_client_connected == NULL) {
			return 0;
		}

		address = p_client_connected->address;
		connected = p_client_connected->connected;
		conn_id = p_client_connected->conn_id;
		server_if = p_client_connected->server_if;
		snprintf(event_buffer, AT_len, "JB%02X%02X%02X%02X%02X%02X%02X%04X%02X",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , connected, conn_id, server_if);
		break;
	}
	case BT_EV_GATT_SERVICE_ADDED: {
		evt_param_service_added_t *p_service_added = (evt_param_service_added_t*) param;

		unsigned char status;
		unsigned char server_if;
		unsigned char svc_type;
		short svc_inst_id;
		bt_uuid_t srvc_uuid;
		short handle;

		if (p_service_added == NULL) {
			return 0;
		}

		status = p_service_added->status;
		server_if = p_service_added->server_if;
		svc_type = p_service_added->svc_type;
		svc_inst_id = p_service_added->svc_inst_id;
		srvc_uuid = p_service_added->srvc_uuid;
		handle = p_service_added->handle;
		snprintf(event_buffer, AT_len, "JC%02X%02X%02X%04X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X",
		         status, server_if, svc_type, svc_inst_id
		         , srvc_uuid.uu[15], srvc_uuid.uu[14], srvc_uuid.uu[13], srvc_uuid.uu[12],
		         srvc_uuid.uu[11], srvc_uuid.uu[10], srvc_uuid.uu[9], srvc_uuid.uu[8],
		         srvc_uuid.uu[7], srvc_uuid.uu[6], srvc_uuid.uu[5], srvc_uuid.uu[4],
		         srvc_uuid.uu[3], srvc_uuid.uu[2], srvc_uuid.uu[1], srvc_uuid.uu[0]
		         , handle);
		break;
	}
	case BT_EV_GATT_INCLUDED_SERVICE_ADDED: {
		evt_param_included_service_added_t *p_included_service_added = (evt_param_included_service_added_t*) param;

		unsigned char status;
		unsigned char server_if;
		short srvc_handle;
		short incl_srvc_handle;

		if (p_included_service_added == NULL) {
			return 0;
		}

		status = p_included_service_added->status;
		server_if = p_included_service_added->server_if;
		srvc_handle = p_included_service_added->srvc_handle;
		incl_srvc_handle = p_included_service_added->incl_srvc_handle;
		snprintf(event_buffer, AT_len, "JD%02X%02X%04X%04X",
		         status, server_if, srvc_handle, incl_srvc_handle
		        );
		break;
	}
	case BT_EV_GATT_CHARACTERISTIC_ADDED: {
		evt_param_characteristic_added_t *p_characteristic_added = (evt_param_characteristic_added_t*) param;

		unsigned char status;
		unsigned char server_if;
		bt_uuid_t char_uuid;
		short srvc_handle;
		short char_handle;

		if (p_characteristic_added == NULL) {
			return 0;
		}

		status = p_characteristic_added->status;
		server_if = p_characteristic_added->server_if;
		char_uuid = p_characteristic_added->char_uuid;
		srvc_handle = p_characteristic_added->srvc_handle;
		char_handle = p_characteristic_added->char_handle;
		snprintf(event_buffer, AT_len, "JE%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%04X",
		         status, server_if,
		         char_uuid.uu[15], char_uuid.uu[14], char_uuid.uu[13], char_uuid.uu[12],
		         char_uuid.uu[11], char_uuid.uu[10], char_uuid.uu[9], char_uuid.uu[8],
		         char_uuid.uu[7], char_uuid.uu[6], char_uuid.uu[5], char_uuid.uu[4],
		         char_uuid.uu[3], char_uuid.uu[2], char_uuid.uu[1], char_uuid.uu[0]
		         , srvc_handle, char_handle);
		break;
	}
	case BT_EV_GATT_DESCRIPTOR_ADDED: {
		evt_param_descriptor_added_t *p_descriptor_added = (evt_param_descriptor_added_t*) param;

		unsigned char status;
		unsigned char server_if;
		bt_uuid_t descr_uuid;
		short srvc_handle;
		short descr_handle;

		if (p_descriptor_added == NULL) {
			return 0;
		}

		status = p_descriptor_added->status;
		server_if = p_descriptor_added->server_if;
		descr_uuid = p_descriptor_added->descr_uuid;
		srvc_handle = p_descriptor_added->srvc_handle;
		descr_handle = p_descriptor_added->descr_handle;
		snprintf(event_buffer, AT_len, "JF%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%04X",
		         status, server_if,
		         descr_uuid.uu[15], descr_uuid.uu[14], descr_uuid.uu[13], descr_uuid.uu[12],
		         descr_uuid.uu[11], descr_uuid.uu[10], descr_uuid.uu[9], descr_uuid.uu[8],
		         descr_uuid.uu[7], descr_uuid.uu[6], descr_uuid.uu[5], descr_uuid.uu[4],
		         descr_uuid.uu[3], descr_uuid.uu[2], descr_uuid.uu[1], descr_uuid.uu[0]
		         , srvc_handle, descr_handle);
		break;
	}
	case BT_EV_GATT_SERVICE_STARTED: {
		evt_param_service_started_t *p_service_started = (evt_param_service_started_t*) param;

		unsigned char status;
		unsigned char server_if;
		short srvc_handle;

		if (p_service_started == NULL) {
			return 0;
		}

		status = p_service_started->status;
		server_if = p_service_started->server_if;
		srvc_handle = p_service_started->srvc_handle;
		snprintf(event_buffer, AT_len, "JG%02X%02X%04X",
		         status, server_if, srvc_handle);
		break;
	}
	case BT_EV_GATT_SERVICE_STOPPED: {
		evt_param_service_stopped_t *p_service_stopped = (evt_param_service_stopped_t*) param;

		unsigned char status;
		unsigned char server_if;
		short srvc_handle;

		if (p_service_stopped == NULL) {
			return 0;
		}

		status = p_service_stopped->status;
		server_if = p_service_stopped->server_if;
		srvc_handle = p_service_stopped->srvc_handle;
		snprintf(event_buffer, AT_len, "JH%02X%02X%04X",
		         status, server_if, srvc_handle);
		break;
	}
	case BT_EV_GATT_SERVICE_DELETED: {
		evt_param_service_deleted_t *p_service_deleted = (evt_param_service_deleted_t*) param;

		unsigned char status;
		unsigned char server_if;
		short srvc_handle;

		if (p_service_deleted == NULL) {
			return 0;
		}

		status = p_service_deleted->status;
		server_if = p_service_deleted->server_if;
		srvc_handle = p_service_deleted->srvc_handle;
		snprintf(event_buffer, AT_len, "JI%02X%02X%04X",
		         status, server_if, srvc_handle);
		break;
	}
	case BT_EV_GATT_RESPONSE_SEND_COMPLETED: {
		evt_param_response_send_completed_t *p_response_send_completed = (evt_param_response_send_completed_t*) param;

		unsigned char status;
		short handle;

		if (p_response_send_completed == NULL) {
			return 0;
		}

		status = p_response_send_completed->status;
		handle = p_response_send_completed->handle;
		snprintf(event_buffer, AT_len, "JJ%02X%04X",
		         status, handle);
		break;
	}
	case BT_EV_GATT_ATTRIBUTE_READ: {
		evt_param_attribute_read_t *p_attribute_read = (evt_param_attribute_read_t*) param;

		bt_bdaddr_t address;
		short conn_id;
		short trans_id;
		short attr_handle;
		short offset;
		unsigned char is_long;

		if (p_attribute_read == NULL) {
			return 0;
		}

		address = p_attribute_read->address;
		conn_id = p_attribute_read->conn_id;
		trans_id = p_attribute_read->trans_id;
		attr_handle = p_attribute_read->attr_handle;
		offset = p_attribute_read->offset;
		is_long = p_attribute_read->is_long;
		snprintf(event_buffer, AT_len, "JK%02X%02X%02X%02X%02X%02X%04X%04X%04X%04X%02X",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , conn_id, trans_id, attr_handle, offset
		         , is_long
		        );
		break;
	}
	case BT_EV_GATT_ATTRIBUTE_WRITE: {
		evt_param_attribute_write_t *p_attribute_write = (evt_param_attribute_write_t*) param;

		bt_bdaddr_t address;
		short conn_id;
		short trans_id;
		short attr_handle;
		short offset;
		unsigned char need_rsp;
		unsigned char is_prep;
		short valueLen;
		unsigned char * value;
		unsigned short convert_len = 0;
		char *convert_str;

		if (p_attribute_write == NULL) {
			return 0;
		}

		address = p_attribute_write->address;
		conn_id = p_attribute_write->conn_id;
		trans_id = p_attribute_write->trans_id;
		attr_handle = p_attribute_write->attr_handle;
		offset = p_attribute_write->offset;
		need_rsp = p_attribute_write->need_rsp;
		is_prep = p_attribute_write->is_prep;
		valueLen = p_attribute_write->valueLen;
		value = p_attribute_write->value;
		raw_bytes_to_str_bytes(value, valueLen, NULL, &convert_len);

		if (convert_len <= 0) {
			break;
		}

		convert_str = (char *) NEW(convert_len);

		if (convert_str == NULL) {
			break;
		}

		raw_bytes_to_str_bytes(value, valueLen, (unsigned char *) convert_str, &convert_len);
		valueLen = convert_len;

		snprintf(event_buffer, AT_len, "JL%02X%02X%02X%02X%02X%02X%04X%04X%04X%04X%02X%02X%04X%s",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , conn_id, trans_id, attr_handle, offset
		         , need_rsp
		         , is_prep
		         , valueLen
		         , convert_str
		        );
		FREE(convert_str);
		break;
	}
	case BT_EV_GATT_EXECUTE_WRITE: {
		evt_param_execute_write_t *p_execute_write = (evt_param_execute_write_t*) param;

		bt_bdaddr_t address;
		short conn_id;
		short trans_id;
		unsigned char exec_write;

		if (p_execute_write == NULL) {
			return 0;
		}

		address = p_execute_write->address;
		conn_id = p_execute_write->conn_id;
		trans_id = p_execute_write->trans_id;
		exec_write = p_execute_write->exec_write;
		snprintf(event_buffer, AT_len, "JM%02X%02X%02X%02X%02X%02X%04X%04X%02X",
		         address.address[5], address.address[4], address.address[3],
		         address.address[2], address.address[1], address.address[0]
		         , conn_id, trans_id, exec_write);
		break;
	}
	case BT_EV_GATT_NOTIFICATION_SENT: {
		evt_param_notification_sent_t *p_notification_sent = (evt_param_notification_sent_t*) param;

		short conn_id;
		unsigned char status;

		if (p_notification_sent == NULL) {
			return 0;
		}

		conn_id = p_notification_sent->conn_id;
		status = p_notification_sent->status;
		snprintf(event_buffer, AT_len, "JN%04X%02X",
		         conn_id, status);
		break;
	}
	case BT_EV_GATT_SERVER_CONGESTION: {
		evt_param_server_congestion_t *p_server_congestion = (evt_param_server_congestion_t*) param;

		short conn_id;
		unsigned char congested;

		if (p_server_congestion == NULL) {
			return 0;
		}

		conn_id = p_server_congestion->conn_id;
		congested = p_server_congestion->congested;
		snprintf(event_buffer, AT_len, "JO%04X%02X",
		         conn_id, congested);
		break;
	}
	case BT_EV_GATT_MTU_CHANGED: {
		evt_param_mtu_changed_t *p_mtu_changed = (evt_param_mtu_changed_t*) param;

		short conn_id;
		short mtu;

		if (p_mtu_changed == NULL) {
			return 0;
		}

		conn_id = p_mtu_changed->conn_id;
		mtu = p_mtu_changed->mtu;
		snprintf(event_buffer, AT_len, "JP%04X%04X",
		         conn_id, mtu);
		break;
	}
	}
#endif //CONFIG_gatt
	return 0;
}

