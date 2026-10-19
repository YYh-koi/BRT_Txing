/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_pbap.h
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/
#include "btdatatype.h"
#include "brt_trace.h"
#include "auto_pbap.h"
#include "internal_pbap.h"
#include "pbap_ui.h"
#include "cmd_composition.h"
#include <string.h>
#include <time.h>
#include <time.h>
#include <pthread.h>

#include "vcf_parse.h"
#include "auto_hf.h"
#include "auto_gap.h"

#define TAG "bse_pbap"

#undef PBAP_USER_DATA_PT

struct PbapBuffStru{
	UINT8				data[8*1024];		//MAX_BUFFER_SIZE		
	UINT16				length;				
	tPBAP_Completeness	is_final;
};

static struct PBAPUserDataStru *s_pbap_user_data = NULL;
#ifdef CONFIG_PBAP_PHOTO
static struct PBAPUserDataStruP *s_pbap_user_data_p = NULL;
#endif
//#ifdef PBAP_USER_DATA_PT
enum {
	PTHREAD_PBAP_READY = 1,
	PTHREAD_PBAP_GETTING,
	PTHREAD_PBAP_DONE,
};

static tPBAP_PullPhoneBookCompleteStru s_pb_complete;
static int pbap_continue_f = 1;
static int p_pb_state = 0;
static int p_pb_complete = 1;
//#endif
static int s_get_pbap_mode = 0; //0:vCardListReq 1:PhoneBookReq
UCHAR save_call_number[32] = {0};
vcard_xml_entry_t pbap_cbk_entry = {0};

static void hlp_pbap_authentication_ind(void *handle, tApp_GetPBContextStru *context);
static void hlp_pbap_connect_complete(void *handle, void *param, tApp_GetPBContextStru *context);
static void hlp_pbap_phonebook_data_ind(void *param);
static void hlp_pbap_pull_phonebook_complete(void *param, int c_pb_type);
static void hlp_pbap_disconnect_complete(tApp_GetPBContextStru *context, void *param);
static void hlp_get_phonebook_cfm_cbk(void *handle, tApp_GetPBContextStru *context, UINT8 event, void *param);
void bt_pbap_cache_clean();

void event_pbap_indication(UINT8 event, void *param)
{
	if (s_pbap_user_data->event_cbk != NULL) {
		s_pbap_user_data->event_cbk((PROFILE_ID_PBAP << 8) | event, param);
	}
}

void hlp_pbap_init(module_event_cbk_t event_cbk)
{
	if (s_pbap_user_data == NULL) {
		s_pbap_user_data = (struct PBAPUserDataStru *)NEW(sizeof(struct PBAPUserDataStru));
	}

	memset(s_pbap_user_data, 0, sizeof(struct PBAPUserDataStru));
	// FIXME
	s_pbap_user_data->storage = PT_PBAP_IDLE;
	s_pbap_user_data->flag_auto_connect = FALSE;
	s_pbap_user_data->event_cbk = event_cbk;
	hlp_pbap_set_status(ST_PBAP_INIT);

#ifdef CONFIG_PBAP_PHOTO
	if(s_pbap_user_data_p == NULL) {
		s_pbap_user_data_p = (struct PBAPUserDataStruP *)NEW(sizeof(struct PBAPUserDataStruP));
	}
	memset(s_pbap_user_data_p, 0, sizeof(struct PBAPUserDataStruP));
#endif
	app_vcfparser_create();
}

void hlp_pbap_server_start(void)
{
	hlp_pbap_set_status(ST_PBAP_READY);
}

void hlp_pbap_server_stop(void)
{
	hlp_pbap_set_status(ST_PBAP_INIT);
}

