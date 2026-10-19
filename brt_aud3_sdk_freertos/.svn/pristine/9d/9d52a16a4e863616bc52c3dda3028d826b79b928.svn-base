/*************************************************************************
    > File Name: auto_app/hlp_pan.c
    > Author: hualiangyu
    > Mail:
    > Created Time: Thu 25 Jun 2015 07:32:20 PM CST
 ************************************************************************/
#include "btdatatype.h"
#include<stdio.h>
#include <pthread.h>
#include "autoconf_bt.h"

#ifdef CONFIG_PAN
#include "btdatatype.h"
#include "btinit.h"
#include "internal_pan.h"
#include "auto_pan.h"
#include "pan_ui.h"
#include "pan_def.h"
#include "bt_power_manager.h"
#include "brt_trace.h"

#define TAG "bse_pan"
struct panUserDataStru {
	UINT8 conn_status;
	UINT8 bd[BD_ADDR_LEN];
	module_event_cbk_t event_cbk;
};

SYSTEM_LOCK s_lock;
static HANDLE g_LocalBridgePort = 0;
static struct PAN_PortStru *s_pan_port = NULL;
static struct panUserDataStru *s_pan_user_data = NULL;
static void hlp_pan_connect_thread(UINT8 *bd);

static INT32 SDPAN_PostConnected(void);
static void hlp_connection_event_cbk(WORD event, UCHAR *hdl, UCHAR * param);
static void hlp_pan_set_status(UINT8 status);

extern BOOL api_send_func(struct PAN_EthFrameStru *pfrm, HANDLE port_handle);
extern void data_rcv_cb(unsigned char *data, BOOL is_general_bd);
extern void Inter_Done(void);
struct FsmTimer *s_inter_init_timer = NULL;
static void inter_init_timer();

void hlp_pan_init(module_event_cbk_t event_cbk)
{
	if(s_pan_user_data == NULL) {
		s_pan_user_data = (struct panUserDataStru *) NEW(sizeof(struct panUserDataStru));
	}
	memset(s_pan_user_data, 0, sizeof(struct panUserDataStru));

	s_pan_user_data->event_cbk = event_cbk;
	s_inter_init_timer = NULL;
	s_lock = CREATE_CRITICAL(NULL);
	hlp_pan_set_status(ST_PAN_INIT);

	PANAPP_RegCbk(NULL, hlp_connection_event_cbk);
}

void hlp_pan_done()
{
	if(s_pan_user_data != NULL) {
		free(s_pan_user_data);
		s_pan_user_data = NULL;
	}
	if(s_inter_init_timer) {
		FSM_DelTimer(&s_inter_init_timer);
		s_inter_init_timer = NULL;
	}
	FREE_CRITICAL(s_lock);
	PAN_UnregExternPort(g_LocalBridgePort);
}

static void event_pan_indication(UINT8 event, void *param)
{
	if (s_pan_user_data && s_pan_user_data->event_cbk != NULL) {
		s_pan_user_data->event_cbk((PROFILE_ID_PAN << 8) | event, param);
	}
}

static void hlp_pan_set_status(UINT8 status)
{
	bse_log_api(TAG, "%s:%d: status is %d", __func__, __LINE__, status);
	if(s_pan_user_data)
		s_pan_user_data->conn_status = status;
}

UINT8 hlp_pan_get_status(void)
{
	if(s_pan_user_data) {
		return s_pan_user_data->conn_status;
	} else {
		return ST_PAN_UNINIT;
	}
}

/*Reference : Btsdk_Connect->Conn_StartClient->Pan_StartClient->Pan_StartClient_SvcConnPANU->SDPAN_PostConnected*/
void hlp_pan_connect(UINT8 *bd)
{

	pthread_t hlp_pan_connect_thread_id;
	pthread_create(& hlp_pan_connect_thread_id, NULL, hlp_pan_connect_thread, bd);
	pthread_detach(hlp_pan_connect_thread_id);
}
static void hlp_pan_connect_thread(UINT8 *bd)
{
	if(bd == NULL || s_pan_user_data == NULL) {
		bse_log_error(TAG, "PAN_DBG bd is error!!!");
		hlp_pan_set_status(ST_PAN_INIT);
		event_pan_indication(EV_PAN_DISCONNECTED, NULL);
		return ;
	}
	/*Set the local attribute*/
	struct PAN_ClntLocalAttr *loc_attr;
	loc_attr = (struct PAN_ClntLocalAttr *) NEW(sizeof(struct PAN_ClntLocalAttr *));
	memset(loc_attr, 0, sizeof(struct PAN_ClntLocalAttr));
	loc_attr->cbSize = sizeof(struct PAN_ClntLocalAttr);
	loc_attr->local_UUID = CLS_PAN_PANU;

	/*Set the remote device information*/
	struct SDAP_PANInfoStru *sdx_info;
	sdx_info = (struct SDAP_PANInfoStru *) NEW(sizeof(struct SDAP_PANInfoStru));
	memset(sdx_info, 0, sizeof(struct SDAP_PANInfoStru));
	sdx_info->size = sizeof(struct SDAP_PANInfoStru);
	sdx_info->svc_type = CLS_PAN_NAP;

	/*Set status, and send notifications*/
	hlp_pan_set_status(ST_PAN_CONNECTING);
	event_pan_indication(EV_PAN_CONNECTING, bd);
	memcpy(s_pan_user_data->bd, bd, BD_ADDR_LEN);

	/*Connect BNEP, NOTE: This is a synchronization function*/
	s_inter_init_timer = FsmAddTimer(NULL, 500, 0, NULL, NULL, (UINT32)inter_init_timer);
	bse_log_info(TAG, "s_inter_init_timer start [%x]", s_inter_init_timer);
	s_pan_port = PAN_ClntStart(bd, sdx_info, loc_attr);
	if(s_pan_port == NULL) {
		if(s_inter_init_timer) {
			FSM_DelTimer(&s_inter_init_timer);
			s_inter_init_timer = NULL;
		}
		hlp_pan_set_status(ST_PAN_INIT);
		event_pan_indication(EV_PAN_DISCONNECTED, bd);
		memset(s_pan_user_data->bd, 0, BD_ADDR_LEN);
		return;
	}
	bse_log_info(TAG, "s_pan_port reserved1 is %d", s_pan_port->reserved1);

	/*Register func to stack*/
	g_LocalBridgePort = PAN_RegExternPort(api_send_func);
	bse_log_info(TAG, "bnep_handle is %x", s_pan_port->bnep_handle);

}

