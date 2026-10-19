/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_avrcp.c
*
*	Description:	This module defines all of the AVRCP 1.4 advanced
					control application interface functions.
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "auto_avrcp.h"
#include "internal_avrcp.h"
#include "avrcp_ui.h"
#include "avctp_ui.h"
#include "brt_trace.h"
#include "auto_commfunc.h"
#include "bt_event.h"
#include "bt_power_manager.h"
#include "bt_manager.h"
#include "sdap_uimacro.h"
#include "sdap_uistru.h"
#include "sdap_uifunc.h"
#include "bip_ui.h"
#include "brlinkd.h"

#define TAG "bse_avrcp"

struct APP_AVRCP_ResourceStru *s_avrcp_user_data = NULL;
static struct FsmTimer* s_avrcp_fast_forwar_timer = NULL;
static struct AVRCP_PassThroughStru s_avrcp_fastforward_t = {0};
#define AVRCP_USER_DATA		(s_avrcp_user_data)
#define AVRCP_STATE			(s_avrcp_user_data->state)

tImageHandle old_cov_handle = -1;

static UINT8 avrcp_notif_events[] = {
	AVRCP_EVENT_PLAYBACK_STATUS_CHANGED,
	AVRCP_EVENT_TRACK_CHANGED,
	AVRCP_EVENT_PLAYBACK_POS_CHANGED,
	AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED,
	AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED,
	AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED,
	AVRCP_EVENT_UIDS_CHANGED
};

static UINT32 avrcp_element_attr[] = {
	AVRCP_MA_PLAYING_TIME,
	AVRCP_MA_TITLEOF_MEDIA,
	AVRCP_MA_NAMEOF_ARTIST,
	AVRCP_MA_NAMEOF_ALBUM,
#ifdef CONFIG_BIP_CLIENT
	AVRCP_MA_IMAGE_HANDLE
#endif
};

static struct FsmTimer *avrcp_get_p_timer = NULL;
static uint32_t s_abs_lable;
void event_avrcp_indication(UINT8 event, void *param)
{
	if (s_avrcp_user_data->event_cbk != NULL) {
		s_avrcp_user_data->event_cbk((PROFILE_ID_AVRCP << 8) | event, param);
	}
}

void hlp_avrcp_init(module_event_cbk_t event_cbk)
{
	if (s_avrcp_user_data == NULL) {
		s_avrcp_user_data = (struct APP_AVRCP_ResourceStru *)NEW(sizeof(struct APP_AVRCP_ResourceStru));
	}

	memset(s_avrcp_user_data, 0, sizeof(struct APP_AVRCP_ResourceStru));
	s_avrcp_user_data->capability = 0;
	s_avrcp_user_data->event_cbk = event_cbk;
    s_avrcp_user_data->config = 3;
	hlp_avrcp_set_status(ST_APP_AVRCP_INIT);
	bta_bip_init();
}

#ifdef CONFIG_CONNECTION_ATHENTICATE_REQUIRED
static void hlp_avrcp_set_security()
{
	struct HCI_SecurityInfoStru *avrcp_reg = NULL;

	avrcp_reg = List_NodeNew(sizeof(struct HCI_SecurityInfoStru));
	avrcp_reg->proto_id = PROT_L2CAP;
	avrcp_reg->channel = PSM_AVCTP;
	avrcp_reg->mask = HCI_SECURITY_AUTHORIZE|HCI_SECURITY_AUTHENTICATE|HCI_SECURITY_ENCRYPT|HCI_SECURITY_INCOMING;
	HCI_Security_RequireReg(avrcp_reg);
}
#endif

void hlp_avrcp_server_start(void)
{
#ifdef CONFIG_BIP_CLIENT
     // AVRCP_ServerStart(AVRCP_CT, 0x03c1);
#ifdef CONFIG_ABS_VOL
	AVRCP_ServerStart(AVRCP_CT, AVRCP_ALL_CT_FEATURE | AVCT_FEATURE_CA_GET_THUMBNAIL | AVCT_FEATURE_CA_GET_IMAGE | AVCT_FEATURE_CA_GET_IMAGE_PROPERTIES);	
    AVRCP_ServerStart(AVRCP_TG, AVTG_FEATURE_BROWSING | AVTG_CATEGORY_2);
#else
	AVRCP_ServerStart(AVRCP_CT, AVRCP_ALL_CT_FEATURE | AVCT_FEATURE_CA_GET_THUMBNAIL | AVCT_FEATURE_CA_GET_IMAGE|AVCT_FEATURE_CA_GET_IMAGE_PROPERTIES);
#endif
#else
    AVRCP_ServerStart(AVRCP_CT, 0);
#endif
	AVRCP_RegCbk((UINT32 *)hlp_avrcp_cbk);
#ifdef CONFIG_CONNECTION_ATHENTICATE_REQUIRED
	hlp_avrcp_set_security();
#endif
	hlp_avrcp_set_status(ST_APP_AVRCP_READY);
}

void hlp_avrcp_server_stop(void)
{
	AVRCP_ServerStop();
	AVRCP_RegCbk(NULL);
	hlp_avrcp_set_status(ST_APP_AVRCP_INIT);
}

void hlp_avrcp_done(void)
{
	if(avrcp_get_p_timer != NULL)
	{
		FSM_DelTimer(&avrcp_get_p_timer);
		avrcp_get_p_timer = NULL;
	}
	if (s_avrcp_user_data != NULL) {
		FREE(s_avrcp_user_data);
		s_avrcp_user_data = NULL;
	}
}

void hlp_avrcp_set_config(UINT8 param)
{
	bse_log_api(TAG,"%s, param=%d\r\n", __func__, param);
	AVRCP_USER_DATA->config = param;
}

UINT8 hlp_avrcp_get_config()
{
	return AVRCP_USER_DATA->config;
}

void hlp_avrcp_connect(UINT8 *remote_bd)
{
	struct AVRCP_ConnectReqStru *in = (struct AVRCP_ConnectReqStru *)NEW(sizeof(struct AVRCP_ConnectReqStru));

	hlp_avrcp_set_status(ST_APP_AVRCP_CONNECTING);
	memcpy(in->bd_addr, remote_bd, BD_ADDR_LEN);
	in->browsing_support = TRUE; /* remote device supports browsing channel or not */
	AVRCP_ConnectReq(in);
}

void hlp_avrcp_disconnect(UINT8 *remote_bd)
{
	if (remote_bd != NULL) {
		AVRCP_DisconnectReq(remote_bd);
	}
}
void hlp_avrcp_button_release_timer(void *param)
{
    struct AVRCP_PassThroughStru *in = (struct AVRCP_PassThroughStru *)param;
    if (in) {
        bse_log_api(TAG,"hlp_avrcp_timer_passthrough_req, cmd=%d, state = %d\r\n", in->vendor_unique_id, AVRCP_STATE);
        if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {
            AVRCP_PassThrough_Req(hlp_avrcp_get_connected_bd(), in);
			if (!bt_manager_get_timer_notify_status()) {
            	hlp_avrcp_get_play_status(AVRCP_USER_DATA->remote_bd);
			}
        }
        FREE(in);
    }
}

