/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_hid.c
*
*	Description:
*
*
*	Create Date:		2014/09/21
***********************************************************************/

#include "internal_hid.h"
#include "auto_hid.h"
#include "sdap_ui.h"

#define TAG "bse_hid"
#ifdef CONFIG_HID

static struct AppHidUserDataStru *s_hid_user_data = NULL;

void event_hid_indication(UINT32 event, void *param)
{
	if (s_hid_user_data->event_cbk != NULL) {
		s_hid_user_data->event_cbk((PROFILE_ID_HID << 8) | event, param);
	}
}

void hlp_hid_init(module_event_cbk_t event_cbk)
{
	if (s_hid_user_data == NULL) {
		s_hid_user_data = (struct AppHidUserDataStru *)NEW(sizeof(struct AppHidUserDataStru));
	}
	memset(s_hid_user_data, 0, sizeof(struct AppHidUserDataStru));

	s_hid_user_data->event_cbk = event_cbk;
	hlp_hid_set_status(ST_APP_HID_INIT);
}

UINT8 hlp_hid_server_start(struct AppHidInfo *hid_info)
{
	UINT8 res = BT_SUCCESS;
#ifndef CONFIG_SDP_FIXED_DATABASE
	UINT32 size = 0;
	struct SDAP_HIDInfoStru *hid_attr = NULL;
	UINT32 temp = sizeof(struct SDAP_HIDInfoStru);

	size = temp + hid_info->desc_len + 2;
	hid_attr = NEW(size);
	memset(hid_attr, 0, size);
	hid_attr->sub_cls = hid_info->sub_cls;
	//hid_attr->profile_ver = 0x0100;
	hid_attr->release_num = 0x0100;
	hid_attr->parser_ver = 0x0111;
	hid_attr->country_code = 0xFE;
	hid_attr->mask = HID_MASK_VCABLE_TRUE;
	hid_attr->mask |= HID_MASK_RECONNINIT_TRUE | OPTATTR_MASK_NORMCONN;
	//hid_attr->mask |= HID_MASK_BOOTDEVICE_TRUE | OPTATTR_MASK_BATTERYPWR;
	hid_attr->super_to = 5000;
	hid_attr->desc_list_size = hid_info->desc_len + 3;
	hid_attr->list[0] = 0x22;	// Type of descriptor
	hid_attr->list[1] = (UINT8)(hid_info->desc_len & 0xFF); // Lower byte of the descriptor size
	hid_attr->list[2] = (UINT8)(hid_info->desc_len >> 8); // higher byte of the descriptor size
	memcpy(&hid_attr->list[3], hid_info->desc_list, hid_info->desc_len);

	res = SDAP_RegisterHIDService("HID Device Service", hid_attr);
	if (res == BT_SUCCESS) {
		s_hid_user_data->attr_svc_hdl = hid_attr->svc_hdl;
		if (!s_hid_user_data->pnp_svc_hdl) {
			struct SDAP_DIInfoStru *pnp_attr = (struct SDAP_DIInfoStru *)NEW((sizeof(struct SDAP_DIInfoStru) - 1));

			memset(pnp_attr, 0, (sizeof(struct SDAP_DIInfoStru) - 1));
			pnp_attr->size = sizeof(struct SDAP_DIInfoStru) - 1;
			pnp_attr->spec_id = 0x0100;
			pnp_attr->vendor_id = 0x23A1;
			pnp_attr->product_id = 0x0111;
			pnp_attr->version = 0x0100;
			pnp_attr->vendor_id_src = 0x0001;
			pnp_attr->list_size = 0;
			if (SDAP_RegisterDIService((UCHAR*)"Device ID", pnp_attr) == BT_SUCCESS) {
				s_hid_user_data->pnp_svc_hdl = pnp_attr->svc_hdl;
			}
			FREE(pnp_attr);
		}
	}
	FREE(hid_attr);

#endif /* CONFIG_SDP_FIXED_DATABASE */
	if (res == BT_SUCCESS) {
		HID_Device_Start(hlp_hid_server_cbk);
		hlp_hid_set_status(ST_APP_HID_READY);
	}

	return res;
}

void hlp_hid_server_stop(void)
{
	HID_Device_Stop();
	if (s_hid_user_data->attr_svc_hdl) {
		SDAP_ServiceUnregister(s_hid_user_data->attr_svc_hdl);
		s_hid_user_data->attr_svc_hdl = NULL;
	}
	hlp_hid_set_status(ST_APP_HID_INIT);
}

void hlp_hid_done(void)
{
	if (s_hid_user_data->conn_info != NULL) {
		FREE(s_hid_user_data->conn_info);
	}
	FREE(s_hid_user_data);
	s_hid_user_data = NULL;
}

void hlp_hid_connect(struct HCI_AddressStru *addr_stru)
{
	if (addr_stru != NULL) {
		HID_Device_ConnectReq(addr_stru);
	}
}

void hlp_hid_disconnect(HANDLE conn_hdl)
{
	if (conn_hdl != NULL) {
		HID_Device_DisconnectReq(conn_hdl);
	} else if (s_hid_user_data->conn_info && s_hid_user_data->conn_info->conn_hdl) {
		HID_Device_DisconnectReq(s_hid_user_data->conn_info->conn_hdl);
	}
}

void hlp_hid_data_req_t(struct HID_DataStru *in)
{
	HID_Device_DataReq(s_hid_user_data->conn_info->conn_hdl, in);
}