void hlp_pbap_done(void)
{
	if (s_pbap_user_data != NULL) {
		if (s_pbap_user_data->p_vcf_parser != NULL) {
			app_vcfparser_free(PBAP_VCF_PARSER_POINTRT, 1);
		}
		FREE(s_pbap_user_data);
		s_pbap_user_data = NULL;
	}
#ifdef CONFIG_PBAP_PHOTO
	if (s_pbap_user_data_p != NULL) {
		if (s_pbap_user_data_p->p_photo_parser != NULL) {
			app_photo_parser_free(PBAP_PHOTO_PARSER_POINTRT);
		}
		FREE(s_pbap_user_data_p);
		s_pbap_user_data_p = NULL;
	}
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get PhoneBook - including connection, Pull, disconnecting.
	bd:				Device address of PSE.
	pse_userid:		User name related to the following password. NULL if not specified.
	pse_password:	Password to access the PSE if required. NULL if not specified.
------------------------------------------------------------------------------------*/
void hlp_pbap_connect(UINT8 *bd, UCHAR *pse_userid, UCHAR *pse_password)
{
	tApp_GetPBContextStru *context;

	if (bd == NULL) {
		return;
	}

	hlp_pbap_set_status(ST_PBAP_CONNECTING);
	context = NEW(sizeof(tApp_GetPBContextStru));
	memset(context, 0, sizeof(tApp_GetPBContextStru));

	if (pse_userid != NULL) {
		context->pse_userid = NEW(strlen(pse_userid) + 1);
		strcpy((char *)context->pse_userid, (const char *)pse_userid);
	}
	if (pse_password != NULL) {
		context->pse_password = NEW(strlen(pse_password) + 1);
		strcpy((char *)context->pse_password, (const char *)pse_password);
	}

	bse_log_api(TAG,  "****PBAP_ClientStartReq:%02x:%02x:%02x:%02x:%02x:%02x.", bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
	s_pbap_user_data->supported_repos = 0xFF;
	PBAP_ClientStartReq(context, bd, 0, NULL, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
}

void hlp_pbap_abort_phonebook()
{
	if((hlp_pbap_get_status() >= ST_PBAP_GETTING) && (hlp_pbap_get_handle() != NULL)) {
#ifdef PBAP_USER_DATA_PT
		pbap_continue_f = 0;
#endif
		PBAP_AbortPullObjectReq(hlp_pbap_get_handle());
	}
}

int hlp_pbap_get_storage()
{
	return PBAP_USER_DATA->storage;
}

void hlp_pbap_set_phonebook_param(UINT8 store_type, UINT16 offset, UINT16 count)
{
	PBAP_USER_DATA->storage = store_type;
	PBAP_USER_DATA->offset = offset;
	PBAP_USER_DATA->max_list_count = count;
    s_get_pbap_mode = 1;
}

void hlp_pbap_set_download_mode(int mode, UCHAR *number)
{
    s_get_pbap_mode = mode;
    strcpy(save_call_number, number);
    PBAP_USER_DATA->storage = PT_PBAP_VCARD;
}

void hlp_pbap_get_vcardListing(UCHAR *value_num)
{
    s_get_pbap_mode = 0;
    WCHAR path_tel[11] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'p', 'b',0};
	tPBAP_PullvCardListReqStru *vcardlist_req = NEW(sizeof(tPBAP_PullvCardListReqStru));
	memset(vcardlist_req,0,sizeof(tPBAP_PullvCardListReqStru));

	hlp_pbap_set_status(ST_PBAP_GETTING);
	strcpy(save_call_number, value_num);

	vcardlist_req->flag = PBAP_FLAG_MAXLISTCOUNT|PBAP_FLAG_SEARCHVALUE| PBAP_FLAG_SEARCHATTRIBUTE;
	vcardlist_req->order = PBAP_ORDER_INDEXED;
	vcardlist_req->search_attribute = PBAP_SCHATTR_NUMBER;
	vcardlist_req->search_value = value_num;
	vcardlist_req->max_list_count = 0x1;
	vcardlist_req->list_start_offset =0x0;
	PBAP_PullvCardListReq(hlp_pbap_get_handle(), NULL, path_tel, vcardlist_req,(tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
	FREE(vcardlist_req);
}

void hlp_pbap_get_phonebook(void)
{
	if(app_gap_is_discovering() == TRUE) {
			app_gap_cancel_discovery();
	}

	if(hlp_hf_get_status() < ST_HF_CONNECTED) {
		hlp_pbap_disconnect();
		return ;
	}

	p_pb_complete = 0;
#ifdef PBAP_USER_DATA_PT
    hlp_pb_buff_init();
	pbap_continue_f = 1;
#endif
    s_get_pbap_mode = 1;
	WCHAR path_tel[15] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'p', 'b', '.', 'v', 'c', 'f', 0};
	WCHAR path_sim[20] = {'S', 'I', 'M', '1', '/', 't', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'p', 'b', '.', 'v', 'c', 'f', 0};
	WCHAR path_och[16] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'o', 'c', 'h', '.', 'v', 'c', 'f', 0};
	WCHAR path_mch[16] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'm', 'c', 'h', '.', 'v', 'c', 'f', 0};
	WCHAR path_ich[16] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'i', 'c', 'h', '.', 'v', 'c', 'f', 0};
	WCHAR path_cch[16] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'c', 'c', 'h', '.', 'v', 'c', 'f', 0};
	tPBAP_PullPhoneBookReqStru *req = NEW(sizeof(tPBAP_PullPhoneBookReqStru));

	hlp_pbap_set_status(ST_PBAP_GETTING);

	memset(req,0,sizeof(tPBAP_PullPhoneBookReqStru));
	req->flag = PBAP_FLAG_MAXLISTCOUNT | PBAP_FLAG_LISTSTARTOFFSET | PBAP_FLAG_FILTER | PBAP_FLAG_PHONEBOOKSIZE;
	req->list_start_offset = PBAP_USER_DATA->offset;
	req->max_list_count = PBAP_USER_DATA->max_list_count;

	PBAP_FilterComposer(req->filter, PBAP_FILTER_N);
	PBAP_FilterComposer(req->filter, PBAP_FILTER_FN);
	PBAP_FilterComposer(req->filter, PBAP_FILTER_TEL);
#ifdef CONFIG_PBAP_PHOTO
	if((PBAP_USER_DATA->pbap_info_mask & PBAP_INFO_MASK_PHOTO)) {
		bse_log_info(TAG, "getting photo item------------------------------------");
		PBAP_FilterComposer(req->filter, PBAP_FILTER_PHOTO);
	}
