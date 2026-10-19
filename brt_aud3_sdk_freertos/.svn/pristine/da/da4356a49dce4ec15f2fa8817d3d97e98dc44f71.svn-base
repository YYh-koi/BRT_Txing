/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2015 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_iap.c
*
*	Description:		Internal Functions For IAP
*
*
*	Create Date:		2015/07/28
***********************************************************************/

#include "global.h"
#include "rfcomm_ui.h"
#include "auto_rfcomm.h"
#include "auto_gap.h"
#include "brt_trace.h"
#include "iap2_ui.h"
#include "internal_iap.h"
#include "auto_iap.h"
#include "internal_manager.h"

#ifdef CONFIG_IAP2
#define TAG "bse_iap2"
static iap_data_t *s_iap_data = NULL;
static struct FsmTimer *s_timer_iap = NULL;

static void Porting_IAP2_EventInd(void *param, UINT8 event);
static void Porting_IAP2_GetLocInfo(UINT8 loc_bd[BD_ADDR_LEN], UINT8 loc_name[64]);
static UINT32 Porting_Timer_Add(UINT32 timeout, CBK_Timer_Expire *cbk, void *param, UINT8 event);
static void Porting_Timer_Kill(UINT32 timer_hdl);
static void Porting_TL_Start();
static void Porting_TL_Stop();
static void Porting_TL_Connect(UINT8 *bd);
static void Porting_TL_Disconnect();
static struct BuffStru *Porting_TL_DataNew(UINT16 size);
static void Porting_TL_DataReq(struct BuffStru *data, UINT8 out_index);
static void Porting_TL_DataConsumed();
static void Porting_Auth_GetCertDataLen();
static void Porting_Auth_GetCertData(UINT8 *buff, UINT16 buff_len);
static void Porting_Auth_ChData(UINT8 *data, UINT16 data_len);
static void Porting_Auth_GetChRspData(UINT8 *buff, UINT16 buff_len);

static IAP2_PortingFuncStru s_iap2_func = {
	Porting_IAP2_EventInd,
	Porting_IAP2_GetLocInfo,
	Porting_Timer_Add,
	Porting_Timer_Kill,
	Porting_TL_Start,
	Porting_TL_Stop,
	Porting_TL_Connect,
	Porting_TL_Disconnect,
	Porting_TL_DataNew,
	Porting_TL_DataReq,
	Porting_TL_DataConsumed,
	Porting_Auth_GetCertDataLen,
	Porting_Auth_GetCertData,
	Porting_Auth_ChData,
	Porting_Auth_GetChRspData
};

void hlp_iap_timer_start_server(void *arg);

void hlp_iap_init(module_event_cbk_t event_cbk)
{
	if (s_iap_data == NULL) {
		s_iap_data = (iap_data_t *)NEW(sizeof(iap_data_t));
	}

	memset(s_iap_data, 0, sizeof(iap_data_t));
	s_iap_data->event_cbk = event_cbk;
	hlp_iap_set_status(ST_IAP_INIT);
}

void hlp_iap_done()
{
	FREE(s_iap_data);
	s_iap_data = NULL;
}

void hlp_iap_start(void)
{
	FsmAddTimer(NULL, 500, 0, (void *)EV_SPP_CONNECT_RET, NULL, (UINT32)hlp_iap_timer_start_server);
}

void hlp_iap_connect(UINT8 *bd)
{
	if (s_iap_data->state == ST_IAP_READY) {
		hlp_iap_set_status(ST_IAP_CONNECTING);
		memcpy(s_iap_data->conn_bd, bd, BD_ADDR_LEN);
		IAP2_ConnectReq(bd);
	}
}

void hlp_iap_connection_cfm()
{
	IAP2_TL_ConnComplete(s_iap_data->conn_bd, IAP2_SUCCESSFUL);
}

void hlp_iap_disconnect()
{
	if (s_iap_data->state == ST_IAP_CONNECTED || s_iap_data->state == ST_IAP_STARTED) {
		IAP2_DisconnectReq();
	}
}

void hlp_iap_hid_start(void)
{
	IAP2_HID_Start();
}

void hlp_iap_hid_stop(void)
{
	IAP2_HID_Stop();
}

void hlp_iap_send_hid_mouse(UINT8 button, CHAR x, CHAR y)
{
	if (hlp_iap_get_status() == ST_IAP_STARTED) {
		iap_hid_report_t report;

		memset(&report, 0, sizeof(iap_hid_report_t));
		report.ctrl = button;
		report.x = x;
		report.y = y;
		IAP2_HidReport(&report, sizeof(iap_hid_report_t));
	}
}

