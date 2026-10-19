/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_gap.h
*
*	Description:
*
*
*	Create Date:		2014/07/31
***********************************************************************/

#ifndef _HLP_GAP_H_
#define _HLP_GAP_H_

#include "global.h"
#include "gap_ui.h"
#include "internal_manager.h"

#ifndef BD_NAME_LEN
#define BD_NAME_LEN 	20 /* included null-terminated character. */
#endif

#define LINK_KEY_LEN	16

enum {
    ST_BOND_NONE,
    ST_BOND_BONDED,
    ST_BOND_BONDING,
};

struct remote_dev_list_str
{
	struct BtList * list;
	SYSTEM_LOCK lock;
};

struct authorize_request_stru {
	int nType;
	unsigned char bd[BD_ADDR_LEN];
};

/* Controller Scan And Parity Mode Seting Mask */
#define BT_GAP_SCAN_MODE_DISABLE				(0)
#define BT_GAP_SCAN_MODE_ENABLE					(1)
#define BT_GAP_PARITY_MODE_DISABLE				(0<<1)
#define BT_GAP_PARITY_MODE_ENABLE				(1<<1)

#define GAP_USER_DATA			(s_gap_user_data)
#define GAP_STATE				(s_gap_user_data->state)
#define GAP_TL_HANDLE			(s_gap_user_data->tl_handle)


/* Interface */
void hlp_gap_init(module_event_cbk_t event_cbk);
void hlp_gap_start(void);
void hlp_gap_init_transport_layer(HANDLE hTLInst, TransportLayerStru *tl_funcs);
void hlp_gap_destory_transport_layer(HANDLE tl_hdl, GAP_UnregisterTLCfmCbk *cfm_cbk);
void hlp_gap_done(void);
void hlp_gap_excute_command(UINT32 op_index, void *context, void *in_par, UINT32 in_par_size);
void hlp_gap_excute_vendor_command(unsigned char nFlag, unsigned char ocf, void *param, unsigned char param_len);
void hlp_gap_excute_vendor_no_return_command(unsigned char nFlag, unsigned char ocf, void *param, unsigned char param_len);
void hlp_gap_reset_hardware(void);
void hlp_gap_set_local_device_class(UINT8 *p_class);
void hlp_gap_set_local_device_name(UINT8 *p_name);
void hlp_gap_set_bondable(UINT8 bondable);
void hlp_gap_set_ssp_mode(UINT8 mode);
void hlp_gap_get_local_device_addr(void);
void hlp_gap_get_remote_device_name(unsigned char *bd_addr);
void hlp_gap_get_connect_handle(UINT8 *bd, UINT16 *conn_hdl);
void hlp_gap_set_authentication_enable(UINT8 mode);
void hlp_gap_set_device_other_attribute(void);
void hlp_gap_reg_tl_cfm_cbk(HANDLE tl_handle, UINT8 result, struct GAP_RegisterTLCfmStru *cfm_par);
void hlp_gap_reset_hardware_cbk(void *context, UINT8 result, void *cfm_par);
void hlp_gap_excute_command_cbk(void *context, UINT8 result, struct GAP_ExecuteCommandCfmStru *cfm_par);
void hlp_gap_excute_vendor_cmd_cbk(void *context, UINT8 result, void *cfm_par);
void hlp_gap_set_status(UINT8 status);
UINT8 hlp_gap_get_status(void);
void hlp_gap_set_visiability(int visable);
void hlp_gap_set_bt_state(BOOL bt_state);
BOOL hlp_gap_get_bt_state(void);
UINT16 hlp_gap_get_connection_handle(UINT8 *remote_bd);
UINT8 hlp_gap_disconnect_acl_if_exist();
void hlp_gap_QoS_setup(UINT16 connection_handle);
void hlp_gap_load_paired_device(bt_remote_device_t * rmt_dev);
void *hlp_get_gap_handle(void);
UINT8 hlp_gap_get_scan_mode(void);
void hlp_gap_save_scan_mode(UINT8 scan_mode);
void hlp_set_usr_pair_mode(int usr_value);
void hlp_set_usr_ssp_mode(int ssp_value);
void hlp_set_usr_iocap_mode(int iocap_value);
void hlp_gap_response_ok();
UINT8 hlp_gap_get_scan_mode_ex(void);
bt_remote_device_t* hlp_gap_find_paired_device(UINT8*bd);
void hlp_gap_stop(void);
void hlp_gap_set_scan_mode(UINT8 mode);
void hlp_get_services_req(UINT8 *bd);
void hlp_gap_user_author_req(int profile_id, unsigned char select);
bt_status_t hlp_gap_add_eir_uuid(UINT8 *data, UINT8 data_len, UINT8 type);
bt_status_t hlp_gap_write_eir();
#endif /* _HLP_GAP_H_ */