#endif
	if((PBAP_USER_DATA->pbap_info_mask & PBAP_INFO_MASK_EMAIL))
		PBAP_FilterComposer(req->filter, PBAP_FILTER_EMAIL);

	if((PBAP_USER_DATA->storage >= PT_PBAP_OCH) && (PBAP_USER_DATA->storage <= PT_PBAP_ICH)
	   || PBAP_USER_DATA->storage == PT_PBAP_CCH) {
		PBAP_FilterComposer(req->filter, PBAP_FILTER_X_IRMC_CALL_DATETIME);
	}

	PBAP_VCF_PARSER_POINTRT = app_vcfparser_alloc(PBAP_USER_DATA->storage, "PBAP");
	PBAP_VCF_PARSER_POINTRT->item_callback = event_pbap_indication;
#ifdef CONFIG_PBAP_PHOTO
	PBAP_PHOTO_PARSER_POINTRT = app_photo_parser_alloc(PBAP_PHOTO_USER_DATA->storage, "PBAP");
	PBAP_PHOTO_PARSER_POINTRT->item_callback = event_pbap_indication;
#endif
	switch(PBAP_USER_DATA->storage) {
	case PT_PBAP_SIM:
	case PT_PBAP_SIM_IDX:
		bse_log_info(TAG,  "****PBAP_PullPhoneBookReq SIM");
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_sim, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
		break;
	case PT_PBAP_TEL:
	case PT_PBAP_TEL_IDX:
		bse_log_info(TAG,  "****PBAP_PullPhoneBookReq TEL");
#if 1
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_tel, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);

#else
        //why failed? need setphonebook? vcard_name err?
	tPBAP_PullvCardReqStru *vcard_req = NEW(sizeof(tPBAP_PullvCardReqStru));
	memset(vcard_req,0,sizeof(tPBAP_PullvCardReqStru));

	vcard_req->flag = PBAP_FLAG_FORMAT|PBAP_FLAG_MAXLISTCOUNT | PBAP_FLAG_LISTSTARTOFFSET | PBAP_FLAG_FILTER | PBAP_FLAG_PHONEBOOKSIZE;
	PBAP_FilterComposer(vcard_req->filter, PBAP_FILTER_N);
	PBAP_FilterComposer(vcard_req->filter, PBAP_FILTER_FN);
	PBAP_FilterComposer(vcard_req->filter, PBAP_FILTER_TEL);
	vcard_req->format = PBAP_FORMAT_VCARD21;
	WCHAR vcard_name[6] = {'4', '.', 'v', 'c', 'f', 0};
    //WCHAR vcard_name[13] = {'t', 'e', 'l', 'e', 'c', 'o', 'm', '/', 'p', 'b', '/', '4', 0};
    PBAP_PullvCardReq(hlp_pbap_get_handle(), NULL, vcard_name, vcard_req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
#endif

		break;
	case PT_PBAP_OCH:
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_och, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
		break;
	case PT_PBAP_MCH:
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_mch, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
		break;
	case PT_PBAP_ICH:
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_ich, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
		break;
	case PT_PBAP_CCH:
		PBAP_PullPhoneBookReq(hlp_pbap_get_handle(), NULL, path_cch, req, (tPBAP_EventCbk *)hlp_get_phonebook_cfm_cbk);
		break;
	default:
		break;
	}

	FREE(req);
}

static void hlp_pbap_authentication_ind(void *handle, tApp_GetPBContextStru *context)
{
	if (context == NULL) {
		return;
	}

	if (context->auth_failed == 0) {
		context->auth_failed = 1;
		PBAP_AuthenticationRsp(handle, context->pse_userid, context->pse_password);
	} else {
		/* Avoid trying the same wrong password repeatedly. Cancel the connection directly. */
		PBAP_AuthenticationRsp(handle, context->pse_userid, NULL);
	}
}

static void hlp_pbap_connect_complete(void *handle, void *param, tApp_GetPBContextStru *context)
{
	tPBAP_ConnectCompleteStru *conn_cmp = (tPBAP_ConnectCompleteStru *)param;

	if (conn_cmp != NULL) {
		if (conn_cmp->result == ER_PBAP_SUCCESS) {
			bse_log_info(TAG,  "PBAP_EVT_CONNECT_COMPLETE success handle = %x", handle);
			PBAP_USER_DATA->pb_handle = handle;
			hlp_pbap_set_status(ST_PBAP_CONNECTED);
			event_pbap_indication(EV_PBAP_CONNECT_SUCCESS, conn_cmp->bd);
#if 1
			if(hlp_pbap_get_flag_auto_connect()) {
				if (PBAP_USER_DATA->storage != PT_PBAP_IDLE) {
                		if(s_get_pbap_mode == 1) {
				    hlp_pbap_get_phonebook();
				} else {
				    if(strlen(save_call_number) >= 0) {
                        			hlp_pbap_get_vcardListing(save_call_number);
                    		}
               		 }
				}
			}
			
#endif
		} else {
			if (context != NULL) {
				if (context->pse_userid != NULL) {
					FREE(context->pse_userid);
				}

				if (context->pse_password != NULL) {
					FREE(context->pse_password);
				}
				FREE(context);
				context = NULL;
			}

			bse_log_info(TAG,  "PBAP_EVT_CONNECT_COMPLETE fail ret=0x%x.", conn_cmp->result);
			hlp_pbap_set_status(ST_PBAP_READY);
			if(hlp_pbap_get_flag_auto_connect()) {
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_FAIL, (void *)(UINT32) (conn_cmp->result));
			}
			hlp_pbap_set_flag_auto_connect(FALSE);
			event_pbap_indication(EV_PBAP_CONNECT_FAIL, conn_cmp->bd);
		}
	}
}