void hlp_iap_mouse_coord(UINT8 ctrl, INT16 x, INT16 y)
{
	char step_num_x = 0;
	char step_num_y = 0;
	char step_length_x = 0;
	char step_length_y = 0;
	iap_hid_report_t rep;

	if ((s_iap_data->state != ST_IAP_STARTED) || (s_iap_data->flag_fc == 0)) {
		return;
	}

	bse_log_api(TAG, "hlp_iap_mouse_data, ctrl=%d, x=%d, y=%d\r\n", ctrl, x, y);
	memset(&rep, 0, sizeof(iap_hid_report_t));

	step_num_x = abs(x)>>7;
	step_num_y = abs(y)>>7;
	bse_log_info(TAG, "hlp_iap_mouse_data, step number, x=%d, y=%d\r\n", step_num_x, step_num_y);

	if (step_num_x > step_num_y) {
		step_length_x = IAP_MOUSE_STEP_LENGTH;
		step_length_y = abs(y)/step_num_x;
	} else if (step_num_x < step_num_y) {
		step_length_x = abs(x)/step_num_y;
		step_length_y = IAP_MOUSE_STEP_LENGTH;
	} else {
		step_length_x = IAP_MOUSE_STEP_LENGTH;
		step_length_y = IAP_MOUSE_STEP_LENGTH;
	}
	bse_log_info(TAG, "hlp_iap_mouse_data, step length, x=%d, y=%d\r\n", step_length_x, step_length_y);

	while (1) {
		if (x >= step_length_x) {
			rep.x = step_length_x;
			x -= step_length_x;
		} else if (x <= -step_length_x) {
			rep.x = -step_length_x;
			x -= -step_length_x;
		} else {
			rep.x = x;
			x = 0;
		}

		if (y >= step_length_y) {
			rep.y = step_length_y;
			y -= step_length_y;
		} else if (y <= -step_length_y) {
			rep.y = -step_length_y;
			y -= -step_length_y;
		} else {
			rep.y = y;
			y = 0;
		}

		if (x == 0 && y == 0) {
			rep.ctrl = ctrl;
		}

		IAP2_HidReport(&rep, sizeof(iap_hid_report_t));

		if (x == 0 && y == 0) {
			break;
		}
	}

}

void hlp_iap_send_key_value(UINT8 key_value_1, UINT8 key_value_2)
{
	if (hlp_iap_get_status() == ST_IAP_STARTED) {
		iap_hid_report_t report;

		memset(&report, 0, sizeof(iap_hid_report_t));
		report.u1.control_uint8_1 = key_value_1;
		report.u2.control_uint8_2 = key_value_2;

		IAP2_HidReport(&report, sizeof(iap_hid_report_t));
	}
}

void hlp_iap_set_status(UINT8 status)
{
	bse_log_api(TAG, "hlp_iap_set_status, now[%d] %d\r\n", s_iap_data->state, status);
	s_iap_data->state = status;
}

UINT8 hlp_iap_get_status(void)
{
	return s_iap_data->state;
}

void hlp_iap_timer_start_server(void *arg)
{
	UINT8 iap_eir_uuid[16] = {0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF};

	app_gap_add_eir(iap_eir_uuid);

	s_iap_data->flag_fc = 1;
	IAP2_Start(&s_iap2_func);
}

void hlp_iap_timer_report_tl_event(void *arg)
{
	if ((UINT32)arg == EV_SPP_CONNECT_RET) {
		IAP2_TL_ConnComplete(s_iap_data->conn_bd, IAP2_SUCCESSFUL);
	} else if ((UINT32)arg == EV_SPP_DISCONNECT_RET) {
		IAP2_TL_DisconnComplete(s_iap_data->conn_bd);
	}
	s_timer_iap = NULL;
}