void hlp_avrcp_passthrough_req(UINT8 *remote_bd, UINT8 op_id, UINT8 vendor_id)
{
	struct AVRCP_PassThroughStru *in = (struct AVRCP_PassThroughStru *)NEW(sizeof(struct AVRCP_PassThroughStru));
	in->vendor_unique_id = vendor_id;
	in->state_flag = AVRCP_STATE_FLAG_BUTTON_PUSHED;
	in->op_id = op_id;
	AVRCP_PassThrough_Req(remote_bd, in);

	in->vendor_unique_id = vendor_id;
	in->state_flag = AVRCP_STATE_FLAG_BUTTON_RELEASED;
	in->op_id = op_id;
	AVRCP_PassThrough_Req(remote_bd, in);

	if(op_id == AVRCP_CMD_OPID_FORWARD || op_id == AVRCP_CMD_OPID_BACKWARD)
	{
		bse_log_api(TAG, "[Req] hlp_avrcp_get_element_attr_req>\r\n");
		hlp_avrcp_get_element_attr_req(remote_bd, 0);
	}

	if (op_id == AVRCP_CMD_OPID_FAST_FORWARD || op_id == AVRCP_CMD_OPID_REWIND)
    {
        bse_log_api(TAG,"Fast Forward/Rewind....\r\n");
        FsmAddTimer(NULL,300,NULL,(void *)in,"button_release",(UINT32)hlp_avrcp_button_release_timer);
    }
    else 
    {
        FREE(in);
        //FsmAddTimer(NULL,300,NULL,(void *)in,"button_release",(UINT32)hlp_avrcp_button_release_timer);
    }	
}

static void get_rcfeature_cbk(void *app_context, UINT8 *bd, UINT16 status, UINT16 svc_type, struct SDAP_AVRCPInfoStru *sdx_info)
{
	bt_bdaddr_t address;
	memcpy(address.address, s_avrcp_user_data->remote_bd, BD_ADDR_LEN);
    if (status == ERROR_SDAP_SUCCESS && sdx_info){

    	bse_log_api(TAG, "get_rcfeature, sdx_info->sup_cg=0x%x, sdx-info->psm=%d", sdx_info->sup_cg, sdx_info->psm);
        if ((sdx_info->sup_cg & AVTG_FEATURE_COVER_ART) == AVTG_FEATURE_COVER_ART) {
	        set_bip_l2cap_psm(sdx_info->psm);
    	    	  bse_log_api(TAG, "get_rcfeature , sdx-info->psm=%d", sdx_info->psm);
	        if (sdx_info->psm != 0) {
                bta_bip_open(address.address);
            }
        }
	} else {
	    bse_log_api(TAG, "get_rcfeature failed, status = %d, sdx-info=%p", status, sdx_info);
	}
	//HAL_CBACK(s_avrcp_user_data->avrcp_cbks, getrcfeatures_cb, &address, rcfeature);
}

static void hlp_avrcp_get_rcfeature_req(UINT8 *bd)
{
	SDAP_GetServiceInfoBDReq(bd, CLS_AVRCP_TG, NULL, 0, NULL, (SDAP_GetServiceInfoBDRsp_Func *)get_rcfeature_cbk);
}

void hlp_avrcp_getcaps_req(UINT8 *bd, UINT8 capability_id)
{
	bse_log_api(TAG, "[Req] Get Capabilities <EVENTS_SUPPORTED>\r\n");
	AVRCP_AdvancedControl_Req(bd, AVRCP_PDUID_GET_CAPABILITIES, &capability_id, sizeof(UINT8));
}

void hlp_avrcp_get_nowplaying_req(UINT8 *bd, UINT32 start_item, UINT32 end_item)
{
	struct AVRCP_GetFolderItemsReqStru *req;
	UINT32 size = sizeof(struct AVRCP_GetFolderItemsReqStru) + (ELEMENT_COUNT - 1) * sizeof(UINT32);
	UINT32 n = 0;
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_FOLDER_ITEMS);

	req = (struct AVRCP_GetFolderItemsReqStru *)NEW(size);
	memset(req, 0, size);
	req->scope = AVRCP_SCOPE_MEDIAPLAYER_NOWPLAYING;
	req->start_item = start_item;
	req->end_item = end_item;
	req->attr_count = ELEMENT_COUNT;
	while (n < ELEMENT_COUNT) {
		req->attr_id[n] = avrcp_element_attr[n];
		n++;
	}

	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, req, size);
	FREE(req);
}

void hlp_avrcp_get_element_attr_req(UINT8 *bd, UINT8 intentional)
{
	struct AVRCP_IDNum4IDStru *req;
	UINT32 tlcr_pdu_id = 0;
	int num = intentional ? ELEMENT_COUNT + 1 : ELEMENT_COUNT;
	UINT16 len = sizeof(struct AVRCP_IDNum4IDStru) + (num - 1) * sizeof(UINT32);
	UINT8 n = 0;

	bse_log_api(TAG, "hlp_avrcp_get_element_attr_req intentional=%d\r\n", intentional);
	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_ELEMENT_ATTRIBUTES);

	req = NEW(len);
	req->num =num;
	memset(req->identifier, 0, 8);
	while (n < num) {
		req->attr_id[n] = avrcp_element_attr[n];
		n++;
	}
	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, req, len);
	FREE(req);
}

void hlp_avrcp_play_item_req(UINT8 *bd, UINT8 *uid, UINT16 uid_counter, UINT8 scope)
{
	struct AVRCP_PlayItemReqStru req;
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_PLAY_ITEM);
	req.scope = scope;
	req.uid_counter = uid_counter;
	memcpy(req.uid, uid, 8);
	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, &req, sizeof(struct AVRCP_PlayItemReqStru));
}

static tImageHandle convert_handle(short img_handle_len, unsigned char * img_handle)
{
	tImageHandle ret_handle;
	char * handle = (char *)NEW(img_handle_len + 1);
	memset(handle, 0, img_handle_len + 1);
	memcpy(handle, img_handle, img_handle_len);
	ret_handle = (tImageHandle) atol(handle);
	FREE(handle);

	return ret_handle;
}
struct AVRCPDataIndStru *hlp_avrcp_create_media_info(UINT8 num, struct AVRCP_4IDStringStru *in)
{
	UINT8 n = 0;
	UINT32 len = 0;
	struct AVRCPDataIndStru *p_data = (struct AVRCPDataIndStru*)NEW(sizeof(struct AVRCPDataIndStru));