static void hlp_pbap_vcardlist_data_ind(void *param, UCHAR *vcard_number)
{
    tPBAP_ObjectDataStru *obj_data = (tPBAP_ObjectDataStru *)param;
    if(PBAP_STATE == ST_PBAP_GETTING) {
        if (param != NULL) {
            UINT16 i=0;
            for(i = 0; i < obj_data->length; i++) {
                printf("%02X ", *(obj_data->data + i));
            }
            printf("\n");
            memset(pbap_cbk_entry.name, 0x0, sizeof(pbap_cbk_entry.name));
            memset(pbap_cbk_entry.handle, 0x0, sizeof(pbap_cbk_entry.handle));
            memset(pbap_cbk_entry.number, 0x0, sizeof(pbap_cbk_entry.number));

            memcpy(pbap_cbk_entry.number, vcard_number, strlen(vcard_number));
            app_process_obex_xml_vcard_listing(obj_data->data, obj_data->length, &pbap_cbk_entry);
            bse_log_info(TAG, "name;%s, handle:%s, number:%s", pbap_cbk_entry.name, pbap_cbk_entry.handle, pbap_cbk_entry.number);

            char dpb_buf_ex[MAX_VCF_ITEM_LEN] = {0};
            snprintf(dpb_buf_ex, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d",5, 0xff, pbap_cbk_entry.name, 0xff, pbap_cbk_entry.number, 0xff, 1);
            event_pbap_indication(EV_PBAP_CURR_CALL_PHONEBOOK_IND, (void *)dpb_buf_ex);
        }
    }
}

UINT8 hlp_pabp_check_number_vcard(UCHAR *calling_number)
{
    if(strlen(calling_number) <= 0) {
        bse_log_info(TAG, "number is  null return");
        return 0;
    }
    if(memcmp(calling_number, pbap_cbk_entry.number,strlen(calling_number)) != 0) {
        return 1;
    }
    if(strlen(pbap_cbk_entry.name) <= 0) {
        return 1;
    }
    return 1;
}

static void hlp_pbap_phonebook_data_ind(void *param)
{
#ifdef PBAP_USER_DATA_PT
	struct PbapBuffStru *obj_data = (struct PbapBuffStru *)param;
#else
	tPBAP_ObjectDataStru *obj_data = (tPBAP_ObjectDataStru *)param;
#endif
	//bse_log_info(TAG,"obj_data->length = %d\n", obj_data->length);
	if(PBAP_STATE == ST_PBAP_GETTING) {
		if (param != NULL) {
			UINT16 i=0;
#ifdef PBAP_USER_DATA_PT
				for(i = 0; i < obj_data->length && pbap_continue_f; i++) {
#else
				for(i = 0; i < obj_data->length; i++) {// && pbap_continue_f
#endif
				app_vcfparser_data_ind(PBAP_VCF_PARSER_POINTRT, obj_data->data + i, obj_data->length, i);
#ifdef CONFIG_PBAP_PHOTO
				app_photoparser_data_ind(PBAP_PHOTO_PARSER_POINTRT, obj_data->data + i);
#endif
			}
			app_vcfparser_stor(PBAP_VCF_PARSER_POINTRT);
#ifdef CONFIG_PBAP_PHOTO
			app_photo_parser_stor(PBAP_PHOTO_PARSER_POINTRT);
#endif
		}
	}
}

static int s_pbap_callog_offset = 0;
static void hlp_pbap_pull_phonebook_complete(void *param, int c_pb_type)
{
	tPBAP_PullPhoneBookCompleteStru *get_pb_cmp = param;

	app_vcfparser_free(PBAP_VCF_PARSER_POINTRT, 0);
	PBAP_VCF_PARSER_POINTRT = NULL;
#ifdef CONFIG_PBAP_PHOTO
	app_photo_parser_free(PBAP_PHOTO_PARSER_POINTRT);
	PBAP_PHOTO_PARSER_POINTRT = NULL;
#endif
	if(hlp_pbap_get_status() >= ST_PBAP_CONNECTED) {
		hlp_pbap_set_status(ST_PBAP_CONNECTED);
	}

	if (get_pb_cmp == NULL) {
		hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
		return;
	}

	bse_log_info(TAG,  "PBAP_EVT_PULL_PHONEBOOK_COMPLETE result:%d.", get_pb_cmp->result);
	
	if (get_pb_cmp->result == ER_PBAP_SUCCESS) {/* Something wrong or getting complete */
		//bse_log_info(TAG,"PBAP_USER_DATA->max_list_count = %d, PBAP_USER_DATA->storage = %d\n", PBAP_USER_DATA->max_list_count, PBAP_USER_DATA->storage);
		if (PBAP_USER_DATA->max_list_count == 0x0000) {
			UINT32 event = (PBAP_USER_DATA->storage << 16) | (get_pb_cmp->pb_size & 0xFFFF);
			
			event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS, (void *)event);
			hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
		}
#ifdef CONFIG_CARLIFE_CERTIFICATE
		else if (PBAP_USER_DATA->storage == PT_PBAP_OCH) {
		      do_get_incoming_call(NULL, 0);
		}
		else if (PBAP_USER_DATA->storage == PT_PBAP_ICH) {
		      do_get_missed_call(NULL, 0);
		}
		else if (PBAP_USER_DATA->storage == PT_PBAP_CCH) {
			if (s_pbap_callog_offset < PABP_CALLOG_SIZE) {
				s_pbap_callog_offset += PABP_CALLOG_SIZE;
				do_get_history_call_ex(s_pbap_callog_offset, PABP_CALLOG_SIZE);
			} else {
				s_pbap_callog_offset = 0;
				hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SUCCESS, NULL);
			}
		}
#endif
		else {
			hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
			bse_log_error(TAG, "hlp_pbap_pull_phonebook_complete c_pb_type = %d\r\n", c_pb_type);
			if(c_pb_type)
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SUCCESS, "1");
			else
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SUCCESS, "2");
		}
	} else {
		bse_log_info(TAG,  "PBAP_EVT_PULL_PHONEBOOK_COMPLETE get_pb_cmp->result=%02x c_pb_type = %d.", get_pb_cmp->result, c_pb_type);
		//if(get_pb_cmp->result == 0xc4 || get_pb_cmp->result == 0xc8)
		//{
			if(c_pb_type)
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SUCCESS, "1");
			else
				event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SUCCESS, "2");
		//}

		if (PBAP_USER_DATA->max_list_count == 0x0000) {
			UINT32 event = (PBAP_USER_DATA->storage << 16);
			event_pbap_indication(EV_PBAP_GET_PHONEBOOK_SIZE_SUCCESS, (void *)event);
		} else {
			event_pbap_indication(EV_PBAP_GET_PHONEBOOK_FAIL, (void *)(UINT32) (get_pb_cmp->result));
		}
		hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
	}
	//hlp_pbap_set_phonebook_param(PT_PBAP_IDLE, 0, 0);
	bse_log_info(TAG, "hlp_pbap_pull_phonebook_complete  end\n");
}

