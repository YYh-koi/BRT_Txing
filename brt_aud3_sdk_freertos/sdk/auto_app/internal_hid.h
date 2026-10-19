/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_hid.h
*
*	Description:
*
*
*	Create Date:		2014/09/21
***********************************************************************/

#ifndef _HLP_HID_H_
#define _HLP_HID_H_

#include "global.h"
#include "hci_ui.h"
#include "hid_ui.h"
#include "bt_event.h"
#include "brt_trace.h"

#ifdef CONFIG_HID

enum {
    ST_APP_HID_UNINIT,
    ST_APP_HID_INIT,
    ST_APP_HID_READY,
    ST_APP_HID_CONNECTING,
    ST_APP_HID_CONNECTED,
    ST_APP_END,
};

struct AppHidConnInfoStru {
	HANDLE conn_hdl;
	struct HCI_AddressStru addr;
};

struct AppHidUserDataStru {
	module_event_cbk_t event_cbk;
	UINT32 state;

	struct AppHidConnInfoStru *conn_info;
	DWORD attr_svc_hdl;
	DWORD pnp_svc_hdl;
	UINT8 hid_block_status;
};

struct AppHidInfo {
	UINT32 desc_len;
	UINT8 *desc_list;
	UINT8 sub_cls;
};

struct AppHidMouseDataStru {
	UINT8 report_id;

	UINT8 button;
	INT16 x;
	INT16 y;
};

struct AppHidKeyboardDataStru {
	UINT8 report_id;

	UINT8 key_value;
};

struct AppHidMediaDataStru {
	UINT8 report_id;

	UINT8 command_value;
};

/* Interface */
void hlp_hid_init(module_event_cbk_t event_cbk);
UINT8 hlp_hid_server_start(struct AppHidInfo *hid_info);
void hlp_hid_server_stop(void);
void hlp_hid_done(void);
void hlp_hid_connect(struct HCI_AddressStru *bd_addr);
void hlp_hid_disconnect(HANDLE conn_hdl);
void hlp_hid_data_req(UINT8 *p_data, UINT16 data_len);
void hlp_hid_cable_plug(struct AppHidConnInfoStru *in);
void hlp_hid_cable_unplug(struct HID_ConnectStru *in);
void *hlp_hid_new_conn_info(struct HCI_AddressStru *addr);
void hlp_hid_cbk_connect_complete(HANDLE conn_hdl, struct HID_ConnectStru *arg);
void hlp_hid_cbk_disconnect_complete(HANDLE conn_hdl, struct HID_ConnectStru *arg);
void hlp_hid_server_cbk(UINT8 event, HANDLE conn_hdl, void *in);
void hlp_hid_set_status(UINT32 status);
UINT32 hlp_hid_get_status(void);

#endif /* CONFIG_HID */
#endif	/* _HLP_HID_H_ */

