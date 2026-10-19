/*************************************************************************
    > File Name: patch_reconn.c
    > Author: hualiangyu
    > Mail: sheldon_wy@126.com
    > Created Time: Tue 28 Jul 2015 05:44:51 PM CST
 ************************************************************************/
#include<stdio.h>
#include "auto_gap.h"
#include "gap_ui.h"
#include "internal_gap.h"
#include "hci_ui.h"
#include "auto_commfunc.h"
#include "internal_manager.h"
#include "bt_power_manager.h"
#include "hci_stru.h"
#include "patch_reconn.h"
#include "auto_hf.h"
#include "hfp_ui.h"
#include "internal_hf.h"

#define TAG "bse_mgr"
static UINT8 g_patch_reconn_mgr = 0;

void pr_set_reconn_mgr(UINT8 val)
{
	bse_log_api(TAG, "pr_set_reconn_mgr val is %d\r\n", val);
	g_patch_reconn_mgr |= val;
}

void pr_clear_reconn_mgr()
{
	bse_log_api(TAG, "pr_clear_reconn_mgr\r\n");
	g_patch_reconn_mgr = 0;
}

BOOL pr_is_connect_by_app()
{
	return g_patch_reconn_mgr & PR_CONNECT_BY_APP? TRUE: FALSE;
}

BOOL pr_is_conn_pair_succ()
{
	return g_patch_reconn_mgr & PR_CONN_PAIR_SUCC ? TRUE: FALSE;
}

BOOL pr_is_hfp_conn_failed()
{
	return g_patch_reconn_mgr & PR_HFP_CONNECT_FAILED ? TRUE: FALSE;
}

BOOL pr_is_acl_disconnect_rem()
{
	return g_patch_reconn_mgr & PR_ACL_DISCONNECT_REM ? TRUE: FALSE;
}

BOOL pr_is_conn_key_miss()
{
	return g_patch_reconn_mgr & PR_CONN_KEY_MISS ? TRUE: FALSE;
}

BOOL pr_is_conn_by_remote()
{
	return g_patch_reconn_mgr & PR_CONNECT_BY_REMOTE ? TRUE: FALSE;
}

void patch_reconn(UCHAR* param)
{
	UCHAR bd[6] = {0};
	int i = 0;
	for(i = 0; i < 6; i++) {
		bd[i] = param[i];
	}
	bse_log_api(TAG, "patch_reconn bd is %02x:%02x:%02x:%02x:%02x:%02x\r\n", bd[0], bd[1], bd[2], bd[3], bd[4], bd[5]);

	if (hlp_hf_get_status() == ST_HF_READY) {
		HFP_Client_Start(hlp_hf_get_hfp_server_handle(), bd, NULL, HFP_ROLE_HF_CLIENT, hlp_hf_event_cbk);
		hlp_hf_set_status(ST_HF_CONNECTING);
		event_hf_indication(EV_HF_CONNECTING, bd);
	}
	free(param);
	pr_clear_reconn_mgr();
}