static void hlp_pbap_disconnect_complete(tApp_GetPBContextStru *context, void *param)
{
	tPBAP_ConnectCompleteStru *conn_param = (tPBAP_ConnectCompleteStru *) param;

	if (context != NULL) {
		if (context->pse_userid != NULL) {
			FREE(context->pse_userid);
		}

		if (context->pse_password != NULL) {
			FREE(context->pse_password);
		}
		FREE(context);
		context = NULL;
	}

	PBAP_USER_DATA->pb_handle = NULL;
	hlp_pbap_set_status(ST_PBAP_READY);
	hlp_pbap_set_flag_auto_connect(FALSE);
	event_pbap_indication(EV_PBAP_DISCONNECTED, (conn_param == NULL ? NULL : conn_param->bd));
}

#undef CONFIG_USE_PBAP_DATA_CACHE
#ifdef CONFIG_USE_PBAP_DATA_CACHE
static uint64_t pbap_time = 0;
static uint64_t system_time(void)
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000000000LL + t.tv_nsec;
}

void hlp_pbap_start_pthread();
//static uint8_t bt_pbap_cache_add_data(struct PbapBuffStru *pbap_data, int len);
static uint8_t bt_pbap_cache_add_data(tPBAP_ObjectDataStru *pbap_data, int len);
#endif

void hlp_pbap_error_phonbook_complete()
{
	int pb_event_type = PBAP_USER_DATA->storage < 2 ? 1 : 0;
	bse_log_info(TAG,"hlp_pbap_error_phonbook_complete pb_event_type = %d pbap_continue_f = %d\n", pb_event_type, pbap_continue_f);
	if(!pbap_continue_f){
		//bse_log_info(TAG,"PBAP_EVT_PULL_PHONEBOOK_COMPLETE pb_event_type = %d \n", pb_event_type);
		hlp_pbap_pull_phonebook_complete((void *)&s_pb_complete, pb_event_type);
	}
}