void hlp_iap_rfcomm_event_cbk(UINT8 event, void *param)
{
	bse_log_api(TAG, "hlp_iap_rfcomm_event_cbk, event=%d, param=%08x\r\n", event, param);

	switch (event) {
	case EV_SPP_DATA_IND: {
		struct app_spp_data_ind_stru *data_ind = (struct app_spp_data_ind_stru *)param;

		if ((s_iap_data->state == ST_IAP_CONNECTED || s_iap_data->state == ST_IAP_STARTED) && (s_iap_data->conn_hdl == data_ind->handle)) {
			IAP2_TL_DataReceive(data_ind->data, data_ind->data_len);
		}
	}
	break;
	case EV_SPP_SERVER_STARTED: {
		rfc_srv_info_t *p_srv_info = (rfc_srv_info_t *)param;

		if (s_iap_data->local_srv_chl == p_srv_info->local_server_channel) {
			s_iap_data->svr_hdl = p_srv_info->sdp_handle;
			hlp_iap_set_status(ST_IAP_READY);
		}
	}
	break;
	case EV_SPP_SERVER_STOPPED:
		break;
	case EV_SPP_CONNECT_RET: {
		rfc_con_info_t *p_conn_info = (rfc_con_info_t *)param;

		bse_log_info(TAG, "EV_SPP_CONNECT_RET, %d\r\n", p_conn_info->conn_state);
		if ((p_conn_info->conn_state == rfc_status_success) || (p_conn_info->conn_state == rfc_conn_state_connected)) {
			if (((s_iap_data->state == ST_IAP_READY) && (s_iap_data->local_srv_chl == p_conn_info->server_channel))
			    || ((s_iap_data->state == ST_IAP_CONNECTING) && (memcmp(s_iap_data->conn_bd, p_conn_info->bd, BD_ADDR_LEN) == 0))) {

				s_iap_data->conn_hdl = p_conn_info->handle;
				memcpy(s_iap_data->conn_bd, p_conn_info->bd, BD_ADDR_LEN);
				hlp_iap_set_status(ST_IAP_CONNECTED);
				s_timer_iap = FsmAddTimer(NULL, 100, 0, (void *)EV_SPP_CONNECT_RET, NULL, (UINT32)hlp_iap_timer_report_tl_event);
			}
		} else {
			s_timer_iap = FsmAddTimer(NULL, 10, 0, (void *)EV_SPP_DISCONNECT_RET, NULL, (UINT32)hlp_iap_timer_report_tl_event);
			hlp_iap_set_status(ST_IAP_READY);
		}
	}
	break;
	case EV_SPP_DISCONNECT_RET: {
		UINT32 handle = (UINT32)param;

		if (s_iap_data->conn_hdl == handle) {
			hlp_iap_set_status(ST_IAP_READY);
			s_timer_iap = FsmAddTimer(NULL, 10, 0, (void *)event, NULL, (UINT32)hlp_iap_timer_report_tl_event);
		}
	}
	break;
	case EV_SPP_SEND_DATA_RET: {
		if (s_iap_data->conn_hdl == (UINT32)param) {
			UINT8 out_credit = app_spp_get_out_credit(s_iap_data->conn_hdl);

			if (out_credit > 0 && s_iap_data->flag_out_index == 1) {
				s_iap_data->flag_out_index = 0;
				IAP2_TL_DataCfm(s_iap_data->out_size, s_iap_data->out_index);
			}
		}
	}
	break;
	default:
		break;
	}
}

static void Porting_IAP2_EventInd(void *param, UINT8 event)
{
	bse_log_api(TAG, "EXT_IAP2_EventInd, event=%d, param=%08x\r\n", event, param);

	switch(event) {
	case IAP2_EV_CONNECTED:
		hlp_iap_set_status(ST_IAP_STARTED);
		break;
	case IAP2_EV_DISCONNECTED:
		if (s_iap_data->state == ST_IAP_READY) {
			s_iap_data->event_cbk(EV_IAP_STOPPED, NULL);
		}
		break;
	case IAP2_EV_HIDSTARTED:
		s_iap_data->event_cbk(EV_IAP_STARTED, NULL);
		break;
	case IAP2_EV_HIDSTOPED:
		s_iap_data->event_cbk(EV_IAP_STOPPED, NULL);
		break;
	case IAP2_EV_EASTARTED:
		break;
	case IAP2_EV_EASTOPPED:
		break;
	case IAP2_EV_FCON:
		s_iap_data->flag_fc = 1;
		break;
	case IAP2_EV_FCOFF:
		s_iap_data->flag_fc = 0;
		break;
	default:
		break;
	}
}