static void inter_init_timer()
{
	bse_log_api(TAG, "%s:%d", __func__, __LINE__);
	if(s_pan_user_data == NULL)
		return ;
	UINT8 * t_bd = hlp_manager_get_local_bd_addr();
	UINT8 local_bd[6];
	memset(local_bd, 0, 6);
	int j = 0;
	for( j = 0; j < 6; j++) {
#if 0
		if(j == 0) {
			/* TUN assigned MAC address use the Bluetooth address to ensure that
			 * the relevance of TUN and Bluetooth , due to the restrictions of
			 * MAC address, we need to modify the first byte to 0x00.*/
			local_bd[0] == 0x00;
		} else
#endif
		{
			local_bd[j] = t_bd[5-j];
		}
	}
	bse_log_info(TAG, "get local bd is %02x:%02x:%02x:%02x:%02x:%02x", local_bd[0], local_bd[1],local_bd[2],local_bd[3],local_bd[4],local_bd[5]);

	ENTER_CRITICAL(s_lock);
	BOOL ret = Inter_Init(local_bd, data_rcv_cb);
	LEAVE_CRITICAL(s_lock);
	if(ret > 0) {
		hlp_pan_set_status(ST_PAN_CONNECTED);
		event_pan_indication(EV_PAN_CONTROL_STATE, "bt-pan");
		event_pan_indication(EV_PAN_CONNECTED, s_pan_user_data->bd);
	} else {
//		event_pan_indication(EV_PAN_DISCONNECTED, s_pan_port->bd);
		hlp_pan_disconnect();
	}
	bse_log_info(TAG, "s_inter_init_timer is %d", s_inter_init_timer);
	s_inter_init_timer = 0;
}

void hlp_pan_disconnect()
{
	if(s_pan_port && hlp_pan_get_status() == ST_PAN_CONNECTED) {
		PAN_Disconn(s_pan_port);

		PAN_UnregExternPort(g_LocalBridgePort);
		if(s_inter_init_timer) {
			FSM_DelTimer(&s_inter_init_timer);
			s_inter_init_timer = NULL;
		} else {
			ENTER_CRITICAL(s_lock);

			pthread_t inter_done_id;
			pthread_create(&inter_done_id, NULL, Inter_Done, NULL);
			pthread_detach(inter_done_id);
			LEAVE_CRITICAL(s_lock);
		}

		event_pan_indication(EV_PAN_DISCONNECTED, s_pan_user_data->bd);
		hlp_pan_set_status(ST_PAN_INIT);
		memset(s_pan_user_data->bd, 0, BD_ADDR_LEN);
	}
}

static void hlp_connection_event_cbk(WORD event, UCHAR *hdl, UCHAR * param)
{
	bse_log_api(TAG, " hlp_connection_event_cbk event is %d, hdl is %p", event, hdl);
	switch(event) {
	case BTAPP_EV_DISC: {
		struct PAN_PortStru *port = hdl;
		if(s_inter_init_timer) {
			FSM_DelTimer(&s_inter_init_timer);
			s_inter_init_timer = NULL;
		}
		if(port) {
			ENTER_CRITICAL(s_lock);
			pthread_t inter_done_id;
			pthread_create(&inter_done_id, NULL, Inter_Done, NULL);
			pthread_detach(inter_done_id);
			LEAVE_CRITICAL(s_lock);

			PAN_UnregExternPort(g_LocalBridgePort);
			hlp_pan_set_status(ST_PAN_INIT);
			event_pan_indication(EV_PAN_DISCONNECTED, s_pan_user_data->bd);
			//	port = NULL;
		}
	}
	break;
	default:
		break;
	}
}

void data_rcv_cb(unsigned char *data, BOOL is_general_bd)
{
	struct PAN_EthFrameStru *pfrm = data;
	UINT8 dst_bd[BD_ADDR_LEN] = {0};
	memcpy(dst_bd, pfrm->frame, BD_ADDR_LEN);
	PAN_SendEthFrm(PAN_GENERAL_ETHERNET, pfrm, TRUE);
}

#endif  //CONFIG_PAN