static void hlp_get_phonebook_cfm_cbk(void *handle, tApp_GetPBContextStru *context, UINT8 event, void *param)
{
	if (hlp_pbap_get_status() == ST_PBAP_UNINIT) {
		bse_log_error(TAG, "hlp_get_phonebook_cfm_cbk, profile application not initialized!\r\n");
		return;
	}

	bse_log_info(TAG,  "****hlp_get_phonebook_cfm_cbk, event=%x.", event);

	switch (event) {
	case PBAP_EVT_CLIENT_START_CFM:
		// handle is pce_instance NOT a connection handle.
		// PBAP_USER_DATA->pb_handle = handle;
		break;
	case PBAP_EVT_CLIENT_SERVICE_INFO:
		s_pbap_user_data->supported_repos = (UINT8)(UINT32) param;
		event_pbap_indication(EV_PBAP_SUPPORTED_REPOS, (void *)s_pbap_user_data->supported_repos);
		break;
	case PBAP_EVT_AUTHENTICATION_IND:
		hlp_pbap_authentication_ind(handle, context);
		break;
	case PBAP_EVT_CONNECT_COMPLETE:
#ifdef PBAP_USER_DATA_PT
		hlp_pbap_start_pthread();
#endif
		hlp_pbap_connect_complete(handle, param, context);
		break;
	case PBAP_EVT_OBJECT_DATA_IND:
		{ /* TODO: Save content in real_par.obj_data->data */
		//pbap_time = system_time();
		//hlp_pbap_phonebook_data_ind(param);
		//uint64_t total_elapsed = (system_time() - pbap_time) / 1000UL;
		//bse_log_info(TAG,"[wl_debug] pbap data sleep time:(%llu us)\n", total_elapsed);
		//bse_log_info(TAG,"write sleep time:(%llu us)", total_duration - total_elapsed);
#ifdef PBAP_USER_DATA_PT
		tPBAP_ObjectDataStru *s_obj_data = (tPBAP_ObjectDataStru *)param;

		bt_pbap_cache_add_data(s_obj_data, s_obj_data->length);
#else
        if(s_get_pbap_mode == 1) {
		    hlp_pbap_phonebook_data_ind(param);
		    bse_log_info(TAG,"PBAP_EVT_OBJECT_DATA_IND -------------------\n");
        } else {
            bse_log_info(TAG,"PBAP_EVT_OBJECT_DATA_IND vcardlist -------------------\n");
            hlp_pbap_vcardlist_data_ind(param, save_call_number);
        }
#endif
		}
		break;
	case PBAP_EVT_PULL_PHONEBOOK_COMPLETE:
	case PBAP_EVT_PULL_VCARDLIST_COMPLETE:
	{
		p_pb_complete = 1;
#ifdef PBAP_USER_DATA_PT
		tPBAP_PullPhoneBookCompleteStru * p = (tPBAP_PullPhoneBookCompleteStru *)param;
		bse_log_info(TAG,"PBAP_EVT_PULL_PHONEBOOK_COMPLETE p_pb_state = %d p->result=%02x\n", p_pb_state, p->result);
		memcpy(&s_pb_complete, p, sizeof(tPBAP_PullPhoneBookCompleteStru));
		if(p_pb_state != PTHREAD_PBAP_GETTING || p->result != 0)//p->result == 0xc4
		{
			pbap_continue_f = 0;
			// int pb_event_type = PBAP_USER_DATA->storage < 2 ? 1 : 0;
			// bse_log_info(TAG,"PBAP_EVT_PULL_PHONEBOOK_COMPLETE pb_event_type = %d \n", pb_event_type);
			// hlp_pbap_pull_phonebook_complete(param, pb_event_type);
			FsmAddTimer(NULL, 150, 0, NULL, NULL, (UINT32) hlp_pbap_error_phonbook_complete);
		}
#else
		hlp_pbap_pull_phonebook_complete(param, 0);
#endif
	}
		break;
	case PBAP_EVT_DISCONNECT_COMPLETE:
#ifdef PBAP_USER_DATA_PT
		pbap_continue_f = 0;
#endif
		hlp_pbap_disconnect_complete(context, param);
		break;
	default: /* PBAP_EVT_CLIENT_START_CFM */
		break;
	}
	bse_log_info(TAG,  "****hlp_get_phonebook_cfm_cbk, event=%x. Exit", event);
}

int hlp_get_current_phonebook_complete_status()
{
	bse_log_info(TAG,  "****hlp_get_current_phonebook_complete_status=%d.", p_pb_complete);
	return p_pb_complete;
}

void hlp_pbap_set_status(UINT8 status)
{
	bse_log_api(TAG,  "*********hlp_pbap_set_status %d.\r\n", status);

	UINT8 pb_stat = status;
	/*pb_stat = pb_stat > ST_PBAP_INIT ? status - 1 : status;

	bse_log_info(TAG,  "pb_stat2 = %d PBAP_STATE = %d\r\n", pb_stat, PBAP_STATE);
	if(pb_stat != PBAP_STATE)
		event_pbap_indication(EV_PBAP_STAT_EV, (void *)pb_stat);
	*/
	bse_log_info(TAG,  "\r\n pb_stat2 = %d PBAP_STATE = %d\r\n", pb_stat, PBAP_STATE);

	if(pb_stat != PBAP_STATE) {
		if(pb_stat > ST_PBAP_INIT) {
			event_pbap_indication(EV_PBAP_STAT_EV, (void *)(pb_stat - 1));
		} else {
			event_pbap_indication(EV_PBAP_STAT_EV, (void *)pb_stat);
		}
	}

	PBAP_STATE = status;
}

UINT8 hlp_pbap_get_status(void)
{
	if (s_pbap_user_data == NULL) {
		return ST_PBAP_UNINIT;
	} else {
		bse_log_api(TAG, "PBAP_STATE is %d", PBAP_STATE);
		return PBAP_STATE;
	}
}

UINT8 hlp_pbap_get_supported_repos()
{
	return s_pbap_user_data->supported_repos;
}

void *hlp_pbap_get_handle(void)
{
	bse_log_api(TAG, "hlp_pbap_get_handle PBAP pb_handle is %x", PBAP_USER_DATA->pb_handle);
	return PBAP_USER_DATA->pb_handle;
}

BOOL hlp_pbap_get_flag_auto_connect(void)
{
	return (BOOL)(PBAP_USER_DATA->flag_auto_connect);
}

void hlp_pbap_set_req_contact_info(INT32 mask)
{
	PBAP_USER_DATA->pbap_info_mask = mask;
}

void hlp_pbap_set_flag_auto_connect(BOOL flag)
{
	PBAP_USER_DATA->flag_auto_connect = flag;
}

void hlp_pbap_disconnect()
{
	PBAP_DisconnectReq(PBAP_USER_DATA->pb_handle);
}

