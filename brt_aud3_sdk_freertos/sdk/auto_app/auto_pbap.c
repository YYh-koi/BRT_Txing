/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_pbap.c
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "internal_pbap.h"
#include "auto_pbap.h"
#include "pbap_ui.h"
#include "brt_trace.h"
#include "bt_event.h"
#define TAG "bse_pbap"

void app_pbap_init(module_event_cbk_t event_cbk)
{
	if (hlp_pbap_get_status() != ST_PBAP_UNINIT) {
		return;
	}

	hlp_pbap_init(event_cbk);
}

UINT8 app_pbap_get_status(void)
{
	return hlp_pbap_get_status();
}

void app_pbap_server_start(void)
{
	if (hlp_pbap_get_status() != ST_PBAP_INIT) {
		return;
	}

	hlp_pbap_server_start();
}

void app_pbap_server_stop(void)
{
	if (hlp_pbap_get_status() < ST_PBAP_READY) {
		return;
	}

	hlp_pbap_server_stop();
}

void app_pbap_done(void)
{
	if (hlp_pbap_get_status() == ST_PBAP_UNINIT) {
		return;
	} else {
		app_pbap_server_stop();
	}

	hlp_pbap_done();
}

void app_pbap_get_phonebook(UINT8 *bd, UINT8 store_type, UINT16 offset, UINT16 count)
{
	if (hlp_pbap_get_status() == ST_PBAP_UNINIT) {
		return ;
	}
	UINT8 zero_bd[BD_ADDR_LEN];
	memset(zero_bd, 0, sizeof(zero_bd));

	hlp_pbap_set_phonebook_param(store_type, offset, count);

	if (hlp_pbap_get_status() == ST_PBAP_READY) {
		if(memcmp(zero_bd, bd, BD_ADDR_LEN) == 0) {
			bse_log_warning(TAG, "zero bd  , will return fail");
			event_pbap_indication(EV_PBAP_CONNECT_FAIL, bd);
			return ;
		}
		hlp_pbap_set_flag_auto_connect(TRUE);
		hlp_pbap_connect(bd, NULL, NULL);
	} else if (hlp_pbap_get_status() == ST_PBAP_CONNECTED) {
		hlp_pbap_get_phonebook();
	}
}

void app_pbap_get_vcardListing(UINT8 *bd, UCHAR *call_number)
{
	if (hlp_pbap_get_status() == ST_PBAP_UNINIT) {
		return ;
	}
	UINT8 zero_bd[BD_ADDR_LEN];
	memset(zero_bd, 0, sizeof(zero_bd));

	if(strlen(call_number) <= 0) {
	    bse_log_warning(TAG, "call number  , will return fail");
	    return;
	}

	if ((hlp_pbap_get_status() == ST_PBAP_READY) && (memcmp(call_number, "123456789", strlen("123456789")) != 0)) {
		if(memcmp(zero_bd, bd, BD_ADDR_LEN) == 0) {
			bse_log_warning(TAG, "zero bd  , will return fail");
			event_pbap_indication(EV_PBAP_CONNECT_FAIL, bd);
			return ;
		}
		hlp_pbap_set_flag_auto_connect(TRUE);
		hlp_pbap_set_download_mode(0, call_number);
		hlp_pbap_connect(bd, NULL, NULL);
	} else if (hlp_pbap_get_status() == ST_PBAP_CONNECTED) {
		hlp_pbap_get_vcardListing(call_number);
	} else {
		bt_event_indicate(BT_EV_EVENT_ERROR, NULL);
	}
}
void app_pbap_abort_phonebook(void)
{
	/*if((hlp_pbap_get_status() >= ST_PBAP_READY) && (hlp_pbap_get_handle() != NULL)) {
		PBAP_AbortPullObjectReq(hlp_pbap_get_handle());
	}*/
	hlp_pbap_abort_phonebook();
}

void app_pbap_connect(UINT8 *bd)
{
	if (hlp_pbap_get_status() == ST_PBAP_READY) {
		hlp_pbap_set_flag_auto_connect(FALSE);
		hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
		hlp_pbap_connect(bd, NULL, NULL);
	} else {
		event_pbap_indication(EV_PBAP_CONNECT_FAIL, bd);
	}
}

void app_pbap_disconnect()
{
	hlp_pbap_disconnect();
}

void app_pbap_stop(void)
{
	if (hlp_pbap_get_status() >= ST_PBAP_CONNECTED && hlp_pbap_get_handle() != NULL) {
		PBAP_ClientStopReq(hlp_pbap_get_handle());
	} else {
		event_pbap_indication(EV_PBAP_DISCONNECTED, NULL);
	}
}

BOOL app_pbap_get_flag_auto_connect(void)
{
	if (hlp_pbap_get_status() >= ST_PBAP_READY) {
		return hlp_pbap_get_flag_auto_connect();
	} else {
		return FALSE;
	}
}
void app_pbap_set_req_contact_info(INT32 mask)
{
	hlp_pbap_set_req_contact_info(mask);
}

profile_t g_pbap_profile = {
	app_pbap_init,
	app_pbap_done,
	app_pbap_server_start,
	app_pbap_server_stop,
};

