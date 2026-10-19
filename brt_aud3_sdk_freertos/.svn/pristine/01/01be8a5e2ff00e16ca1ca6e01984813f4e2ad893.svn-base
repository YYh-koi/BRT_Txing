#include "auto_ancs.h"
#include "auto_at.h"
#include "brt_trace.h"
#include "ancs_client.h"

#define TAG "bse_ancs"
#define CONFIG_ANCS_DEBUG

#ifdef CONFIG_ANCS_DEBUG
#define CASE_RET(a) case a: return #a

static const char *get_notif_event_string(int ev)
{
	switch (ev) {
		CASE_RET(BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED);
		CASE_RET(BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED);
		CASE_RET(BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED);
	}

	return "unknown";
}

static const char *get_notif_category_id_string(int ev)
{
	switch (ev) {
		CASE_RET(BLE_ANCS_CATEGORY_ID_OTHER);
		CASE_RET(BLE_ANCS_CATEGORY_ID_INCOMING_CALL);
		CASE_RET(BLE_ANCS_CATEGORY_ID_MISSED_CALL);
		CASE_RET(BLE_ANCS_CATEGORY_ID_VOICE_MAIL);
		CASE_RET(BLE_ANCS_CATEGORY_ID_SOCIAL);
		CASE_RET(BLE_ANCS_CATEGORY_ID_SCHEDULE);
		CASE_RET(BLE_ANCS_CATEGORY_ID_EMAIL);
		CASE_RET(BLE_ANCS_CATEGORY_ID_NEWS);
		CASE_RET(BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS);
		CASE_RET(BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE);
		CASE_RET(BLE_ANCS_CATEGORY_ID_LOCATION);
		CASE_RET(BLE_ANCS_CATEGORY_ID_ENTERTAINMENT);
	}

	return "unknown";
}
#endif

static module_event_cbk_t s_ancs_event_cbk;

static void ancs_event_indicate(unsigned int event, void *param)
{
	if (s_ancs_event_cbk != NULL) {
		s_ancs_event_cbk((PROFILE_ID_ANCS << 8) | event, param);
	}
}

static void ble_ancs_evt_handler(ble_ancs_evt_t *event)
{
	ble_ancs_connection_state_t *connection_state;
	ble_ancs_ios_notification_t *notification;
	ble_ancs_notif_attribute_t  *notif_attribute;
	ble_ancs_app_attribute_t    *app_attribute;

	if (event == NULL) {
		return;
	}
	if (event->evt_type == BLE_ANCS_EVT_CONNECTION_STATE) {
		connection_state = &event->data.connection_state;
		int evt_conn = 1;//disconnect

		if(connection_state->connected)
			evt_conn = 3;
		//connection_state =  connection_state ? 3 : 1;

		bse_log_info(TAG,"BLE_ANCS_EVT_CONNECTION_STATE evt_conn = %d", evt_conn);
		ancs_event_indicate(EV_ANCS_CONNECTION_STATE, (void *)evt_conn);
	} else if (event->evt_type == BLE_ANCS_EVT_IOS_NOTIFICATION) {
		notification = &event->data.notification;
#ifdef CONFIG_ANCS_DEBUG
		bse_log_info(TAG, "event_id:%s, event_flag:0X%02X,category_id:%s",
		             get_notif_event_string(notification->event_id), (notification->event_flags),
		             get_notif_category_id_string(notification->category_id));
#endif

		ancs_event_indicate(EV_ANCS_IOS_NOTIFICATION, notification);
		bse_log_info(TAG,"app_ancs_get_app_attribute START");
		//app_ancs_get_app_attribute("com.tencent.xin", 37);
		app_ancs_get_notification_attribute(notification->notification_uid, 0x01, 65535);

	} else if (event->evt_type == BLE_ANCS_EVT_NOTIF_ATTRIBUTE) {
		notif_attribute = &event->data.notif_attribute;
		ancs_event_indicate(EV_ANCS_NOTIFICATION_ATTRIBUTE, notif_attribute);
	} else if (event->evt_type == BLE_ANCS_EVT_APP_ATTRIBUTE) {
		app_attribute = &event->data.app_attribute;
		ancs_event_indicate(EV_ANCS_APP_ATTRIBUTE, app_attribute);
	}
}

void ble_ancs_evt_handler_ex(UINT8 *data, int len)
{
	bse_log_info(TAG, "%s len = %d", __func__, len);
	int i, j=0;
	UINT8 att_event_buff[512] = {0};
	for(i=0; i<len; i++)
	{
		sprintf(&att_event_buff[j], "%02x", data[i]);
		j+=2;
		if(j >= 512)
			break;
	}

	ancs_event_indicate(EV_ANCS_NOTIFICATION_ATTRIBUTE, att_event_buff);
}

void app_ancs_start()
{
	ble_ancs_init(ble_ancs_evt_handler);
}

void app_ancs_stop()
{
	ble_ancs_done();
}

void app_ancs_init(module_event_cbk_t callback)
{
	s_ancs_event_cbk = callback;
}

void app_ancs_done()
{
	s_ancs_event_cbk = NULL;
}

void app_ancs_get_notification_attribute(unsigned char *uid, unsigned char attr_id, int attr_max_len)
{
	ble_ancs_attr_list_t attr_list[3];
	attr_list[0].attribute_id = 0x01;
	attr_list[0].attribute_len = 0xFFFF;

	attr_list[1].attribute_id = 0x03;
	attr_list[1].attribute_len = 0xFFFF;

	attr_list[2].attribute_id = 0x05;
	attr_list[2].attribute_len = 0xFFFF;

	ble_ancs_get_notification_attributes(uid, 3, attr_list);
}

void app_ancs_get_app_attribute(unsigned char *app_id, unsigned char attr_id)
{
	ble_ancs_get_app_attributes(app_id, 1, &attr_id);
}

void app_ancs_perform_action(unsigned char *uid, unsigned char action)
{
	ble_ancs_perform_action(uid, action);
}

profile_t g_ancs_profile = {
	app_ancs_init,
	app_ancs_done,
	NULL,
	NULL,
};

