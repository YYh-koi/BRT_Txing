/*************************************************************************
    > File Name: auto_app/patch_iphone.h
    > Author: hualiangyu
    > Mail: sheldon_wy@126.com
    > Created Time: Tue 28 Jul 2015 03:04:05 PM CST
 ************************************************************************/


#define PR_CONNECT_BY_APP 0x01
#define PR_CONN_PAIR_SUCC (0X01 << 1)
#define PR_HFP_CONNECT_FAILED (0X01 << 2)
#define PR_ACL_DISCONNECT_REM (0X01 << 3)
#define PR_CONN_KEY_MISS (0X01 << 4)
#define PR_CONNECT_BY_REMOTE (0x01 << 5)

void pr_set_reconn_mgr(UINT8 val);
void pr_clear_reconn_mgr();
BOOL pr_is_connect_by_app();
BOOL pr_is_conn_pair_succ();
BOOL pr_is_hfp_conn_failed();
BOOL pr_is_acl_disconnect_rem();
BOOL pr_is_conn_key_miss();
BOOL pr_is_conn_by_remote();
void patch_reconn(UCHAR* param);