#ifdef CONFIG_USE_PBAP_DATA_CACHE

#define MAX_BUFFER_SIZE (8*1024)
//first time to memset 0 for playing
#define NEED_RESET_NULL_BUFEER_COUNT 10
//first time cache max packet for buffer
#define MAX_CACHE_PACKETS 15//15//11
//if cached packets less than min, wait a longer time
#define MIN_CACHE_PACKETS 5 //7
//if cached packest less than limited, break, wait for MAX_CACHE
#define LIMITED_CACHE_PACKET 3

enum {
    STATE_UNINIT = 0x00,
    STATE_READY = 0x01, /*Ready means thread started*/
    STATE_IDLE = 0x02, /*Idle means no data in process*/
};

struct LoopCtrl {
	int thread_started;
	int data_processed;
	int is_cleaning;
};

static HANDLE pbap_data_ev;
static UINT8 s_block_ev = 0;
static struct LoopCtrl s_pbap_loop_ctrl;

static SYSTEM_LOCK s_pbap_lock = 0;
static struct BtList *s_pbap_data_list = NULL;
static pthread_t s_pbap_thread_id = 0;
static	uint64_t last_time = 0;

static int curr_audio_f = 1;
static pthread_t p_id = 0;

static void bt_pbap_cache_timer_play(void);

void hlp_pbap_start_pthread()
{
	bse_log_info(TAG,"%s", __func__);
	//bse_log_info(TAG,"%s %d\n", __func__);
	s_pbap_loop_ctrl.thread_started = 1;
	s_pbap_loop_ctrl.data_processed = 1;
	s_pbap_loop_ctrl.is_cleaning = 0;
#ifdef PBAP_USER_DATA_PT
	p_pb_state = PTHREAD_PBAP_READY;
#endif
	s_block_ev = STATE_READY | STATE_IDLE;
	if (pbap_data_ev == NULL) {
		pbap_data_ev = CREATE_EVENT(NULL);
	}
	bse_log_info(TAG,"pbap_data_ev =  %p\n", pbap_data_ev);
	if (s_pbap_data_list == NULL) {
		s_pbap_data_list = (struct BtList *)BtNewListNew(sizeof(struct PbapBuffStru));
		s_pbap_lock = CREATE_CRITICAL(NULL);
		
		pthread_create(&s_pbap_thread_id, NULL, bt_pbap_cache_timer_play, NULL); 
		pthread_detach(s_pbap_thread_id);
	} else {
		bt_pbap_cache_clean();
	}
}

void app_pbap_stop_pthread()
{
	//may not got there

	bse_log_info(TAG,"%s", __func__);
}

#if 0
void bt_handle_master_device_change(bt_bdaddr_t address)
{
	bse_log_info(TAG,"%s [%02X:%02X:%02X:%02X:%02X:%02X]", __func__, address.address[5], address.address[4],address.address[3],
						address.address[2], address.address[1], address.address[0]);

	ENTER_CRITICAL(s_pbap_lock);
	if (memcmp(s_master_device.address, address.address, BD_ADDR_LEN) == 0) {
		bse_log_info(TAG,"%s, same master device change. Ignore", __func__);
		LEAVE_CRITICAL(s_pbap_lock);
		return;
	}
	memcpy(s_master_device.address, address.address, BD_ADDR_LEN);
	bt_audio_cache_clean();
	s_pbap_loop_ctrl.is_cleaning = 1;
	LEAVE_CRITICAL(s_pbap_lock);
}
#endif

void bt_pbap_cache_clean()
{
	//s_pbap_loop_ctrl.thread_started = 0;
	//s_pbap_loop_ctrl.data_processed = 0;

	bse_log_info(TAG,"%s", __func__);
	if (s_pbap_data_list) {
	    ENTER_CRITICAL(s_pbap_lock);
		NewListRemoveAll(s_pbap_data_list);
	    LEAVE_CRITICAL(s_pbap_lock);
	}
	
}

static uint8_t bt_audio_cache_get_count(int flag)
{
	UINT8 counter = 0;
	struct AudioBuffStru *p_node;
	if (!s_pbap_data_list) return 0;

	p_node = s_pbap_data_list->head;
	while (p_node != NULL) {
		counter++;
		p_node = (struct AudioBuffStru *)NEXT_MEM(s_pbap_data_list, p_node);
	}

	if (counter > MAX_CACHE_PACKETS) {
		//bse_log_info(TAG,"%d bt_audio_cache count = %d\r\n", flag, counter);
		//bse_log_info(TAG,"bt_audio_cache count = %d, overrun!!!\r\n", counter);
		//bse_log_info(TAG,"counter > MAX_CACHE_PACKETS\n");
	}

	if (counter <= MIN_CACHE_PACKETS) {
		//bse_log_info(TAG,"%d bt_audio_cache count = %d\r\n", flag, counter);
		//bse_log_info(TAG,"bt_audio_cache count = %d, underrun!!!\r\n", counter);
	}

	if (counter <= LIMITED_CACHE_PACKET) {
		//bse_log_info(TAG,"%d bt_audio_cache count = %d\r\n", flag, counter);
	}

	if (counter == 0) {
		//bse_log_info(TAG,"counter == 0\n");
		BRT_DBG("bt_audio_cache writen data finished!!!");
	}

	return counter;
}