	memset(p_data, 0, sizeof(struct AVRCPDataIndStru));
	while (n < num) {
		len = (in + n)->len > MAX_STR_LEN ? MAX_STR_LEN : (in + n)->len;
		switch ((in + n)->attr_id) {
		case AVRCP_MA_PLAYING_TIME:
			memcpy(p_data->time, (in + n)->value, len > AVRCP_TIME_LEN - 1 ? AVRCP_TIME_LEN - 1 : len);
			break;
		case AVRCP_MA_TITLEOF_MEDIA:
			p_data->title = (UINT8 *)NEW(len+1);
			if (p_data->title != NULL) {
				memset(p_data->title, 0, len+1);
				memcpy(p_data->title, (in + n)->value, len);
			}
			break;
		case AVRCP_MA_NAMEOF_ARTIST:
			p_data->artist = (UINT8 *)NEW(len+1);
			if (p_data->artist != NULL) {
				memset(p_data->artist, 0, len+1);
				memcpy(p_data->artist, (in + n)->value, len);
			}
			break;
		case AVRCP_MA_NAMEOF_ALBUM:
			p_data->album = (UINT8 *)NEW(len+1);
			if (p_data->album != NULL) {
				memset(p_data->album, 0, len+1);
				memcpy(p_data->album, (in + n)->value, len);
			}
			break;
#ifdef CONFIG_BIP_CLIENT
		case AVRCP_MA_IMAGE_HANDLE:
			p_data->img_handle = (UINT8 *)NEW(len+1);
			if (p_data->img_handle != NULL) {
				memset(p_data->img_handle, 0, len+1);
				memcpy(p_data->img_handle, (in + n)->value, len);
				tImageHandle cov_handle = convert_handle(len, (in + n)->value);
								
				bse_log_info(TAG, "hlp_avrcp_create_media_info cov_handle=[%ld]", cov_handle);
				if(old_cov_handle != cov_handle) {
					old_cov_handle = cov_handle;
					set_bip_cover_handle(cov_handle);
					if(sw_get_avrcp_bip_permission() || (hlp_avrcp_get_config() & PROFILE_AVRCP_MEDIA_IMAGE))
						bta_bip_update_cover_art(NULL);
				}
			}			
			break;
#endif
		default:
			break;
		}
		n++;
	}

	if (p_data->title == NULL) {
		p_data->title = NEW(1);
		p_data->title[0] = 0;
	}
	if (p_data->artist == NULL) {
		p_data->artist = NEW(1);
		p_data->artist[0] = 0;
	}
	if (p_data->album == NULL) {
		p_data->album = NEW(1);
		p_data->album[0] = 0;
	}
	if (p_data->img_handle == NULL) {
		p_data->img_handle = NEW(1);
		p_data->img_handle[0] = 0;
	}
	return p_data;
}

void hlp_avrcp_destroy_media_info(struct AVRCPDataIndStru *p_data)
{
	if (p_data == NULL)
		return;

	if (p_data->title != NULL) {
		FREE(p_data->title);
	}

	if (p_data->artist!= NULL) {
		FREE(p_data->artist);
	}

	if (p_data->album!= NULL) {
		FREE(p_data->album);
	}

	FREE(p_data);
}

void hlp_avrcp_get_play_status(UINT8 *bd)
{
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_PLAY_STATUS);
	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, NULL, 0);
}
void hlp_avrcp_get_play_status_timer()
{
    UINT32 tlcr_pdu_id = 0;
    if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {
        AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_PLAY_STATUS);
        AVRCP_AdvancedControl_Req(hlp_avrcp_get_connected_bd(), tlcr_pdu_id, NULL, 0);
    }
}

void hlp_avrcp_get_folder_items(UINT8 *bd, UINT8 scope, UINT32 start_item, UINT32 end_item)
{
	struct AVRCP_GetFolderItemsReqStru *req;
	UINT32 size = sizeof(struct AVRCP_GetFolderItemsReqStru) + (ELEMENT_COUNT - 1) * sizeof(UINT32);
	UINT32 n = 0;
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_FOLDER_ITEMS);

	req = (struct AVRCP_GetFolderItemsReqStru *)NEW(size);
	memset(req, 0, size);
	req->scope = scope;
	req->start_item = start_item;
	req->end_item = end_item;
	req->attr_count = ELEMENT_COUNT;
	while (n < ELEMENT_COUNT) {
		req->attr_id[n] = avrcp_element_attr[n];
		n++;
	}

	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, req, size);
	FREE(req);
}

void hlp_avrcp_set_player_app_setting_value_req(UINT8 *remote_bd, UINT8 attribute_id,UINT8 value)
{/*response have no parameters*/
	struct AVRCP_NumIDPairStru *opstr = (struct AVRCP_NumIDPairStru *)NEW(sizeof(struct AVRCP_NumIDPairStru));
	UINT16 len = sizeof(struct AVRCP_NumIDPairStru);
	opstr->num = 1;
	opstr->id[0].attr_id = attribute_id;
	opstr->id[0].value_id = value;
	
	AVRCP_AdvancedControl_Req(remote_bd, AVRCP_PDUID_SET_CURRENTPLAYER_APPLICATION_SETTING_VALUE, (void*)opstr, len);
	FREE(opstr);
}

void hlp_avrcp_get_folder_list(UINT8 *bd, UINT32 start_item, UINT32 end_item)
{
	struct AVRCP_GetFolderItemsReqStru *req;
	UINT32 size = sizeof(struct AVRCP_GetFolderItemsReqStru) + (ELEMENT_COUNT - 1) * sizeof(UINT32);
	UINT32 n = 0;
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_GET_FOLDER_ITEMS);

	req = (struct AVRCP_GetFolderItemsReqStru *)NEW(size);
	memset(req, 0, size);
	req->scope = AVRCP_SCOPE_MEDIAPLAYER_VIRTUAL_FILESYSTEM;
	req->start_item = start_item;
	req->end_item = end_item;
	req->attr_count = ELEMENT_COUNT;
	while (n < ELEMENT_COUNT) {
		req->attr_id[n] = avrcp_element_attr[n];
		n++;
	}

	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, req, size);
	FREE(req);
}

void hlp_avrcp_change_path(UINT8 *bd, UINT16 uid_counter, UINT8 direction, UINT8 *folder_uid)
{
	struct AVRCP_ChangePathReqStru req;
	UINT32 tlcr_pdu_id = 0;

	AVRCP_SET_PDUID(tlcr_pdu_id, AVRCP_PDUID_CHANGE_PATH);

	req.uid_counter = uid_counter;
	req.direction = direction;
	memcpy(req.folder_uid, folder_uid, sizeof(req.folder_uid));

	AVRCP_AdvancedControl_Req(bd, tlcr_pdu_id, &req, sizeof(req));
}

