/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_gap.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "auto_gap.h"
#include "internal_gap.h"
#include "auto_commfunc.h"
#include "brt_trace.h"

#define TAG "bse_gap"
void app_gap_init(module_event_cbk_t event_cbk)
{
	if (hlp_gap_get_status() != ST_GAP_UNINIT) {
		return;
	}
	
	hlp_gap_init(event_cbk);
}

void app_gap_server_start(void)
{
	if (hlp_gap_get_status() != ST_GAP_INIT) {
		return;
	}

	hlp_gap_start();
}

void app_gap_server_stop(void)
{
	if (hlp_gap_get_status() < ST_GAP_READY) {
		return;
	}

	hlp_gap_stop();
}

void app_gap_done(void)
{
	if (hlp_gap_get_status() == ST_GAP_UNINIT) {
		return;
	} else {
		app_gap_server_stop();
	}

	hlp_gap_done();
}

void app_gap_vendor_command(unsigned char nFlag, unsigned char ocf, void *param, unsigned char param_len)
{
	if (hlp_gap_get_status() >= ST_GAP_READY) {
		hlp_gap_excute_vendor_command(nFlag, ocf, param, param_len);
	}
}

void app_gap_init_transport_layer(HANDLE hTLInst,TransportLayerStru *tl_funcs)
{
	hlp_gap_init_transport_layer(hTLInst, tl_funcs);
}

void app_gap_destory_transport_layer(HANDLE tl_hdl, GAP_UnregisterTLCfmCbk *cfm_cbk)
{
	hlp_gap_destory_transport_layer(tl_hdl, cfm_cbk);
}

void app_gap_get_remote_device_name(UINT8 *bd_addr)
{
	// TODO Why not ST_GAP_WORKING?
	if (hlp_gap_get_status() >= ST_GAP_READY) {
		hlp_gap_get_remote_device_name(bd_addr);
	}
}

void app_gap_scan_mode_control(UINT8 type, BOOL ctl)
{
	if (hlp_gap_get_status() != ST_GAP_WORKING) {
		return;
	}

	UINT8 mode = 0x00;
	UINT8 pr_mode = hlp_gap_get_scan_mode_ex();
	if (type < BT_SET_SCAN_MODE_TYPE_DOUBLE) {
		pr_mode &= ~(1<<type);
		pr_mode |= ctl<<type;
		mode = pr_mode;
	} else if (type == BT_SET_SCAN_MODE_TYPE_DOUBLE) {
		mode = (ctl<<BT_SET_SCAN_MODE_TYPE_SCAN) | (ctl<<BT_SET_SCAN_MODE_TYPE_PAIR);
	} else {
		mode = pr_mode;
	}

	if(0)//ctl
	{
		printf("disconn open scan-------------------\n");
		hlp_gap_set_scan_mode(1);
		hlp_gap_set_scan_mode(3);
	}

	hlp_gap_set_scan_mode(mode);
}

void app_gap_set_scan_mode(UINT8 scan_mode)
{
	if (hlp_gap_get_status() != ST_GAP_WORKING) {
		return;
	}

	hlp_gap_set_scan_mode(scan_mode);
	hlp_gap_save_scan_mode(scan_mode);
	hlp_gap_response_ok();
}

void app_gap_get_scan_mode(void)
{
    if (hlp_gap_get_status() != ST_GAP_WORKING) {
		return;
	}
	hlp_gap_get_scan_mode();
}

UINT8 app_gap_get_scan_mode_ex(void)
{
    if (hlp_gap_get_status() != ST_GAP_WORKING) {
		return -1;
	}
	return hlp_gap_get_scan_mode_ex();
}

void app_gap_get_connect_handle(UINT8 *bd, UINT16 *conn_hdl)
{
	if (hlp_gap_get_status() != ST_GAP_WORKING) {
		return;
	}

	hlp_gap_get_connect_handle(bd, conn_hdl);
}

void app_gap_reset_hardware()
{
	//if (hlp_gap_get_status() != ST_GAP_READY) {
		//return;
	//}
	bse_log_info(TAG,"%s", __func__);
	hlp_gap_reset_hardware();
}

void app_gap_add_eir(UINT8 *p_uuid, UINT8 len, UINT8 type)
{
	if (hlp_gap_get_status() < ST_GAP_READY) {
		return;
	}

	hlp_gap_add_eir_uuid(p_uuid, len, type);
}

void app_get_services(UINT8 *bd)
{
	if(bd != NULL) {
		hlp_get_services_req(bd);
	} else {
		return;
	}
}

UINT8* app_get_device_cod()
{
	return  hlp_manager_get_local_device_class();
	//bse_log_info("cod = %s", p);
	
}

void app_set_ssp_mode(int mode)
{
	hlp_gap_set_ssp_mode(mode);
}

void app_set_usr_pair_mode(int usr_value)
{
	hlp_set_usr_pair_mode(usr_value);
}

void app_set_ssp_cap_mode(int usr_value)
{
	hlp_set_usr_ssp_mode(usr_value);
}

void app_set_io_cap_mode(int usr_value)
{
	hlp_set_usr_iocap_mode(usr_value);
}

void app_gap_user_author_req(int profile_id, unsigned char select)
{
	bse_log_info(TAG, "app_gap_user_author_req profileID[%x]select[%x]", profile_id, select);
	hlp_gap_user_author_req(profile_id, select);
}

profile_t g_gap_profile = {
	app_gap_init,
	app_gap_done,
	app_gap_server_start,
	app_gap_server_stop,
};