static void Porting_IAP2_GetLocInfo(UINT8 loc_bd[BD_ADDR_LEN], UINT8 loc_name[64])
{
	UINT8 *p_bd = hlp_manager_get_local_bd_addr();
	UINT8 *p_name = hlp_manager_get_local_name();
	UINT16 name_len = strlen(p_name);

	bse_log_api(TAG, "EXT_IAP2_GetLocInfo------>\r\n");
	loc_bd[0] = p_bd[5];
	loc_bd[1] = p_bd[4];
	loc_bd[2] = p_bd[3];
	loc_bd[3] = p_bd[2];
	loc_bd[4] = p_bd[1];
	loc_bd[5] = p_bd[0];
	memset(loc_name, 0, sizeof(loc_name));
	memcpy(loc_name, p_name, name_len > sizeof(loc_name)-1 ? sizeof(loc_name)-1 : name_len);
}


/*-----------------------------------------------------------------------------
Description:
	Add a timer.
Arguments:
	timeout:	[in] Timeout value in milliseconds;
	cbk:		[in] Callback function for timer expires;
	param:		[in] Parameter;
	event:		[in] Timer event.
Return:
	Timer Handle.
-----------------------------------------------------------------------------*/
static UINT32 Porting_Timer_Add(UINT32 timeout, CBK_Timer_Expire *cbk, void *param, UINT8 event)
{
	bse_log_api(TAG, "EXT_Timer_Add------>\r\n");
	return FsmAddTimer(NULL, timeout, event, param, NULL, (UINT32)cbk);
}


/*-----------------------------------------------------------------------------
Description:
	Kill a timer.
Arguments:
	timer_hdl:	[in] Timer Handle.
-----------------------------------------------------------------------------*/
static void Porting_Timer_Kill(UINT32 timer_hdl)
{
	bse_log_api(TAG, "EXT_Timer_Kill, timer_hdl=%08x------>\r\n", timer_hdl);
	FSM_DelTimer(&timer_hdl);
}

/*-----------------------------------------------------------------------------
Description:
	Start Tranport Layer, register local Device Name.
Remarks:
	If the Transport Layer is Bluttooth SPP, register SPP Serivce with the MFI
	specified Accessory UUID.
-----------------------------------------------------------------------------*/
static void Porting_TL_Start()
{
	UINT8 iap_uuid[16] = {0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF};

	s_iap_data->local_srv_chl= app_spp_get_server_channel();
	bse_log_api(TAG, "EXT_TL_Start------>%d\r\n", s_iap_data->local_srv_chl);
	app_spp_server_start_ex(s_iap_data->local_srv_chl, "IAP", iap_uuid);
}

/*-----------------------------------------------------------------------------
Description:
	Stop Tranport Layer.
-----------------------------------------------------------------------------*/
static void Porting_TL_Stop()
{
	bse_log_api(TAG, "EXT_TL_Stop------>\r\n");
	app_spp_server_stop_ex(s_iap_data->svr_hdl);
}

/*-----------------------------------------------------------------------------
Description:
	Establish a TL connection.
Arguments:
	bd:		Bluetooth address.
Remarks:
	If the Transport Layer is Bluetooth SPP, connect SPP with the MFI specified
	Device UUID.
-----------------------------------------------------------------------------*/
static void Porting_TL_Connect(UINT8 *bd)
{
	UINT8 iap_uuid[16] = {0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFE};

	bse_log_api(TAG, "EXT_TL_Connect------>\r\n");
	app_spp_connect_ex(0, bd, iap_uuid);
}

/*-----------------------------------------------------------------------------
Description:
	Terminate a TL connection.
Arguments:
	conn_hdl:	[in] connection handle.
-----------------------------------------------------------------------------*/
static void Porting_TL_Disconnect()
{
	bse_log_api(TAG, "EXT_TL_Disconnect------>\r\n");
	app_spp_disconnect(s_iap_data->conn_hdl);
}

static struct BuffStru *Porting_TL_DataNew(UINT16 size)
{
	UINT16 mfs = app_spp_get_mfs(s_iap_data->conn_hdl);
	UINT16 remain_len = mfs < size ? mfs : size;

	return BuffNew(remain_len, 0);
}

/*-----------------------------------------------------------------------------
Description:
	Send data on transport layer.
Arguments:
	conn_hdl:	[in] connection handle;
	data:		[in] the Packet will be sent.
	data_len:	[in] Length of the packet will be sent;
-----------------------------------------------------------------------------*/
static void Porting_TL_DataReq(struct BuffStru *data, UINT8 out_index)
{
	UINT8 out_credit = 0;

	bse_log_api(TAG, "EXT_TL_DataReq---->%d\r\n", out_index);
	s_iap_data->out_index = out_index;
	app_spp_send_data(s_iap_data->conn_hdl, data);
	out_credit = app_spp_get_out_credit(s_iap_data->conn_hdl);
	if (out_credit > 0) {
		s_iap_data->flag_out_index = 0;
		IAP2_TL_DataCfm(DATASIZE(data), out_index);
	} else {
		s_iap_data->flag_out_index = 1;
		s_iap_data->out_size = DATASIZE(data);
	}
	FREE(data);
}

