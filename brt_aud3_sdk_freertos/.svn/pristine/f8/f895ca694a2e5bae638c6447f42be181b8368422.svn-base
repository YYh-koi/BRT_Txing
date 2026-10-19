#ifndef APP_HID_H
#define APP_HID_H

#include "global.h"
#include "bt_event.h"
#include "profile.h"

#ifdef CONFIG_HID

#define MAX_X_OF_MOUSE	0x07FF
#define MAX_Y_OF_MOUSE	0x07FF
#define MIN_X_OF_MOUSE	0xF801
#define MIN_Y_OF_MOUSE	0xF801

enum {
    EV_BTC_CONNECT_RET,
    EV_BTC_DISCONNECT_RET,
};

enum {
	HID_MEDIA_KEY_REALSE,
	HID_MEDIA_KEY_PLAY_PUASE,
	HID_MEDIA_KEY_NEXT,
	HID_MEDIA_KEY_PREV,
	HID_MEDIA_KEY_SABSUP,
	HID_MEDIA_KEY_SABSDOWN,
};

extern profile_t g_hid_profile;

void app_hid_init(module_event_cbk_t p_event_cbk);
void app_hid_server_start(void);
void app_hid_done(void);
void app_hid_server_start_by_user_param(UINT32 device_type, UINT8 *user_desc, UINT32 len);
void app_hid_connect(UINT8* bd_addr);
void app_hid_disconnect(HANDLE conn_hdl);
void app_hid_send_mouse_data(UINT8 button, INT16 x, INT16 y);
void app_hid_send_key_value(UINT8 key_value);
void app_hid_send_media_value(UINT8 comm_value);
#endif /* CONFIG_HID */
#endif /* APP_HID_H */