void hlp_avrcp_get_folder_items_cfm(UINT8 rsp_type, struct AVRCP_GetFolderItemsRspStru *in)
{
	struct avrcp_get_folder_res_items_type *item_res = in->item;

	if ((rsp_type == AVRCP_RSP_REJECTED) || (in->status != AVRCP_ERROR_SUCCESSFUL)) {
		event_avrcp_indication(EV_AVRCP_ELEMENT_END, 0);
		return;
	}

	switch (item_res->item_type) {
	case AVRCP_ITEMTYPE_MEDIAPLAYER_ITEM: {
		struct AVRCP_MediaPlayerItemStru *item = in->item;
		UINT8 buff[128] = {0};

		memcpy(buff, item->name, (item->name_len > 127 ? 127 : item->name_len));
		bse_log_info(TAG, "AVRCP_ITEMTYPE_MEDIAPLAYER_ITEM player id=[%d] name=[%s]", item->player_id, buff);
	}
	break;
	case AVRCP_ITEMTYPE_FOLDER_ITEM: {
		struct AVRCP_FolderItemStru *item = in->item;
		struct AVRCPFolderInfoIndStru *p_data = (struct AVRCPFolderInfoIndStru *)NEW(sizeof(struct AVRCPFolderInfoIndStru));
		int len = item->name_len > AVRCP_FOLDER_LEN - 1 ? AVRCP_FOLDER_LEN - 1 : item->name_len;

		memset(p_data, 0, sizeof(struct AVRCPFolderInfoIndStru));
		memcpy(p_data->name, item->name, len);
		memcpy(p_data->uid, item->folder_uid, sizeof(p_data->uid));
		event_avrcp_indication(EV_AVRCP_FOLDER_LIST_INFO, p_data);
		FREE(p_data);
	}
	break;
	case AVRCP_ITEMTYPE_MEDIAELEMENT_ITEM: {
		struct AVRCP_MediaElementItemStru *item = in->item;
		struct AVRCPDataIndStru *p_data = NULL;

		p_data = hlp_avrcp_create_media_info(item->attr_num, item->entry);
		memcpy(p_data->uid, item->element_uid, sizeof(item->element_uid));
		event_avrcp_indication(EV_AVRCP_ELEMENT_ATTRS, p_data);
		hlp_avrcp_destroy_media_info(p_data);
	}
	break;
	default:
		break;
	}

	if (in->items_num == 0) {
		event_avrcp_indication(EV_AVRCP_ELEMENT_END, 0);
	}
}

void hlp_avrcp_group_conntion(UINT32 event, UINT8 *param)
{
	struct AVCTP_ConnectCompleteStru *in = (struct AVCTP_ConnectCompleteStru *)param;

	bse_log_api(TAG, "hlp_avrcp_group_conntion, event=0x%08X\r\n", event);
	switch (event) {
	case AVRCP_EV_CONNECT_COMPLETE:
		s_avrcp_user_data->capability = 0;
		if (in->channel == AVCTP_CONTROL_CHANNEL) {
			if (in->result == BT_SUCCESS) {
				memcpy(AVRCP_USER_DATA->remote_bd, in->bd_addr, BD_ADDR_LEN);
				hlp_avrcp_set_status(ST_APP_AVRCP_CONNECTED);

				/* Get Capabilities EVENTS_SUPPORTED*/

#ifdef CONFIG_BIP_CLIENT
                    /* Get rcfeature */
                         hlp_avrcp_get_rcfeature_req(s_avrcp_user_data->remote_bd);
#endif 
				hlp_avrcp_getcaps_req(AVRCP_USER_DATA->remote_bd, AVRCP_CAPABILITYID_EVENTS_SUPPORTED);

				event_avrcp_indication(EV_AVRCP_CONN_COMPLETED, AVRCP_USER_DATA->remote_bd);
			} else {
				if (hlp_avrcp_get_status() == ST_APP_AVRCP_CONNECTING) {
					hlp_avrcp_set_status(ST_APP_AVRCP_READY);

					event_avrcp_indication(EV_AVRCP_CONN_COMPLETED, NULL);
				}
			}
		} else if (in->channel == AVCTP_BROWSING_CHANNEL) {
			AVRCP_USER_DATA->bBrowsingChanel = (in->result == BT_SUCCESS);
		}
		break;
	case AVRCP_EV_DISCONNECT_COMPLETE:
		s_avrcp_user_data->capability = 0;
		old_cov_handle = -1;
		if (in->channel == AVCTP_CONTROL_CHANNEL) {
			hlp_avrcp_set_status(ST_APP_AVRCP_READY);
			event_avrcp_indication(EV_AVRCP_DISC_COMPLETED, AVRCP_USER_DATA->remote_bd);
			//memset(AVRCP_USER_DATA->remote_bd, 0, sizeof(AVRCP_USER_DATA->remote_bd));
		}
		break;
	default:
		break;
	}
}

void hlp_avrcp_group_command(UINT8 *bd, UINT32 event)
{
	AVRCP_GeneralReject_Rsp(bd, AVRCP_ERROR_INVALID_COMMAND, event);
}

UINT8 hlp_avrcp_get_capability(void)
{
	if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
		return s_avrcp_user_data->capability;
	}
	return -1;
}
void hlp_avrcp_fast_forward_timer()
{
    if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {
        //UART_Printf("hlp_avrcp_timer_passthrough_req, cmd=%d, state = %d\r\n",s_avrcp_fastforward_t.vendor_unique_id, AVRCP_STATE);
        AVRCP_PassThrough_Req(hlp_avrcp_get_connected_bd(), &s_avrcp_fastforward_t);
        s_avrcp_fast_forwar_timer = FsmAddTimer(NULL,500,NULL,NULL,"fastforward",(UINT32)hlp_avrcp_fast_forward_timer);
    } else {
        s_avrcp_fast_forwar_timer = NULL;
    }    
}

void hlp_avrcp_passthrough_req_ex(UINT8 *remote_bd, UINT8 op_id, UINT8 vendor_id, UINT8 state)
{
    struct AVRCP_PassThroughStru *in = (struct AVRCP_PassThroughStru *)NEW(sizeof(struct AVRCP_PassThroughStru));
    in->vendor_unique_id = vendor_id;
    in->state_flag = (state == 1) ? AVRCP_STATE_FLAG_BUTTON_PUSHED : AVRCP_STATE_FLAG_BUTTON_RELEASED;
    in->op_id = op_id;
    if (op_id == AVRCP_CMD_OPID_FAST_FORWARD || op_id == AVRCP_CMD_OPID_REWIND)
    {
        if (state) {
            memcpy(&s_avrcp_fastforward_t, in, sizeof(struct AVRCP_PassThroughStru));
            s_avrcp_fast_forwar_timer = FsmAddTimer(NULL,500,NULL,NULL,"fastforward",(UINT32)hlp_avrcp_fast_forward_timer);
        } else if (s_avrcp_fast_forwar_timer){
             FSM_DelTimer(&s_avrcp_fast_forwar_timer);
             s_avrcp_fast_forwar_timer = NULL;
             memset(&s_avrcp_fastforward_t, 0, sizeof(struct AVRCP_PassThroughStru));
        } 
    }
    AVRCP_PassThrough_Req(remote_bd, in);
    FREE(in);
}

