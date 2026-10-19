#ifndef _APP_MAP_H_
#define _APP_MAP_H_
#include "btdatatype.h"
#include "bt_event.h"
#include "profile.h"

enum app_map_event_type {
    EV_MAP_CONNECTION_EVENT,			/* connected or disconnected */
    EV_MAP_NEW_SMS_ARRIVED, 			/* new SMS notification */
    EV_MAP_RETRIEVE_SMS_RESULT, 		/* the result indication of retrive command */
    EV_MAP_GET_SMS_LIST_RESULT, 		/* the result indication of getting SMS list command */
    EV_MAP_SEND_SMS_RESULT,			/* the result indication of sending SMS command */
    EV_MAP_DELETE_SMS_RESULT, 		        /* the result indication of deleting SMS command */
    EV_MAP_BMSG_DATA_IND,
    EV_MAP_GET_SMS_LIST_COMPLETE,
};

enum app_map_connection_result_type {
    MAP_RESULT_CONNECTED,
    MAP_RESULT_CONNECT_FAILED,
    MAP_RESULT_DISCONNECTED,
    MAP_RESULT_DISCONNECT_FAILED
};

enum app_map_operation_result_type {
    MAP_RESULT_OK,
    MAP_RESULT_INVALID_PARAMETER,
    MAP_RESULT_CONNECTON_NOT_EXIST,
    MAP_RESULT_MEMORY_ERROR,
    MAP_RESULT_GML_COMPLETE,
};

typedef struct tag_app_map_sms_item {
	long handle;
	char *subject;
	char *date_time;
	char *sender_name;
	char *sender_address;
	char *recipient_name;
	char *recipient_address;
	UINT8 type; /* 1 for EMAIL, 2 for SMS_GSM, 3 for SMS_CDMA, 4 for MMS */
	int size;
	int readed; /* 1 for READ, 0 for UNREAD */
	UINT8 *content;
} app_map_sms_item_t;

typedef void (*app_map_event_callback)(int event, UINT8 result, UINT8 *pdu, UINT16 length);

extern profile_t g_map_profile;

void app_map_init(module_event_cbk_t event_cbk);
void app_map_done();
void app_map_register_callback(app_map_event_callback callback);
void app_map_connect(UINT8 *bd);
void app_map_disconnect();
void app_map_get_sms_list(UINT16 offset, UINT16 max_count);
void app_map_get_sms_list_ex(UINT8 type, UINT16 offset, UINT16 max_count);
void app_map_send_sms(app_map_sms_item_t *sms);
void app_map_get_message(UINT8 *msg_handle);
void app_map_delete_sms(UINT8 *msg_handle);
void app_map_retrieve_sms(long handle);

#endif //_APP_MAP_H_

