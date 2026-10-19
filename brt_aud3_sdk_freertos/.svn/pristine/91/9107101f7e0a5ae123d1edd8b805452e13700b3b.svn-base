/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_a2dp.c
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "global.h"
#include "auto_a2dp.h"
#include "a2dp_ui.h"
#include "brt_trace.h"
#include "internal_a2dp.h"

#define TAG "bse_a2dp"

#ifdef CONFIG_BIND_PROFILES
int a2dp_disallow_tmp = 0;
UINT8 deny_a2dp_address[6] = {0};
UINT8 deny_a2dp_initaddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif

void app_a2dp_init(module_event_cbk_t event_cbk)
{
	if (hlp_a2dp_get_status() != ST_A2DP_UNINIT) {
		return;
	}

	hlp_a2dp_init(event_cbk);
}

UINT8 app_a2dp_get_status(void)
{
	return hlp_a2dp_get_status();
}

void app_a2dp_query_status(void)
{
    hlp_a2dp_query_status();
}
void app_a2dp_server_start()
{
	if (hlp_a2dp_get_status() != ST_A2DP_INIT) {
		return;
	}

	hlp_a2dp_server_start();

#ifdef CONFIG_BIND_PROFILES
    //app_a2dp_set_deny(deny_a2dp_initaddr);
#endif
}

void app_a2dp_server_stop(void)
{

	if (hlp_a2dp_get_status() < ST_A2DP_READY) {
		return;
	}

	hlp_a2dp_server_stop();
}

void app_a2dp_done(void)
{

	if (hlp_a2dp_get_status() == ST_A2DP_UNINIT) {
		return;
	} else {
		app_a2dp_server_stop();
	}

	hlp_a2dp_done();
}

UINT8 app_a2dp_connect(UINT8 *bd_addr)
{ 	
#ifdef CONFIG_BIND_PROFILES
    if(a2dp_disallow_tmp && ((memcmp(deny_a2dp_address, bd_addr, 6) == 0) || memcmp(deny_a2dp_address, deny_a2dp_initaddr, 6) == 0))
    {
        bse_log_api(TAG, "A2DP is DENY LIST! a2dp_disallow_tmp = %d\r\n", a2dp_disallow_tmp);
        return -1;
    }
#endif
    struct A2DP_ConnectReqStru *in;
    struct A2DP_ServiceCapStru *sc;
    UINT8 *p;
    UINT8 len;
    UINT8 bit_pool = 32;

    if (hlp_a2dp_get_status() == ST_A2DP_READY) {
        hlp_a2dp_event_OK();
        hlp_a2dp_set_status(ST_A2DP_CONNECTING);

        in = NEW(sizeof(struct A2DP_ConnectReqStru));
        memset(in, 0, sizeof(struct A2DP_ConnectReqStru));
        in->cbk = NULL;
        memcpy(in->bd, bd_addr, BD_ADDR_LEN);
        in->src = 0;
        in->version = 0x0100;/* remote a2dp service version */

#ifdef CONFIG_BT_CHIP_CSR8350
        len = 13;
        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 9;
        p[4] = 0 << 4;
        p[5] = A2DP_CODEC_NONA2DP;
        p[6] = 0x4f;
        p[7] = 0x00;
        p[8] = 0x00;
        p[9] = 0x00;
        p[10] = 0x01;
        p[11] = 0x00;
        p[12] = A2DP_APTX_SF_44100 | A2DP_APTX_CHMODE_STEREO;		
        List_AddTail(&in->perfer, sc);
        len = 13;
        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 9;
        p[4] = 0 << 4;
        p[5] = A2DP_CODEC_NONA2DP;
        p[6] = 0x4f;
        p[7] = 0x00;
        p[8] = 0x00;
        p[9] = 0x00;
        p[10] = 0x01;
        p[11] = 0x00;
        p[12] = A2DP_APTX_SF_48000 | A2DP_APTX_CHMODE_STEREO;		
        List_AddTail(&in->perfer, sc);
#endif

        len = 10;
        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 6;
        p[4] = 0 << 4;
        p[5] = 0;
        p[6] = A2DP_SBC_SF_48000 | A2DP_SBC_CHMODE_JOINT_STEREO;
        p[7] = 0x10 | 0x04 | 0x01;
        p[8] = 2;
        p[9] = bit_pool;
        List_AddTail(&in->perfer, sc);

        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 6;
        p[4] = 0 << 4;
        p[5] = 0;
        p[6] = A2DP_SBC_SF_44100 | A2DP_SBC_CHMODE_JOINT_STEREO;
        p[7] = 0x10 | 0x04 | 0x01;
        p[8] = 2;
        p[9] = bit_pool;
        List_AddTail(&in->perfer, sc);

        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 6;
        p[4] = 0 << 4;
        p[5] = 0;
        p[6] = A2DP_SBC_SF_44100 | A2DP_SBC_CHMODE_STEREO;
        p[7] = 0x10 | 0x04 | 0x01;
        p[8] = 2;
        p[9] = bit_pool;
        List_AddTail(&in->perfer, sc);

        sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
        sc->len = len;
        p = sc->service_capability;
        p[0] = 1;
        p[1] = 0;
        p[2] = 7;
        p[3] = 6;
        p[4] = 0 << 4;
        p[5] = 0;
        p[6] = A2DP_SBC_SF_48000 | A2DP_SBC_CHMODE_STEREO;
        p[7] = 0x10 | 0x04 | 0x01;
        p[8] = 2;
        p[9] = bit_pool;
        List_AddTail(&in->perfer, sc);
        A2DP_Connect(in);
    } else {
        bse_log_api(TAG, "A2DP is Connected!\r\n");
        return -1;
    }

    return 0;
}

UINT8 app_a2dp_disconnect(void)
{
	bse_log_api(TAG, "%s", __FUNCTION__);
	if (hlp_a2dp_get_status() >= ST_A2DP_CONNECTED) {
        hlp_a2dp_event_OK();
		A2DP_Disconnect(hlp_a2dp_get_handle());
	} else {
		return -1;
	}
	return 0;
}

#ifdef CONFIG_BIND_PROFILES
void app_a2dp_set_deny(UINT8 *addr)
{
	a2dp_disallow_tmp = 1;
    if(addr != NULL) {
        memset(deny_a2dp_address, 0, 6);
        memcpy(deny_a2dp_address, addr, 6);
    } else {
        memcpy(deny_a2dp_address, deny_a2dp_initaddr, 6);
    }
   
    bse_log_api(TAG, "app_a2dp_set_deny a2dp_disallow_tmp = %d\r\n", a2dp_disallow_tmp);
}

void app_a2dp_cannel_deny()
{
    memset(deny_a2dp_address, 0, 6);
	a2dp_disallow_tmp = 0;
    bse_log_api(TAG, "app_a2dp_cannel_deny a2dp_disallow_tmp = %d\r\n", a2dp_disallow_tmp);
}
#endif

profile_t g_a2dp_profile = {
	app_a2dp_init,
	app_a2dp_done,
	app_a2dp_server_start,
	app_a2dp_server_stop,
};