static void hlp_avrcp_check_capability(UINT8 capability)
{
	switch (capability) {
	case AVRCP_EVENT_TRACK_CHANGED:
		s_avrcp_user_data->capability |= TRACK_CHANGED_SUPPORT;
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		s_avrcp_user_data->capability |= POS_CHANGED_SUPPORT;
		break;
	default:
		break;
	}
}

void hlp_avrcp_get_caps_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_GetCapabilitiesRspStru *in)
{
	UINT8 n = 0;
	UINT8 m = 0;

	bse_log_api(TAG, "hlp_avrcp_get_caps_cfm, rsp_type = %02X\r\n", rsp_type);
	bse_log_api(TAG, "[AVRCP] id=%02X, count=%d\r\n", in->id, in->count);

	if ((rsp_type == AVRCP_RSP_REJECTED) ||
	    ((in->id != AVRCP_CAPABILITYID_EVENTS_SUPPORTED) && (in->id != AVRCP_CAPABILITYID_COMPANY_ID))) {
		bse_log_error(TAG, "get caps fail!\r\n");

		event_avrcp_indication(EV_AVRCP_GET_CAPS_CMF, (void *)FALSE);
		return;
	}

	if (rsp_type != AVRCP_RSP_STABLE) {
		while ((n++) < NOTIF_COUNT) {
			hlp_avrcp_register_noification_req(bd, avrcp_notif_events[n]);
		}
		event_avrcp_indication(EV_AVRCP_GET_CAPS_CMF, (void *)TRUE);
	} else if (in->id == AVRCP_CAPABILITYID_EVENTS_SUPPORTED) {
		/* DBG */
		UINT8 dbg_str[100] = {0};
		strcpy(dbg_str, "[Cfm] Get Capabilities <EVENTS_SUPPORTED>");
		n = 0;
		while (n < in->count) {
			snprintf((dbg_str + strlen(dbg_str)), sizeof(dbg_str)-(strlen(dbg_str))," 0x%02X", in->capability[n]);
			hlp_avrcp_check_capability(in->capability[n]);
			n++;
		}
		bse_log_verbose(TAG, "%s\r\n", dbg_str);

		n = 0;
		while (n < in->count) {
			m = 0;
			while ((m < NOTIF_COUNT) && (in->capability[n] != avrcp_notif_events[m])) {
				m++;
			}
			if (m < NOTIF_COUNT) {
				hlp_avrcp_register_noification_req(bd, avrcp_notif_events[m]);
			}
			n++;
		}
		event_avrcp_indication(EV_AVRCP_GET_CAPS_CMF, (void *)TRUE);
	}
}

void hlp_avrcp_get_element_attr_cfm(UINT8 rsp_type, struct AVRCP_Num4IDValueStru *in)
{
	bse_log_api(TAG, "hlp_avrcp_get_element_attr_cfm, rsp_type: 0x%02X\r\n", rsp_type);
	if (rsp_type == AVRCP_RSP_STABLE) {
		struct AVRCPDataIndStru *p_data = NULL;
		p_data = hlp_avrcp_create_media_info(in->num, in->id_value);
		bse_log_api(TAG, "p_data->time:%s\r\n", p_data->time);
		event_avrcp_indication(EV_AVRCP_ELEMENT_ATTRS_OF_NOW_PLAYING, p_data);
		hlp_avrcp_destroy_media_info(p_data);
	} else {
		bse_log_error(TAG, "hlp_avrcp_get_element_attr_cfm, rsp_type: 0x%02X, error: 0x%02X\r\n"
		              , rsp_type, in->num);
	}
}

void hlp_avrcp_get_play_status_cfm(UINT8 rsp_type, struct AVRCP_GetPlayStatusRspStru *in)
{
	if (rsp_type == AVRCP_RSP_REJECTED) {
		return;
	}
	if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {
		if (in->play_status == AVRCP_PLAYSTATUS_PLAYING) {
			if (hlp_avrcp_get_status() != ST_APP_AVRCP_PLAYING) {
				hlp_avrcp_set_status(ST_APP_AVRCP_PLAYING);
				event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, (void *)TRUE);
			}
		} else if (in->play_status == AVRCP_PLAYSTATUS_PAUSED) {//in->play_status == AVRCP_PLAYSTATUS_STOPPED|| 
			if (hlp_avrcp_get_status() != ST_APP_AVRCP_PAUSED) {
				hlp_avrcp_set_status(ST_APP_AVRCP_PAUSED);
				event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, (void *)FALSE);
			}
		}else if (in->play_status == AVRCP_PLAYSTATUS_STOPPED) {
			if (hlp_avrcp_get_status() != ST_APP_AVRCP_STOPPED) {
				hlp_avrcp_set_status(ST_APP_AVRCP_STOPPED);
				event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, (void *)FALSE);
			}
		}
		
		if (AVRCP_STATE >= ST_APP_AVRCP_PLAYING) {
			if((in->song_position >= 0) && (in->song_position <= in->song_length)) {
				event_avrcp_indication(EV_AVRCP_NOW_PLAY_STATUS_IND, (struct AVRCPNowPlayStatusIndStru *)in);
			}
		}
	}
}

void hlp_avrcp_passthough_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_PassThroughStru *in)
{
	if (rsp_type == AVRCP_RSP_REJECTED || hlp_avrcp_get_status() < ST_APP_AVRCP_CONNECTED) {
		return;
	}

	if (in && (in->op_id < AVRCP_CMD_OPID_REWIND || in->op_id > AVRCP_CMD_OPID_FAST_FORWARD)) {
		//if (in->state_flag == AVRCP_STATE_FLAG_BUTTON_PUSHED) {
#if 0
		if(0) {
			struct AVRCP_PassThroughStru *passth = (struct AVRCP_PassThroughStru *)NEW(sizeof(struct AVRCP_PassThroughStru));
			passth->vendor_unique_id = in->vendor_unique_id;
			passth->state_flag = AVRCP_STATE_FLAG_BUTTON_RELEASED;
			passth->op_id = in->op_id;

			bse_log_api(TAG, "hlp_avrcp_passthough_cfm, rsp_type: 0x%02X, op_id=0x%02x\r\n", rsp_type, in->op_id);
			AVRCP_PassThrough_Req(bd, passth);
			FREE(passth);
		} else {
#ifndef CONFIG_DEFINE_BT_EVENT
			if (AVRCP_CMD_OPID_PAUSE == in->op_id) {
				bse_log_api(TAG, "hlp_avrcp_passthough_cfm, extra pause response.\r\n");
				event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, FALSE);
			} else if (AVRCP_CMD_OPID_PLAY == in->op_id) {
				bse_log_api(TAG, "hlp_avrcp_passthough_cfm, extra play response.\r\n");
				event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, TRUE);
			}
#else
			hlp_avrcp_get_element_attr_req(AVRCP_USER_DATA->remote_bd);
#endif		
			if (!bt_manager_get_timer_notify_status()) {
				hlp_avrcp_get_play_status(AVRCP_USER_DATA->remote_bd);
			}	
		}