void hlp_hid_data_req(UINT8 *p_data, UINT16 data_len)
{
	if (s_hid_user_data->conn_info != NULL) {
		struct HID_DataStru *in = (struct HID_DataStru *)NEW(sizeof(struct HID_DataStru)+data_len);

		in->report_type = HID_INPUT_REPORT;
		in->data_len = data_len;
		memcpy(in->data, p_data, data_len);
		HID_Device_DataReq(s_hid_user_data->conn_info->conn_hdl, in);
	}
}

void hlp_hid_cable_plug(struct AppHidConnInfoStru *in)
{
	struct HID_ConnectStru req;

	if (s_hid_user_data->conn_info) {
		if (memcmp(&s_hid_user_data->conn_info->addr, &in->addr, sizeof(struct HCI_AddressStru))) {
			s_hid_user_data->conn_info->conn_hdl = in->conn_hdl;
			FREE(in);
		} else {
			FREE(s_hid_user_data->conn_info);
		}
	}
	s_hid_user_data->conn_info = in;
	in = NULL;
	memset(&req, 0, sizeof(struct HID_ConnectStru));
	memcpy(&req.addr, &s_hid_user_data->conn_info->addr, sizeof(struct HCI_AddressStru));
	HID_Device_CablePlug(&req);
}

void hlp_hid_cable_unplug(struct HID_ConnectStru *in)
{
	if (s_hid_user_data->conn_info) {
		FREE(s_hid_user_data->conn_info);
		s_hid_user_data->conn_info = NULL;
	}

	hlp_hid_set_status(ST_APP_HID_READY);
}

/* Inernal Functions */
void *hlp_hid_new_conn_info(struct HCI_AddressStru *addr)
{
	struct AppHidConnInfoStru *conn_info = NULL;

	conn_info = NEW(sizeof(struct AppHidConnInfoStru));
	memset(conn_info, 0, sizeof(struct AppHidConnInfoStru));
	memcpy(&conn_info->addr, addr, sizeof(struct HCI_AddressStru));

	return conn_info;
}

void hlp_hid_cbk_connect_complete(HANDLE conn_hdl, struct HID_ConnectStru *arg)
{
	bse_log_api(TAG, "hlp_hid_cbk_connect_complete, ret=%d\r\n", arg->result);

	if (arg->result == HID_SUCCESSFUL) {
		struct AppHidConnInfoStru *conn_info = hlp_hid_new_conn_info(&arg->addr);

		conn_info->conn_hdl = conn_hdl;
		if (s_hid_user_data->conn_info != NULL
		    && (memcmp(&conn_info->addr, &s_hid_user_data->conn_info->addr, sizeof(struct HCI_AddressStru)))) {

			HID_Device_CableUnplugReq(&arg->addr);
			FREE(conn_info);

			hlp_hid_set_status(ST_APP_HID_READY);
			event_hid_indication(EV_BTC_CONNECT_RET, NULL);
		} else {
			hlp_hid_cable_plug(conn_info);
			hlp_hid_set_status(ST_APP_HID_CONNECTED);
			//EventInd("HA");
			event_hid_indication(EV_BTC_CONNECT_RET, arg->addr.bd);
		}
	} else {
		if (s_hid_user_data->conn_info != NULL) {
			s_hid_user_data->conn_info->conn_hdl = NULL;
		}

		HID_Device_CableUnplugReq(&arg->addr);
		hlp_hid_set_status(ST_APP_HID_READY);

		event_hid_indication(EV_BTC_CONNECT_RET, NULL);
	}
}

void hlp_hid_cbk_disconnect_complete(HANDLE conn_hdl, struct HID_ConnectStru *arg)
{
	bse_log_api(TAG, "hlp_hid_cbk_disconnect_complete, ret=%d\r\n", arg->result);

	if (s_hid_user_data->conn_info != NULL) {
		FREE(s_hid_user_data->conn_info);
		s_hid_user_data->conn_info = NULL;
	}

	HID_Device_CableUnplugReq(&(arg->addr));
	hlp_hid_set_status(ST_APP_HID_READY);

	event_hid_indication(EV_BTC_DISCONNECT_RET, arg->addr.bd);
}

void hlp_hid_server_cbk(UINT8 event, HANDLE conn_hdl, void *in)
{
	bse_log_api(TAG, "hlp_hid_server_cbk event=%d\r\n", event);

	switch (event) {
	case HID_EV_CONNECT:
		hlp_hid_cbk_connect_complete(conn_hdl, in);
		break;
	case HID_EV_DISCONNECT:
		hlp_hid_cbk_disconnect_complete(conn_hdl, in);
		break;
	case HID_EV_CABLE_UNPLUG:
		hlp_hid_cable_unplug(in);
		break;
	default:
		break;
	}
}


void hlp_hid_set_status(UINT32 status)
{
	if ((s_hid_user_data != NULL) &&(status >= ST_APP_HID_INIT) && (status < ST_APP_END)) {
	    bse_log_info(TAG, "hlp_hid_set_status = %d\r\n", status);
		s_hid_user_data->state = status;
	}
}

UINT32 hlp_hid_get_status(void)
{
	if (s_hid_user_data != NULL) {
	    bse_log_info(TAG, "hlp_hid_get_status = %d\r\n", s_hid_user_data->state);
		return s_hid_user_data->state;
	} else {
	    bse_log_info(TAG, "hlp_hid_get_status ST_APP_HID_UNINIT\r\n");
		return ST_APP_HID_UNINIT;
	}
}

#endif /* CONFIG_HID */