/*--------------------------------------------------------------------------------
Description:
    Data is consumed, and notify remote device to send more data if necessary.
---------------------------------------------------------------------------------*/
static void Porting_TL_DataConsumed()
{

}

static UINT8 hlp_iap_get_cert_data(iap_cp_data_t *p_auth)
{
	char ret = -1;
	UINT8 i = 0;
	UINT8 cert_data_length_byte[2];
	UINT16 read_length = 0;
	UINT32 cert_date_current_page_reg = CP_CERTIFICATE_DATA_PAGE_START_REG;

	bse_log_api(TAG, "hlp_iap_get_cert_data\r\n");
	if (p_auth != NULL) {
		dep_read_data_from_cp(CP_CERTIFICATE_DATA_LENGTH_REG, cert_data_length_byte, sizeof(cert_data_length_byte));
		p_auth->data_len = (cert_data_length_byte[0]<<8)|(cert_data_length_byte[1]);

		bse_log_info(TAG, "[hlp_iap_get_cert_data] %d\r\n", p_auth->data_len);
		p_auth->p_buffer = (UINT8 *)NEW(p_auth->data_len);
		p_auth->data_cur = 0;
		for (read_length; read_length < p_auth->data_len; read_length+=CP_CERTIFICATE_DATA_PAGE_SIZE) {
			//bse_log_verbose(TAG, "read_length=%d\r\n", read_length);
			if (p_auth->data_len - read_length > CP_CERTIFICATE_DATA_PAGE_SIZE) {
				ret = dep_read_data_from_cp(cert_date_current_page_reg
				                            , p_auth->p_buffer+read_length
				                            , CP_CERTIFICATE_DATA_PAGE_SIZE);
			} else {
				ret = dep_read_data_from_cp(cert_date_current_page_reg
				                            , p_auth->p_buffer+read_length
				                            , p_auth->data_len-read_length);
			}

			cert_date_current_page_reg++;
			if (ret == 0) {
				bse_log_verbose(TAG, "read fail num=%d\r\n", read_length/CP_CERTIFICATE_DATA_PAGE_SIZE);
				FREE(p_auth->p_buffer);
				memset(p_auth, 0, sizeof(iap_cp_data_t));
				break;
			}
		}
	}
	return (read_length >= p_auth->data_len);
}

static UINT8 hlp_iap_get_ch_data(iap_cp_data_t *p_ch, UINT8 *challenge, UINT8 ch_len)
{
	char ret = -1;
	UINT8 wait_status_counter = 20;
	UINT8 auth_ctl_status = 0x00;
	UINT8 signature_data_length_byte[2] = {0};

	bse_log_api(TAG, "hlp_iap_get_ch_data, ch_len=%d\r\n", ch_len);

	if (challenge != NULL) {
		//write challenge data length
		dep_write_data_to_cp(CP_CHALLENGR_DATA_LENGTH_REG, &ch_len, CP_CHALLENGR_DATA_LENGTH_REG_SIZE);
		//write challenge data
		dep_write_data_to_cp(CP_CHALLENGR_DATA_REG, challenge, ch_len);

		auth_ctl_status = 0x01;
		dep_write_data_to_cp(CP_AUTH_CTL_AND_STATUS_REG, &auth_ctl_status, sizeof(auth_ctl_status));

		auth_ctl_status = 0x00;
		while ((wait_status_counter-- > 0) && (CP_AUTH_CTL_AND_STATUS_GET_STATUS_BIT(auth_ctl_status) != 0x01)) {
			SLEEP(5);
			dep_read_data_from_cp(CP_AUTH_CTL_AND_STATUS_REG, &auth_ctl_status, sizeof(auth_ctl_status));
			bse_log_verbose(TAG, "hlp_iap_get_ch_data, status=%d, err = %d\r\n", CP_AUTH_CTL_AND_STATUS_GET_STATUS_BIT(auth_ctl_status), CP_AUTH_CTL_AND_STATUS_GET_ERR_BIT(auth_ctl_status));
		}

		if ((CP_AUTH_CTL_AND_STATUS_GET_ERR_BIT(auth_ctl_status) == 0) && (CP_AUTH_CTL_AND_STATUS_GET_STATUS_BIT(auth_ctl_status) == 0x01)) {
			ret = dep_read_data_from_cp(CP_SIGNATURE_DATA_LENGTH_REG, signature_data_length_byte, sizeof(signature_data_length_byte));
			p_ch->data_len = (signature_data_length_byte[0]<<8)|(signature_data_length_byte[1]);
			p_ch->data_cur = 0;

			bse_log_info(TAG, "ret = %d, signature_data_length = %d\r\n", ret, p_ch->data_len);
			if ((p_ch->data_len > 0) && (p_ch->data_len <= CP_SIGNATURE_DATA_SIZE_LIMIT)) {
				p_ch->p_buffer = (UINT8 *)NEW(p_ch->data_len);
				ret = dep_read_data_from_cp(CP_SIGNATURE_DATA_REG, p_ch->p_buffer, p_ch->data_len);

				if (ret == 0) {
					bse_log_info(TAG, "hlp_iap_get_ch_data, ret = %d\r\n", ret);
					FREE(p_ch->p_buffer);
					memset(p_ch, 0, sizeof(iap_cp_data_t));

					return 0;
				}
			}
		}
	}
	return 1;
}