#endif
	}
}

#if 0
UINT8 title[MAX_STR_LEN + 1] = {0};
UINT8 artist[MAX_STR_LEN + 1] = {0};
UINT8 album[MAX_STR_LEN + 1] = {0};
void hlp_avrcp_element_attrs_to_str(UINT8 *str, UINT8 num, struct AVRCP_4IDStringStru *in)
{
	UINT32 time = 0;
	UINT16 title_len = 0;
	UINT16 artist_len = 0;
	UINT16 album_len = 0;
	UINT16 len = 0;
	UINT8 n = 0;

	while (n < num) {
		len = (in + n)->len > MAX_STR_LEN ? MAX_STR_LEN : (in + n)->len;
		switch ((in + n)->attr_id) {
		case AVRCP_MA_PLAYING_TIME:
			if (((in + n)->len != 0) && ((in + n)->value != NULL)) {
				time = atoi((in + n)->value);
			}
			break;
		case AVRCP_MA_TITLEOF_MEDIA:
			title_len = len;
			memcpy(title, (in + n)->value, title_len);
			break;
		case AVRCP_MA_NAMEOF_ARTIST:
			artist_len = len;
			memcpy(artist, (in + n)->value, artist_len);
			break;
		case AVRCP_MA_NAMEOF_ALBUM:
			album_len = len;
			memcpy(album, (in + n)->value, album_len);
		default:
			break;
		}
		n++;
	}
	sprintf(str, "%08X%02X%s%02X%s%02X%s", time
	        , title_len, title, artist_len, artist, title_len, title);
}
#endif

void hlp_avrcp_register_noification_req(UINT8 *bd, UINT8 event_id)
{
	struct AVRCP_RegisterNotifReqStru *in;
	UINT16 len_in = sizeof(struct AVRCP_RegisterNotifReqStru);
	bse_log_api(TAG, "hlp_avrcp_register_noification_req,event_id:%d" , event_id);
	in = NEW(len_in);
	in->event_id = event_id;
	if (event_id == AVRCP_EVENT_PLAYBACK_POS_CHANGED) {
		in->playback_interval = 1;
	} else {
		in->playback_interval = 0;
	}

	AVRCP_AdvancedControl_Req(bd, AVRCP_PDUID_REGISTER_NOTIFICATION, in, len_in);
	FREE(in);
}

void hlp_avrcp_get_palyer_app_setting_value_req(UINT8 *remote_bd, UINT8* attribute_id,UINT8 num)
{
      UINT16 len = (num-1)*sizeof(UINT8) + sizeof(struct AVRCP_NumIDStru);
	struct AVRCP_NumIDStru* opstr = (struct AVRCP_NumIDStru *)NEW(len);
	
	opstr->num = num;
       memcpy(opstr->id, attribute_id, num);
	AVRCP_AdvancedControl_Req(remote_bd, AVRCP_PDUID_GET_CURRENTPLAYER_APPLICATION_SETTING_VALUE, (void*)opstr, len);
	FREE(opstr);
}

void hlp_avrcp_get_palyer_app_setting_value_cfm(UINT8 rsp_type, struct AVRCP_NumIDPairStru *in)
{
   	bse_log_api("%s, rsp_type: 0x%02X\r\n",__FUNCTION__,rsp_type);
    if (rsp_type == AVRCP_RSP_STABLE) 
    {
        //INT16  reg = 0;
        //UINT8 PASA_ID = 0;
        //UINT8 value_id = 0;
        UINT8 num = 0;
        for(num = 0;num < (in->num);num++)
        {   //for local reserve.Note:attr_id as value[subscript]. 
            //AVRCP_SDK_RES.value[in->id[num].attr_id - 1].attr_id = in->id[num].attr_id;
            //AVRCP_SDK_RES.value[in->id[num].attr_id - 1].value_id = in->id[num].value_id;

            //for debug
            //PASA_ID =  in->id[num].attr_id;
            //value_id =  in->id[num].value_id;
            //reg = (value_id << 8) |PASA_ID;
            //event_avrcp_indication(EV_AVRCP_PLAYER_APPLICATION_SETTING_CHANGED, (void*)reg);
        }
        event_avrcp_indication(EV_AVRCP_PLAYER_SETTTING_STATUS_IND, (void *)in);
    }
}