static uint8_t bt_pbap_cache_add_data(tPBAP_ObjectDataStru *pbap_data, int len)
{
	//bse_log_info(TAG,"bt_pbap_cache_add_data len = %d\n", len);
	struct BtList *p_device_list = s_pbap_data_list;
	struct PbapBuffStru *pBuff = NULL;

	ENTER_CRITICAL(s_pbap_lock);
	if (s_pbap_loop_ctrl.is_cleaning && !(s_block_ev & STATE_IDLE)) {
		//bse_log_info(TAG,"%s, Not add new data to list", __func__);
		LEAVE_CRITICAL(s_pbap_lock);	
		return 1;
	}
	s_pbap_loop_ctrl.is_cleaning = 0;

	if (p_device_list && pbap_data != NULL && len > 0) {
		UINT16 buf_len = 0;
		pBuff = (struct PbapBuffStru *)BtListMemberNew(sizeof(struct PbapBuffStru));
		buf_len = len < MAX_BUFFER_SIZE ? len : MAX_BUFFER_SIZE;
		if (pBuff) {
			//memcpy(pBuff, pbap_data, buf_len);
			//pBuff->len = buf_len;
			//memcpy(pBuff->address.address, address.address, BD_ADDR_LEN);
			memcpy(pBuff->data, pbap_data->data, len);
			//strncpy(pBuff->data, pbap_data->data, len);
			pBuff->length = len;
			pBuff->is_final = pbap_data->is_final;

			NewListAddTail(p_device_list, (UINT8 *)pBuff);
		} else {
			bse_log_info(TAG,"%s, New buffer fail");
		}
	}	

	//uint8_t count = bt_audio_cache_get_count(1);
	LEAVE_CRITICAL(s_pbap_lock);
	//bse_log_info(TAG,"a2dp count = %d\n", count);	
	//if (count > MAX_CACHE_PACKETS) {
	//if (1) {
		//first time cache 20 packets
		//bse_log_info(TAG,"s_block_ev = %d\n", s_block_ev);
		if (s_block_ev & STATE_IDLE) {
			s_block_ev &= ~STATE_IDLE;
			bse_log_info(TAG,"SET EVENT");
			p_pb_state = PTHREAD_PBAP_GETTING;
			SET_EVENT(pbap_data_ev);
		}
	//}
	bse_log_info(TAG,"bt_pbap_cache_add_data end\n");
	return 1;
}

static void bt_pbap_cache_timer_play(void)
{
	bse_log_info(TAG,"%s", __func__);

	static int p_datalen = -1;
	struct timespec timeout;
	timeout.tv_sec = 0;
	struct PbapBuffStru *p_node = NULL;
	//void *p_node = NULL;

//	uint64_t total_processed = 0;
	uint64_t total_duration = 0;
	uint64_t total_elapsed = 0;
	uint64_t single_packet_time = 0;
//	uint64_t last_time = 0;

	while (s_pbap_loop_ctrl.thread_started) {
		bse_log_info(TAG,"Into Loop 1, wait pbap_data_ev");
		WAITFOR_SINGLEOBJECT(pbap_data_ev);
		bse_log_info(TAG,"Starting Loop 2");
        //total_processed = 0;
        last_time = 0;
		while (s_pbap_loop_ctrl.data_processed) {

			ENTER_CRITICAL(s_pbap_lock);
			p_node = (struct PbapBuffStru *)NewListRemoveHead(s_pbap_data_list);
			//p_node = NewListRemoveHead(s_pbap_data_list);
			//uint8_t count = bt_audio_cache_get_count(0);
			LEAVE_CRITICAL(s_pbap_lock);
			
			//bse_log_info(TAG,"p_node = %d\n", p_node->length);
			if (p_node) {
				p_pb_state = PTHREAD_PBAP_GETTING;
				hlp_pbap_phonebook_data_ind(p_node);

				//if(p_node->length < p_datalen)
				//{
					
				//}
				//p_datalen = p_node->length;
				BtListMemberFree(p_node);
				p_node = NULL;
				//usleep(100000);
			} else {
				if(hlp_get_current_phonebook_complete_status())
				{
					//p_datalen = -1;
					//bse_log_info(TAG,"List is Empty ******************* ");
					s_block_ev |= STATE_IDLE;
					
					int curr_pbdata_event_type = 0;

					bse_log_info(TAG,"PBAP_USER_DATA->storage = %d\n", PBAP_USER_DATA->storage);
					if(PBAP_USER_DATA->storage < 2){
						curr_pbdata_event_type = 1;
						hlp_event_last_day_pbap_packet(pbap_continue_f);
					}

					bse_log_info(TAG,"hlp_pbap_pull_phonebook_complete s_pb_complete.result = %02x\n", s_pb_complete.result);
					if(s_pb_complete.result == 0x00)//s_pb_complete.result != 0xc4
						hlp_pbap_pull_phonebook_complete((void *)&s_pb_complete, curr_pbdata_event_type);
						p_pb_state = PTHREAD_PBAP_DONE;

					break;
				} else {
					usleep(200000);
				}
			}
//			nanosleep(&timeout, &timeout);
		}
	}
}
#else
void app_pbap_start_pthread(unsigned char type){}
void app_pvap_stop_pthread(){}
//void bt_handle_master_device_change(bt_bdaddr_t address) {}
#endif