static UINT16 hlp_iap_get_data_from_buff(UINT8 *buff, UINT16 buff_len)
{
	UINT16 valid_len = s_iap_data->p_cp_data.data_len - s_iap_data->p_cp_data.data_cur;
	UINT16 remain_len = buff_len > valid_len ? valid_len : buff_len;
	memcpy(buff, s_iap_data->p_cp_data.p_buffer + s_iap_data->p_cp_data.data_cur, remain_len);

	s_iap_data->p_cp_data.data_cur += remain_len;
	if (s_iap_data->p_cp_data.data_cur == s_iap_data->p_cp_data.data_len) {
		FREE(s_iap_data->p_cp_data.p_buffer);
		memset(&(s_iap_data->p_cp_data), 0, sizeof(s_iap_data->p_cp_data));
	}

	return remain_len;
}

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Accessory Certificate data total length.
-----------------------------------------------------------------------------*/
static void Porting_Auth_GetCertDataLen()
{
	if (hlp_iap_get_cert_data(&(s_iap_data->p_cp_data))) {
		bse_log_info(TAG, "EXT_Auth_GetCertDataLen, success!, %d\r\n", s_iap_data->p_cp_data.data_len);
		IAP2_Auth_CertDataLen(s_iap_data->p_cp_data.data_len);
	} else {
		bse_log_info(TAG, "EXT_Auth_GetCertDataLen, fail!\r\n");
	}
}

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Authentication Certificate data.

Arguments:
	buff:		[in] Specifies the buffer to return Authentication Certificate data;
	buff_len:	[in] Buffer length.
-----------------------------------------------------------------------------*/
static void Porting_Auth_GetCertData(UINT8 *buff, UINT16 buff_len)
{
	IAP2_Auth_CertData(hlp_iap_get_data_from_buff(buff, buff_len));
}

/*-----------------------------------------------------------------------------
Description:
	Callback function to compute a Response to the Authentication Challenge.
Arguments:
	data:		[in] Authentication Challenge;
	len:		[in] Authentication Challenge length.
-----------------------------------------------------------------------------*/
static void Porting_Auth_ChData(UINT8 *data, UINT16 data_len)
{
	if (hlp_iap_get_ch_data(&(s_iap_data->p_cp_data), data, data_len) > 0) {
		bse_log_info(TAG, "EXT_Auth_ChData, success!, %d\r\n", s_iap_data->p_cp_data.data_len);
		IAP2_Auth_ChRspDataLen(s_iap_data->p_cp_data.data_len);
	} else {
		bse_log_info(TAG, "EXT_Auth_ChData, fail!\r\n");
	}
}

/*-----------------------------------------------------------------------------
Description:
	Callback function to get Authentication Challenge Response data.
Arguments:
	buff:		[in] Response data buffer. Fill the Data in BUFDATA(data).
	buff_len:	[in] Authentication Challenge data offset.
-----------------------------------------------------------------------------*/
static void Porting_Auth_GetChRspData(UINT8 *buff, UINT16 buff_len)
{
	IAP2_Auth_ChRspData(hlp_iap_get_data_from_buff(buff, buff_len));
}
#endif	/* CONFIG_IAP2 */