void hlp_avrcp_register_noification_cfm(UINT8 *bd, UINT8 rsp_type, struct AVRCP_RegisterNotifRspStru *in)
{
	bse_log_api(TAG, "hlp_avrcp_register_noification_cfm, event: 0x%02X, rsp_type: 0x%02X\r\n"
	            , in->event_id, rsp_type);

	if ((rsp_type != AVRCP_RSP_INTERIM) && (rsp_type != AVRCP_RSP_CHANGED)) {
		return;
	}

	switch (in->event_id) {
	case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
		if ((rsp_type == AVRCP_RSP_CHANGED) || (rsp_type == AVRCP_RSP_INTERIM)) {
			if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {

				btrc_play_status_t_ex play_status;
				play_status = (btrc_play_status_t_ex)*(in->param);
				bse_log_api(TAG, "play_status is 0x%x", play_status);

				/*if (*(in->param) == TRUE) {
					hlp_avrcp_set_status(ST_APP_AVRCP_PLAYING);
				} else {
					hlp_avrcp_set_status(ST_APP_AVRCP_PAUSED);
				}*/
				if(play_status == BTRC_PLAYSTATE_PLAYING){
					hlp_avrcp_set_status(ST_APP_AVRCP_PLAYING);
				}else if(play_status == BTRC_PLAYSTATE_PAUSED) {
					hlp_avrcp_set_status(ST_APP_AVRCP_PAUSED);
				}else if(play_status == BTRC_PLAYSTATE_STOPPED) {
					hlp_avrcp_set_status(ST_APP_AVRCP_STOPPED);
				}
			}
			event_avrcp_indication(EV_AVRCP_PLAYBACK_STATUS_CHANGED, (void *)(UINT32) (*(in->param)));
		}
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		if (rsp_type == AVRCP_RSP_CHANGED) {
			if (!bt_manager_get_timer_notify_status()) {
				hlp_avrcp_get_element_attr_req(bd, 1);
				hlp_avrcp_get_play_status(bd);
	          		FsmAddTimer(NULL, 1000, 0, NULL, NULL, (UINT32)hlp_avrcp_get_play_status_timer);
			}
		}
        	if(rsp_type == AVRCP_RSP_INTERIM){
            //hlp_avrcp_get_element_attr_req(bd);
          		  if (!bt_manager_get_timer_notify_status()) {
			    hlp_avrcp_get_play_status(bd);
	            //avrcp_get_p_timer = FsmAddTimer(NULL, 2500, 0, NULL, NULL, (UINT32)hlp_avrcp_get_play_status_timer);
            }
        }
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		if (rsp_type == AVRCP_RSP_CHANGED) {
			UINT32 pos = ((struct AVRCP_PlaybackPosChangedNotifStru *)in)->pos;
			event_avrcp_indication(EV_AVRCP_PLAYBACK_POS_CHANGED, (void *)pos);
		}
		break;
	case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
		if (rsp_type == AVRCP_RSP_CHANGED) {
			event_avrcp_indication(EV_AVRCP_NOW_PLAYING_CONTENT_CHANGED, 0);
		}
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		if (rsp_type == AVRCP_RSP_CHANGED) {
			AVRCP_USER_DATA->uid_counter = ((struct AVRCP_UIDsChangedNotifStru *)in)->uid_counter;
			event_avrcp_indication(EV_AVRCP_UPDATA_UIDS, (void *)(UINT32) (AVRCP_USER_DATA->uid_counter));
		}
		break;
	case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
		if (rsp_type == AVRCP_RSP_CHANGED) {
			UINT8 attribute_id[] = {
				AVRCP_CMD_PASA_ID_EQUALIZER,
				AVRCP_CMD_PASA_ID_REPEAT,
				AVRCP_CMD_PASA_ID_SHUFFLE,
				AVRCP_CMD_PASA_ID_SCAN
			};
			hlp_avrcp_get_palyer_app_setting_value_req(hlp_avrcp_get_connected_bd()\
																, attribute_id,sizeof(attribute_id));
		}
		break;
	default:
		break;
	}
}
void hlp_avrcp_group_response(UINT8 *bd, UINT32 event, UINT8 *param)
{
	UINT8 rsp_type = AVRCP_GET_4BIT_CR(event);

	bse_log_api(TAG, "hlp_avrcp_group_response, rsp_type=%x, pduid=0x%02X\r\n", rsp_type, AVRCP_GET_PDUID(event));
	switch (AVRCP_GET_PDUID(event)) {
	case AVRCP_PDUID_GET_CAPABILITIES:
		hlp_avrcp_get_caps_cfm(bd, rsp_type, param);
		break;
	case AVRCP_PDUID_GET_CURRENTPLAYER_APPLICATION_SETTING_VALUE:
		hlp_avrcp_get_palyer_app_setting_value_cfm(rsp_type, (struct AVRCP_NumIDPairStru *)param);
        break;
	case AVRCP_PDUID_GET_ELEMENT_ATTRIBUTES:
		hlp_avrcp_get_element_attr_cfm(rsp_type, param);
		break;
	case AVRCP_PDUID_GET_PLAY_STATUS:
		hlp_avrcp_get_play_status_cfm(rsp_type, param);
		break;
	case AVRCP_PDUID_GET_FOLDER_ITEMS:
		hlp_avrcp_get_folder_items_cfm(rsp_type, param);
		break;
	case AVRCP_PDUID_REGISTER_NOTIFICATION:
		hlp_avrcp_register_noification_cfm(bd, rsp_type, param);
		break;
	case AVRCP_PDUID_PLAY_ITEM:
		bse_log_info(TAG, "[CFM] Play Item, rsp_type: 0x%02X, status: 0x%02X"
		             , rsp_type, *param);
		break;
	case AVRCP_PDUID_PASSTHROUGH:
		hlp_avrcp_passthough_cfm(bd, rsp_type, param);
		break;
	default:
		break;
	}
}

void hlp_avrcp_cbk(UINT8 *bd, UINT32 event, UINT8 *param, UINT16 len)
{
	if (hlp_avrcp_get_status() == ST_APP_AVRCP_UNINIT) {
		bse_log_error(TAG, "hlp_avrcp_cbk, profile application not initialized!\r\n");
		return;
	}
	bse_log_api(TAG, "hlp_avrcp_cbk %x,%x\r\n",AVRCP_GET_CBK_GROUPING(event),AVRCP_GET_PDUID(event));

	switch (AVRCP_GET_CBK_GROUPING(event)) {
	case AVRCP_EV_GROUP_CONNECTION:
		hlp_avrcp_group_conntion(event, param);
		break;
	case AVRCP_EV_GROUP_COMMAND:
#ifdef CONFIG_ABS_VOL
		switch(AVRCP_GET_PDUID(event)) {
		case AVRCP_PDUID_GET_CAPABILITIES:
			hlp_avrcp_getcaps_ind(bd, event, *param);
			break;
		case AVRCP_PDUID_REGISTER_NOTIFICATION:/* param: struct AVRCP_RegisterNotifReqStru */
			hlp_avrcp_reg_notification_ind(bd, event, (struct AVRCP_RegisterNotifRspStru*)param);
			break;
		case AVRCP_PDUID_SET_ABSOLUTE_VOLUME:
			hlp_avrcp_set_abs_vol_ind(bd, event, *param);
			break;
		default:
			hlp_avrcp_group_command(bd, event);
			break;
		}
#else
		hlp_avrcp_group_command(bd, event);
#endif
		break;
	case AVRCP_EV_GROUP_RESPONSE:
		hlp_avrcp_group_response(bd, event, param);
		break;
	default:
		break;
	}

	AVRCP_FreeEventParam(event, param);/* FIXME, internal pointer? */
	bse_log_api(TAG, "hlp_avrcp_cbk %x,%x Exit\r\n",AVRCP_GET_CBK_GROUPING(event),AVRCP_GET_PDUID(event));
}


void hlp_avrcp_set_status(UINT8 status)
{
	UINT8 even_stat;
	int prev_f = -1;
	bse_log_verbose(TAG, "hlp_avrcp_set_status %d\n", status);
	AVRCP_STATE = status;
	even_stat = status;
	if(status > ST_APP_AVRCP_READY)
		even_stat = status - 1;
	if(get_g_bt_opened_status())
	{
		if(status == ST_APP_AVRCP_PLAYING)
			event_avrcp_indication(EV_AVRCP_PLAYER_SETTTING_STATUS_IND, NULL);
	
		if(status <= ST_APP_AVRCP_CONNECTED){
			event_avrcp_indication(EV_AVRCP_STAT, (void *)even_stat);
			prev_f = -1;
		} else {
			if(prev_f != status)
			{
				event_avrcp_indication(EV_AVRCP_PLAY_STAT, (void *)(status - ST_APP_AVRCP_STOPPED));
				prev_f = status;
			}
		}
	}
	

}

UINT8 hlp_avrcp_get_status(void)
{
	if (s_avrcp_user_data == NULL) {
		return ST_APP_AVRCP_UNINIT;
	} else {
		return AVRCP_STATE;
	}
}

