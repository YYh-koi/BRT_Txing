/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/

#include "global.h"		//for NEW/FREE

#ifdef CONFIG_MAP
#include "internal_map.h"
#include "auto_map.h"
#include "internal_bmessage.h"
#define TAG "bse_map"

/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     app_map.c
 * @Description:
        This file implements the interfaces of MAP module.

 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-11
 ---------------------------------------------------------------------------*/

void app_map_init(module_event_cbk_t event_cbk)
{
	if(hlp_map_get_status() == STATE_UNINIT) {
		hlp_map_init(event_cbk);
	}
}

void app_map_done()
{
	if(hlp_map_get_status() != STATE_UNINIT) {
		hlp_map_done();
	}
}

void app_map_register_callback(app_map_event_callback callback)
{
	hlp_map_register_callback(callback);
}

void app_map_connect(UINT8 *bd)
{
	if(hlp_map_get_status() == STATE_READY) {
		hlp_map_connect(bd);
	}
}

void app_map_disconnect()
{
	if(hlp_map_get_status() >= STATE_CONNECTING) {
		hlp_map_disconnect();
	}
}

void app_map_get_sms_list_ex(UINT8 type, UINT16 offset, UINT16 max_count)
{
	bse_log_api(TAG, "%s:type is %d", __func__, type);
	hlp_map_get_message_listing(type, offset, max_count);
}
void app_map_get_sms_list(UINT16 offset, UINT16 max_count)
{
	hlp_map_get_message_listing(HLP_MAP_MESSAGE_LOCATION_INBOX, offset, max_count);
}

void app_map_send_sms(app_map_sms_item_t *sms)
{
	UINT8 *content = NULL;
	UINT16 length = 0;
	bmessage_compose(sms, &content, &length);

	hlp_map_send_message(content, length);

	if (content) {
		FREE(content);
	}
}

void app_map_get_message(UINT8 *msg_handle)
{
	hlp_map_get_message(msg_handle);
}

void app_map_delete_sms(UINT8 *msg_handle)
{
	hlp_map_delete_message(msg_handle);
}

void app_map_retrieve_sms(long handle)
{
}

profile_t g_map_profile = {
	app_map_init,
	app_map_done,
	NULL,
	NULL,
};

#endif /* CONFIG_MAP */