void hlp_avrcp_quiry_status()
{
	avrcp_status_t status;
	memset(&status, 0, sizeof(avrcp_status_t));

	status.status = AVRCP_STATE;
	memcpy(status.bd, AVRCP_USER_DATA->remote_bd, BD_ADDR_LEN);
	event_avrcp_indication(EV_AVRCP_RETURN_STATUS, (void *) &status);
}

UINT8 *hlp_avrcp_get_connected_bd(void)
{
	UINT8 empty_bd[BD_ADDR_LEN];
	memset(empty_bd, 0, BD_ADDR_LEN);

	if (AVRCP_STATE >= ST_APP_AVRCP_CONNECTED) {
		return AVRCP_USER_DATA->remote_bd;
	}

	return empty_bd;
}

UINT16 hlp_avrcp_get_uid_counter(void)
{
	return AVRCP_USER_DATA->uid_counter;
}

#ifdef CONFIG_ABS_VOL
UINT8 abs_volume = -1;

static void hlp_avrcp_getcaps_ind(UINT8 *bd, UINT32 event, UINT8 cap_id)
{
	//UNUSED(bd);
	//UNUSED(event);
	bse_log_verbose(TAG, "hlp_avrcp_getcaps_ind : cap_id is 0x%x", cap_id);
	UINT8 len;
	struct AVRCP_GetCapabilitiesRspStru *rsp = NULL;

	switch (cap_id) {
	case AVRCP_CAPABILITYID_COMPANY_ID:
		len = (UINT8)(sizeof(struct AVRCP_GetCapabilitiesRspStru) - 1 + 3);
		rsp = (struct AVRCP_GetCapabilitiesRspStru *)NEW(len);
		rsp->count = 1;
		/*Only set Bluetooth SIG Company ID*/
		rsp->capability[0] = 0x00;
		rsp->capability[1] = 0x19;
		rsp->capability[2] = 0x58;
		break;
	case AVRCP_CAPABILITYID_EVENTS_SUPPORTED:
		len = (UINT8)(sizeof(struct AVRCP_GetCapabilitiesRspStru) - 1 + 3);
		rsp = (struct AVRCP_GetCapabilitiesRspStru *)NEW(len);
		rsp->count = 3;
		rsp->capability[0] = AVRCP_EVENT_PLAYBACK_STATUS_CHANGED;
		rsp->capability[1] = AVRCP_EVENT_SYSTEM_STATUS_CHANGED;
		rsp->capability[2] = AVRCP_EVENT_VOLUME_CHANGED;
		break;
	default:
	{
		UINT8 temp = AVRCP_ERROR_INVALID_PARAMETER;
		AVRCP_GeneralReject_Rsp(bd, temp, event);
		break;
	}
	}
	if(rsp) {
		rsp->id = cap_id;
		AVRCP_AdvancedControl_Rsp(bd, event, rsp, len);
		FREE(rsp);
	}
}

static void hlp_avrcp_reg_notification_ind(UINT8 *bd, UINT32 event, struct AVRCP_RegisterNotifRspStru *param)
{
	//if (!auto_global_obj_is_valid(PROFILE_ID_AVRCP_CONTROLLER) || (param == NULL)) {
		//return;
	//}

	bse_log_verbose(TAG, "hlp_avrcp_reg_notification_ind :%x", param->event_id);
	struct AVRCP_RegisterNotifRspStru rsp;
	UINT8 notif = 1;
	bt_bdaddr_t address;
	memcpy(address.address, bd, BD_ADDR_LEN);
	rsp.param[0] = 0;

	switch (param->event_id) {
	case AVRCP_EVENT_VOLUME_CHANGED:
		bse_log_verbose(TAG, "----> Register Volume Change Notification.");
		notif = 0;
#ifndef CONFIG_OS_ANDROID
		notif = 1;
#endif
		s_abs_lable = AVRCP_GET_TL(event);
		bse_log_verbose(TAG, "hlp_avrcp_reg_notification_ind :%u", s_abs_lable);
		//AVRCP_CONTROLLER_CBK(handle_register_notification_abs_vol_cb, address, AVRCP_GET_TL(event));
		break;
	case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
		AVRCP_SET_RSPTYPE(event, AVRCP_RSP_INTERIM);
		break;
	case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
		AVRCP_SET_RSPTYPE(event, AVRCP_RSP_INTERIM);
		break;
	default:
		AVRCP_SET_RSPTYPE(event, AVRCP_RSP_NOT_IMPLEMENTED);
		break;
	}

	rsp.event_id = param->event_id;
	if (notif) {
		AVRCP_RegisterNotif_Rsp(event, &rsp, sizeof(struct AVRCP_RegisterNotifRspStru));
	}
}

static void hlp_avrcp_set_abs_vol_ind(UINT8 *bd, UINT32 event, UINT8 volume)
{
	bt_bdaddr_t address;

	//if (!auto_global_obj_is_valid(PROFILE_ID_AVRCP_CONTROLLER)) {
		//return;
	//}

	memcpy(address.address, bd, BD_ADDR_LEN);
	bse_log_verbose(TAG, "hlp_avrcp_set_abs_vol_ind volume is %d", volume);
	if (volume != abs_volume) {
		if (volume > AVRCP_ABSOLUTE_VOLUME_MAX) {
			volume = AVRCP_ABSOLUTE_VOLUME_MAX;
		}
		abs_volume = volume;
		if (hlp_avrcp_get_status() >= ST_APP_AVRCP_CONNECTED) {
#ifndef CONFIG_OS_ANDROID
			AVRCP_AdvancedControl_Rsp(address.address, event, &volume, 1);
#endif
			//AVRCP_CONTROLLER_CBK(handle_set_abs_volume_cb, address, abs_volume, AVRCP_GET_TL(event));
			event_avrcp_indication(EV_AVRCP_ABS_VOL_IND, (void *)volume);
			return;
		}

	}

	/* Make sure send the rsp*/
	AVRCP_AdvancedControl_Rsp(address.address, event, &volume, 1);
}
#endif

void hlp_avrcp_send_register_abs_vol_rsp(UINT8 *bd, UINT8 rsp_type, int abs_vol)
{
	printf("%s\n", __func__);
	struct AVRCP_RegisterNotifRspStru rsp;
	UINT32 event = AVRCP_PDUID_REGISTER_NOTIFICATION;
	AVRCP_SET_TL(event, s_abs_lable);
	bse_log_api(TAG, "%s abs volum = %d\r\n", __func__, abs_vol);
	AVRCP_SET_RSPTYPE(event, (rsp_type == 1) ? AVRCP_RSP_INTERIM : AVRCP_RSP_CHANGED);
	rsp.event_id = AVRCP_EVENT_VOLUME_CHANGED;
	rsp.param[0] = abs_vol;
	AVRCP_RegisterNotif_Rsp(event, &rsp, sizeof(struct AVRCP_RegisterNotifRspStru));
}
