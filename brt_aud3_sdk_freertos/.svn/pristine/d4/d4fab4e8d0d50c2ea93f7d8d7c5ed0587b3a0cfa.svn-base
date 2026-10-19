/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/
#include "autoconf_bt.h"

#ifdef CONFIG_MAP
//#include "fileio.h"
#include "expat.h"
#include "internal_map.h"
#include "map_hlp.h"
#include "vcf_parse.h"
#include "bt_power_manager.h"

#define STRCASECMP strcmp
#define TAG "bse_map"
/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     hlp_map.c
 * @Description:
        This file provides basic map operations and callbacks for MAP module.
 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-14
 ---------------------------------------------------------------------------*/

typedef struct {
	UINT8	bd[BDLENGTH];
	UINT8	is_server;			/* 0 - MAS Client; 1 - MAS Server */
	UINT8	connected;			/* 0 - connecting; 1 - connected */
	void	*mac_inst_handle;	/* MAS connection instance handle */
	void	*mns_con_handle;	/* MNS connection instance handle  */
} tAppConnInstanceStru;

typedef struct {
	UINT32 total_len;
	UINT32 remain_len;
	UINT8  *content;
} tAppClntContextStru;

typedef struct tag_hlp_map_get_msg {
	UINT8 msg_handle[17];
} hlp_map_get_msg_context_t;


/* FIXME length depends on "bt_event_indicate" */
#ifdef CONFIG_UNLIMIT_RESOURCE
#define HLP_EV_MAP_IND_MAX_LEN  (256 * 3)
#else
#define HLP_EV_MAP_IND_MAX_LEN  250
#endif
#define MAP_MSG_PATH_DEPTH 2
static char *MAP_MSG_FULL_PATH[MAP_MSG_PATH_DEPTH] = {"telecom", "msg"};
#define PUSH_DIRECTORY "outbox"

static hlp_map_context_t *s_context = NULL;

/*add*/
static UINT8 *s_map_contont = NULL;
map_message_info_t g_new_msg = {0};

static unsigned char s_map_info_parsed = 1;
static int s_contont_length = 0;
static int s_contont_index = 0;
static unsigned char s_map_mms_msg = FALSE;//save new message type, is mms or not
static UINT8 s_map_inbox_msg = FALSE;
static map_message_info_t *s_map_get_list_step = NULL;

extern unsigned char XML_ParserReset(XML_Parser parser, const XML_Char *encoding);
static void map_xml_start_handler(void *userdata, const char *name, const char **atts);
static void map_xml_end_handler(void *userdata, const char *name);
static void map_parser_free(XML_Parser *parser, int type);
static void hlp_map_free_xml_object(UINT8 *data, UINT8 type);

static void *map_selection_connection(void);
static void app_mns_event_callback(void *handle, void *context, tMAP_Event_Type event, void *param);
static void map_app_connection_event_callback(void *handle, void *context, tMAP_Event_Type event, void *param);
static void map_app_set_folder_callback(void *handle, void *context, tMAP_Event_Type event, UINT16 *result);
static void map_app_reg_notification_callback(void *handle, void *context, tMAP_Event_Type event, tMAP_NotificationRegCompleteStru *param);
static void map_find_mse_device_to_discover_callback(void *context, UINT16 result, struct GAP_DeviceDiscoveryCfmStru *cfm);
static void map_app_get_message_list_callback(void *handle, XML_Parser *parser, tMAP_Event_Type event, void *param);
static void map_app_get_folder_list_callback(void *handle, XML_Parser *parser, tMAP_Event_Type event, void *param);
static void map_app_push_message_callback(void *handle, tAppClntContextStru *context, tMAP_Event_Type event, void *param);
static void hlp_map_trigger_evt_callback(int event, UINT8 result);
static void hlp_map_AT_event_indicate(int event, UINT8 result, UINT8 *pdu, UINT16 length);

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        This function registers local services.
 */
/* ---------------------------------------------------------------------------*/
static void hlp_map_event_indication(int event, void *param)
{
	if (s_context->event_cbk) {
		s_context->event_cbk((PROFILE_ID_MAP << 8) | event, param);
	}
}

void hlp_map_init(module_event_cbk_t event_cbk)
{
	if (s_context) {
		tAppConnInstanceStru *cli_inst;

		while ((cli_inst = List_RemoveHead(&s_context->conn_list)) != NULL) {
			LFREE(cli_inst);
		}

		FREE(s_context);
	}

	s_context = (hlp_map_context_t *) NEW(sizeof(hlp_map_context_t));
	memset(s_context, 0, sizeof(hlp_map_context_t));
	s_context->state = STATE_READY;
	s_context->dir_depth = 0;
	s_context->event_cbk = event_cbk;

	s_context->get_msg_list = BtListNew(sizeof(hlp_map_get_msg_context_t));

	s_context->store_type = HLP_MAP_MESSAGE_LOCATION_IDLE;
	s_context->max_list_counts = 0;
	hlp_map_memory_list_init();

	/* Register MNS service */
	MAP_MNSServerStartReq(NULL, NULL, app_mns_event_callback);
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        This function to release resources and unregister MNS service.
 */
/* ---------------------------------------------------------------------------*/
void hlp_map_done(void)
{
	if (s_context) {
		tAppConnInstanceStru *cli_inst;

		while ((cli_inst = List_RemoveHead(&s_context->conn_list)) != NULL) {
			LFREE(cli_inst);
		}

		if (s_context->get_msg_list) {
			RemoveAll(s_context->get_msg_list);
			BtListFree(s_context->get_msg_list);
			s_context->get_msg_list = NULL;
		}

		FREE(s_context);
		s_context = NULL;
	}

	MAP_MNSServerStopReq();
}

void hlp_map_register_callback(app_map_event_callback callback)
{
	if (s_context) {
		s_context->callback = callback;
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Connect to MSE.
 */
/* ---------------------------------------------------------------------------*/
void hlp_map_connect(UINT8 *bd)
{
	if (bd && s_context) {
		s_context->state = STATE_CONNECTING;
		MAP_MASClientStartReq(NULL, bd, NULL, map_app_connection_event_callback);
	} else {
		hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_INVALID_PARAMETER);
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Disconnect or cancel the connection.
 */
/* ---------------------------------------------------------------------------*/
void hlp_map_disconnect(void)
{
	tAppConnInstanceStru *cli_inst = map_selection_connection();

	if (cli_inst != NULL) {
		MAP_ClientStopReq(cli_inst->mac_inst_handle);
	}
}

void hlp_map_get_message_listing(int location, UINT16 start_offset, UINT16 max_count)
{
	tMAP_GetMessageListReqStru req_param;
	tAppConnInstanceStru *cli_inst = map_selection_connection();

	if (cli_inst != NULL && s_context->state == STATE_CONNECTED) {
		XML_Parser p = XML_ParserCreate(NULL);
		MAPXMLFINDDATA *user_data;
		char *pf = NULL;

		XML_SetElementHandler(p, map_xml_start_handler, map_xml_end_handler);
		user_data = (MAPXMLFINDDATA *) NEW(sizeof(MAPXMLFINDDATA));

		if (user_data) {
			memset(user_data, 0, sizeof(MAPXMLFINDDATA));
			user_data->file_type = IDX_MAP_FILE_MSG_LISTING;
		}
		XML_SetUserData(p, user_data);

		memset(&req_param, 0, sizeof(tMAP_GetMessageListReqStru));
		req_param.flag = MAP_FLAG_MAXLISTCOUNT | MAP_FLAG_LISTSTARTOFFSET;
		req_param.max_list_count= max_count;
		req_param.list_start_offset= start_offset;
		//req_param.filter_message_type = MAP_FILTER_MSGTYPE_SMS_GSM | MAP_FILTER_MSGTYPE_SMS_CDMA;

		switch (location) {
		case HLP_MAP_MESSAGE_LOCATION_INBOX:
			pf = "inbox";
			break;

		case HLP_MAP_MESSAGE_LOCATION_OUTBOX:
			pf = "outbox";
			break;

		case HLP_MAP_MESSAGE_LOCATION_SENT:
			pf = "sent";
			break;

		case HLP_MAP_MESSAGE_LOCATION_DELETED:
			pf = "deleted";
			break;

		case HLP_MAP_MESSAGE_LOCATION_DRAFT:
			pf = "draft";
			break;
		}

		if (pf) {
			UINT8 folder[16] = {0};
			ByteToWide(pf, folder, strlen(pf));
			MAP_GetMessageListReq(cli_inst->mac_inst_handle, p, folder, &req_param, map_app_get_message_list_callback);
		}
	} else {
		// TODO hlp_map_trigger_evt_callback(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_CONNECTON_NOT_EXIST);
	}
}

void hlp_map_get_folder_listing(void)
{
	tMAP_GetFolderListReqStru req_param;
	tAppConnInstanceStru *cli_inst = map_selection_connection();

	if (cli_inst != NULL && s_context->state == STATE_CONNECTED) {
		XML_Parser p = XML_ParserCreate(NULL);
		MAPXMLFINDDATA *user_data;
		XML_SetElementHandler(p, map_xml_start_handler, map_xml_end_handler);
		user_data = (MAPXMLFINDDATA *) NEW(sizeof(MAPXMLFINDDATA));

		if (user_data) {
			memset(user_data, 0, sizeof(MAPXMLFINDDATA));
			user_data->file_type = IDX_MAP_FILE_FOLDER_LISTING;
		}
		XML_SetUserData(p, user_data);

		memset(&req_param, 0, sizeof(tMAP_GetFolderListReqStru));
		req_param.flag = MAP_FLAG_MAXLISTCOUNT;
		req_param.max_list_count = 0x5000;
		MAP_GetFolderListReq(cli_inst->mac_inst_handle, p, &req_param, map_app_get_folder_list_callback);
	} else {
		// TODO
	}
}


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        This function request the MSE to send a SMS message to target peer.
 *
 * @Arguments:
        content  The content of the SMS message to send
        length The length of content
 */
/* ---------------------------------------------------------------------------*/
void hlp_map_send_message(UINT8 *content, UINT16 length)
{
	tMAP_PushMessageReqStru req_param;
	tAppConnInstanceStru *cli_inst;

	if (content == NULL || length <= 0) {
		hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, MAP_RESULT_INVALID_PARAMETER);
	}

	cli_inst = map_selection_connection();

	if (cli_inst != NULL && s_context->state == STATE_CONNECTED) {
		char *folder;
		tAppClntContextStru *context = (tAppClntContextStru *) NEW(sizeof(tAppClntContextStru));

		if (context == NULL) {
			hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, MAP_RESULT_MEMORY_ERROR);
			return;
		}

		context->total_len = length;
		context->remain_len = length;
		context->content = NEW(length);

		if (context->content == NULL) {
			hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, MAP_RESULT_MEMORY_ERROR);
			FREE(context);
			return;
		}

		memcpy(context->content, content, length);
		req_param.flag = MAP_FLAG_CHARSET | MAP_FLAG_RETRY | MAP_FLAG_TRANSPARENT;
		req_param.charset = MAP_CHARSET_UTF8;
		req_param.retry = MAP_MASK_ON;
		req_param.transparent = MAP_MASK_ON;

		folder = NEW(16);

		if (folder != NULL) {
			memset(folder, 0, 16);
			ByteToWide(PUSH_DIRECTORY, folder, strlen(PUSH_DIRECTORY));
		}

		MAP_PushMessageReq(cli_inst->mac_inst_handle, context, folder, &req_param, map_app_push_message_callback);

		if (folder != NULL) {
			FREE(folder);
		}
	} else {
		hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, MAP_RESULT_CONNECTON_NOT_EXIST);
	}
}

static void hlp_map_get_message_cfm_cbk(void *handle, UINT8 *context_handle, tMAP_Event_Type event, void *param)
{
	static UINT8 *p_data = NULL;
	static UINT32 recv_len = 0;
	tAppMACEvtParameters real_par;
	real_par.param = param;

	switch (event) {
	case MAP_EVT_OBJECT_DATA_IND: {
		app_vcfparser_msg_handle_ind(s_context->p_vcf_parser, context_handle);
		if (real_par.obj_data->data != NULL) {
			UINT32 i = 0;
			UINT32 data_len = strlen(real_par.obj_data->data);

			for (i = 0; i<data_len; i++) {
				app_vcfparser_data_ind(s_context->p_vcf_parser, real_par.obj_data->data+i);
			}
			map_parse(context_handle, real_par);
		}
	}
	break;
	case MAP_EVT_GET_MESSAGE_COMPLETE: {
		hlp_map_get_msg_context_t *p_node = NULL;
		UINT8 buff[3] = {'\r', '\n', 0};

		//app_vcfparser_data_ind(s_context->p_vcf_parser, buff);
		//app_vcfparser_data_ind(s_context->p_vcf_parser, buff+1);
		//app_vcfparser_stor(s_context->p_vcf_parser);
		//app_vcfparser_free(s_context->p_vcf_parser);
		s_context->p_vcf_parser = NULL;

		if (context_handle != NULL) {
			FREE(context_handle);
		}

		s_context->state = STATE_CONNECTED;
		p_node = RemoveHead(s_context->get_msg_list);

		if (p_node != NULL) {
			hlp_map_get_message(p_node->msg_handle);
			BtListMemberFree(p_node);
		}
		if (s_map_get_list_step && strlen(s_map_get_list_step->message_handle) > 0) {
			hlp_map_get_message(s_map_get_list_step->message_handle);
			s_map_get_list_step = (map_message_info_t *)NEXT_MEM(s_context->msg_list, s_map_get_list_step);
		} else if (s_context->get_list_flag) {
//                    bt_event_indicate(BT_EV_MAP_GET_MESSAGE_COMPLETE, NULL);
			hlp_map_memory_list_done();
			s_context->get_list_flag = FALSE;
		}
	}
	break;
	default:
		break;
	}
}

void hlp_map_get_message(UINT8 *msg_handle)
{
	tAppConnInstanceStru *cli_inst = NULL;
	tMAP_GetMessageReqStru req_param;
	WCHAR w_msg_handle[17];

	if (msg_handle != NULL) {
		if (s_context->state == STATE_CONNECTED) {
			int handle_len = strlen(msg_handle);
			UINT8 *context_handle = (UINT8 *) NEW(handle_len + 1);
			cli_inst = map_selection_connection();

			if(cli_inst) {
				memcpy(context_handle, msg_handle, handle_len + 1);
				memset(w_msg_handle, 0, 17 * sizeof(WCHAR));
				ByteToWide(msg_handle, w_msg_handle, handle_len);

				memset(&req_param, 0, sizeof(tMAP_GetMessageReqStru));
				req_param.flag = MAP_FLAG_ATTACHMENT | MAP_FLAG_CHARSET/* | MAP_FLAG_FRACTIONREQUEST*/;
				req_param.attachment = MAP_MASK_ON;
				req_param.charset = MAP_CHARSET_UTF8;
				req_param.fraction_request = 0;

				s_context->state = STATE_GETTING;
				s_context->p_vcf_parser = app_vcfparser_alloc(VCF_PARSE_TYPE_MSG, "MAP");

				if (s_context->p_vcf_parser) {
					s_context->p_vcf_parser->item_callback = hlp_map_event_indication;
				}

				MAP_GetMessageReq(cli_inst->mac_inst_handle, context_handle, w_msg_handle, &req_param, hlp_map_get_message_cfm_cbk);
			}
		} else if (s_context->state == STATE_GETTING) {
			hlp_map_get_msg_context_t *p_node = (hlp_map_get_msg_context_t *)BtListMemberNew(sizeof(hlp_map_get_msg_context_t));
			if (p_node != NULL) {
				memset(p_node, 0, sizeof(hlp_map_get_msg_context_t));
				memcpy(p_node->msg_handle, msg_handle, strlen(msg_handle)+1);
				AddTail(s_context->get_msg_list, p_node);
			}
		}
	}
}

void hlp_map_delete_message_cbk(void *handle, void *context, tMAP_Event_Type event, UINT16 *result)
{
	switch (event) {
	case MAP_EVT_SET_MESSAGE_STATUS_COMPLETE:
		bse_log_api(TAG, "%s:result:%d.", __FUNCTION__, *result);
		hlp_map_trigger_evt_callback(EV_MAP_DELETE_SMS_RESULT, *result);
		break;
	default:
		break;
	}
}

void hlp_map_delete_message(UINT8 *msg_handle)
{
	tAppConnInstanceStru *cli_inst = NULL;
	WCHAR w_msg_handle[16];

	if (msg_handle != NULL && s_context->state == STATE_CONNECTED) {
		cli_inst = map_selection_connection();
		if(cli_inst) {
			memset(w_msg_handle, 0, 16);
			ByteToWide(msg_handle, w_msg_handle, strlen(msg_handle));

			MAP_SetMessageStatusReq(cli_inst->mac_inst_handle, NULL, w_msg_handle, MAP_MSGSTATUS_DELETED, hlp_map_delete_message_cbk);
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCE application event callback - MNS Server events.
------------------------------------------------------------------------------------*/
static void app_mns_event_callback(void *handle, void *context, tMAP_Event_Type event, void *param)
{
	tAppMACEvtParameters real_par;
	real_par.param = param;

	switch (event) {
	case MAP_EVT_SERVER_START_COMPLETE:
		break;

	case MAP_EVT_CONNECT_COMPLETE:
		break;

	case MAP_EVT_DISCONNECT_COMPLETE:
		break;

	case MAP_EVT_EVENT_REPORT_IND:
		break;

	case MAP_EVT_SERVER_STOP_COMPLETE:
		break;

	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCE application event callback - Connection events.
------------------------------------------------------------------------------------*/
static void map_app_connection_event_callback(void *handle, void *context, tMAP_Event_Type event, void *param)
{
	tAppConnInstanceStru *cli_inst;
	tAppMACEvtParameters real_par;
	real_par.param = param;

	if (s_context == NULL) {
		return;
	}

	switch (event) {
	case MAP_EVT_CLIENT_START_CFM:
		cli_inst = List_NodeNew(sizeof(tAppConnInstanceStru));
		memcpy(cli_inst->bd, param, BDLENGTH);
		cli_inst->mac_inst_handle = handle;
		List_AddHead(&s_context->conn_list, cli_inst);
		break;

	case MAP_EVT_CONNECT_COMPLETE:
		cli_inst = LIST_FIND(&s_context->conn_list, tAppConnInstanceStru *, mac_inst_handle, handle);

		if (real_par.conn_cmp->result == ER_MAP_SUCCESS) {
			cli_inst->connected = TRUE;
			s_context->state = STATE_REGISTERING_NOTIF;
			s_context->dir_depth = 0;
			MAP_SetNotificationRegistrationReq(cli_inst->mac_inst_handle, NULL, map_app_reg_notification_callback);
		} else {
			if (cli_inst != NULL) {
				List_RemoveAt(&s_context->conn_list, cli_inst);
				LFREE(cli_inst);
			}

			s_context->state = STATE_READY;
			/* XXX fail reason ignored */
			hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_CONNECT_FAILED);
		}

		break;

	case MAP_EVT_EVENT_REPORT_IND:
		if (real_par.evt_report->length > 0 && real_par.evt_report->data) {
			XML_Parser p = XML_ParserCreate(NULL);
			MAPXMLFINDDATA *user_data;

			XML_SetElementHandler(p, map_xml_start_handler, map_xml_end_handler);
			user_data = (MAPXMLFINDDATA *) NEW(sizeof(MAPXMLFINDDATA));

			if (user_data) {
				memset(user_data, 0, sizeof(MAPXMLFINDDATA));
				user_data->file_type = IDX_MAP_FILE_EVENT_OBJECT;
			}

			XML_SetUserData(p, user_data);

			if (XML_Parse(p, real_par.evt_report->data, real_par.evt_report->length, 0) == XML_STATUS_ERROR) {
				bse_log_api(TAG, "Parse error at line %d:\n %s\n",
				            XML_GetCurrentLineNumber(p), XML_ErrorString(XML_GetErrorCode(p)));
			}

			// FIXME the is_final never is MAP_DATA_COMPLETE
			if (XML_Parse(p, NULL, 0, 1) == XML_STATUS_ERROR) {
				bse_log_api(TAG, "Parse error at line \n%s\n", XML_ErrorString(XML_GetErrorCode(p)));
			}

			map_parser_free(p, IDX_MAP_FILE_EVENT_OBJECT);
		}

		break;

	case MAP_EVT_DISCONNECT_COMPLETE:
		cli_inst = LIST_FIND(&s_context->conn_list, tAppConnInstanceStru *, mac_inst_handle, handle);

		if (cli_inst != NULL) {
			List_RemoveAt(&s_context->conn_list, cli_inst);
			LFREE(cli_inst);
		}

		s_context->state = STATE_READY;
		hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_DISCONNECTED);

		break;

	default:
		break;
	}
}

static void map_app_set_folder_callback(void *handle, void *context, tMAP_Event_Type event, UINT16 *result)
{
	FREE(context);

	if (*result != ER_MAP_SUCCESS) {
		/* should NOT enter here */
		s_context->state = STATE_READY;
		hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_CONNECT_FAILED);
		return;
	}

	s_context->dir_depth++;

	if (s_context->dir_depth < MAP_MSG_PATH_DEPTH) {
		tAppConnInstanceStru *cli_inst = map_selection_connection();

		if (cli_inst) {
			char *dir = MAP_MSG_FULL_PATH[s_context->dir_depth];
			int str_size = strlen(dir) + 1;
			WCHAR *folder = NEW(sizeof(WCHAR) * str_size);
			memset(folder, 0, sizeof(WCHAR) * str_size);
			ByteToWide(dir, (char *)folder, str_size);
			MAP_SetChildFolderReq(cli_inst->mac_inst_handle, folder, folder, map_app_set_folder_callback);
		} else {
			/* should NOT enter here */
			s_context->state = STATE_READY;
			hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_CONNECT_FAILED);
		}
	} else {
		s_context->state = STATE_CONNECTED;
		hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_CONNECTED);
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Select a connection for further operation.
---------------------------------------------------------------------------*/
static void *map_selection_connection(void)
{
	tAppConnInstanceStru *cli_inst = NULL;

	// FIXME if want to support multiple connections, we should traverse the list to find the target connection
	if (s_context->conn_list.head == s_context->conn_list.tail) {
		cli_inst = s_context->conn_list.head;
	}

	return cli_inst;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCE application event callback - Update Inbox complete event.
------------------------------------------------------------------------------------*/
static void map_app_reg_notification_callback(void *handle, void *context, tMAP_Event_Type event, tMAP_NotificationRegCompleteStru *param)
{
	// FIXME enter specified folder here?
	tAppConnInstanceStru *cli_inst = map_selection_connection();

	if (cli_inst != NULL) {
		char *dir;
		int str_size;
		WCHAR *folder;
		s_context->state = STATE_SETTING_FOLDER;
		dir = MAP_MSG_FULL_PATH[s_context->dir_depth];
		str_size = strlen(dir) + 1;
		folder = NEW(sizeof(WCHAR) * str_size);
		memset(folder, 0, sizeof(WCHAR) * str_size);
		ByteToWide(dir, (char *)folder, str_size);
		MAP_SetChildFolderReq(cli_inst->mac_inst_handle, folder, folder, map_app_set_folder_callback);
	} else {
		/* XXX should NOT reach here. */
		hlp_map_trigger_evt_callback(EV_MAP_CONNECTION_EVENT, MAP_RESULT_CONNECT_FAILED);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCE application event callback - Object data indication and Get MessageListing complete event
------------------------------------------------------------------------------------*/
static void map_app_get_message_list_callback(void *handle, XML_Parser *parser, tMAP_Event_Type event, void *param)
{
	tAppMACEvtParameters real_par;
	WCHAR cur_folder[] = {'.', 0};
	real_par.param = param;

	switch (event) {
	case MAP_EVT_OBJECT_DATA_IND:
		if (real_par.obj_data->data != NULL) {
			if (parser) {
				XML_Parse(parser, real_par.obj_data->data, real_par.obj_data->length, 0);
			}
		}

		break;

	case MAP_EVT_ASK4_GET_REQUEST_IND:
		// XXX
		break;

	case MAP_EVT_GET_MESSAGELIST_COMPLETE:
		if (real_par.get_msglist_cmp->result == ER_MAP_SUCCESS) {
			if (real_par.get_msglist_cmp->flag & MAP_FLAG_MESSAGELISTINGSIZE) {
				// TODO notify size: real_par.get_msglist_cmp->message_listing_size
			}

			if (parser) {
				XML_Parse(parser, NULL, 0, 1);
			}
		}

		hlp_map_event_indication(EV_MAP_GET_SMS_LIST_COMPLETE, 0);

		// TODO notify result: real_par.get_msglist_cmp->result
		map_parser_free(parser, IDX_MAP_FILE_MSG_LISTING);
		break;

	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCE application event callback - Object data indication and Get FolderListing complete event
------------------------------------------------------------------------------------*/
static void map_app_get_folder_list_callback(void *handle, XML_Parser *parser, tMAP_Event_Type event, void *param)
{
	tAppMACEvtParameters real_par;
	real_par.param = param;

	switch (event) {
	case MAP_EVT_OBJECT_DATA_IND:
		if (real_par.obj_data->data != NULL) {
			if (parser) {
				int ret = XML_Parse(parser, real_par.obj_data->data, real_par.obj_data->length, 0);
			}
		}

		break;

	case MAP_EVT_ASK4_GET_REQUEST_IND:
		break;

	case MAP_EVT_GET_FOLDERLIST_COMPLETE:
		if (real_par.get_fdlist_cmp->result == ER_MAP_SUCCESS) {
			if (parser) {
				int ret = XML_Parse(parser, NULL, 0, 1);
			}
		} else {
			bse_log_info(TAG, "> Failed to get FolderListing, reason is %04x.", real_par.get_fdlist_cmp->result);
		}

		map_parser_free(parser, IDX_MAP_FILE_FOLDER_LISTING);
		break;

	default:
		break;
	}
}

static void map_app_push_message_callback(void *handle, tAppClntContextStru *context, tMAP_Event_Type event, void *param)
{
	tAppMACEvtParameters real_par;
	real_par.param = param;

	switch (event) {
	case MAP_EVT_ASK4_PUT_REQUEST_IND:
		if (context->remain_len < real_par.ask4_put_req->max_data_size) {
			real_par.ask4_put_req->max_data_size = context->remain_len;
		}

		break;

	case MAP_EVT_ASK4_MESSAGE_DATA_IND:
		if (real_par.obj_data->length > context->remain_len) {
			real_par.obj_data->length = context->remain_len;
		}

		memcpy(real_par.obj_data->data, context->content + context->total_len - context->remain_len, real_par.obj_data->length);

		if (real_par.obj_data->length < context->remain_len) {
			context->remain_len -= real_par.obj_data->length;
		} else {
			context->remain_len = 0;
			real_par.obj_data->is_final = MAP_DATA_COMPLETE;
		}

		break;

	case MAP_EVT_PUSH_MESSAGE_COMPLETE:
		if (real_par.put_msg_cmp->result == ER_MAP_SUCCESS) {
			bse_log_info(TAG, "Message(len:%d) pushed (Handle 0x%x)!", context->total_len, real_par.put_msg_cmp->message_handle);
			hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, MAP_RESULT_OK);
		} else {
			bse_log_info(TAG, "Failed to push Message(len:%d-%d), reason is %04x.",
			             context->total_len, context->remain_len, real_par.put_msg_cmp->result);
			hlp_map_trigger_evt_callback(EV_MAP_SEND_SMS_RESULT, real_par.put_msg_cmp->result);
		}

		FREE(context->content);
		FREE(context);
		break;

	default:
		break;
	}
}

static void map_xml_message_list_start_handler(MAPXMLFINDDATA *user_data, const char *name, const char **atts)
{
	map_message_list_item_t *obj_stru = (map_message_list_item_t *) List_NodeNew(sizeof(map_message_list_item_t));
	UINT8 **p1 = NULL;
	UINT8 **p2 = NULL;
	int known_key = 1;
	char header[16];

	p1 = (UINT8 **) atts;
	memset(header, 0, 16);
	strcpy(header, "MSG:BEGIN");
	hlp_map_AT_event_indicate(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_OK, header, strlen(header));

	while (*p1 != NULL) {
		p2 = p1 + 1;
		known_key = 1;

		if (STRCASECMP(*p1, "handle") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->msg_hdl, (*p2));
		} else if (STRCASECMP(*p1, "subject") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->subject, (*p2));

			if (obj_stru->subject != NULL) {
				obj_stru->mask |= MASK_ITEM_SUBJECT;
			}
		} else if (STRCASECMP(*p1, "datetime") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->date_time, (*p2));

			if (obj_stru->date_time != NULL) {
				obj_stru->mask |= MASK_ITEM_DATETIME;
			}
		} else if (STRCASECMP(*p1, "sender_name") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->sender_name, (*p2));

			if (obj_stru->sender_name != NULL) {
				obj_stru->mask |= MASK_ITEM_SENDER_NAME;
			}
		} else if (STRCASECMP(*p1, "sender_addressing") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->sender_addr, (*p2));

			if (obj_stru->sender_addr != NULL) {
				obj_stru->mask |= MASK_ITEM_SENDER_ADDR;
			}
		} else if (STRCASECMP(*p1, "replyto_addressing") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->replyto_addr, (*p2));

			if (obj_stru->replyto_addr != NULL) {
				obj_stru->mask |= MASK_ITEM_REPLYTO_ADDR;
			}
		} else if (STRCASECMP(*p1, "recipient_name") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->recipient_name, (*p2));

			if (obj_stru->recipient_name != NULL) {
				obj_stru->mask |= MASK_ITEM_RECIPIENT_NAME;
			}
		} else if (STRCASECMP(*p1, "recipient_addressing") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->recipient_addr, (*p2));

			if (obj_stru->recipient_addr != NULL) {
				obj_stru->mask |= MASK_ITEM_RECIPIENT_ADDR;
			}
		} else if (STRCASECMP(*p1, "type") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->msg_type, (*p2));

			if (obj_stru->msg_type != NULL) {
				obj_stru->mask |= MASK_ITEM_MSG_TYPE;
			}
		} else if (STRCASECMP(*p1, "size") == 0) {
			obj_stru->msg_size = EnhancedAtoI(*p2);   /*DataType Convert*/
			obj_stru->mask |= MASK_ITEM_MSG_SIZE;
		} else if (STRCASECMP(*p1, "text") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->is_text, (*p2));

			if (obj_stru->is_text != NULL) {
				obj_stru->mask |= MASK_ITEM_IS_TEXT;
			}
		} else if (STRCASECMP(*p1, "reception_status") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->reception_status, (*p2));

			if (obj_stru->reception_status != NULL) {
				obj_stru->mask |= MASK_ITEM_RECEPTION_STATUS;
			}
		} else if (STRCASECMP(*p1, "attachment_size") == 0) {
			obj_stru->attach_size = EnhancedAtoI(*p2);   /*DataType convert*/
			obj_stru->mask |= MASK_ITEM_ATTACHMENT_SIZE;
		} else if (STRCASECMP(*p1, "priority") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->msg_priority, (*p2));

			if (obj_stru->msg_priority != NULL) {
				obj_stru->mask |= MASK_ITEM_PRIORITY;
			}
		} else if (STRCASECMP(*p1, "read") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->is_read, (*p2));

			if (obj_stru->is_read != NULL) {
				obj_stru->mask |= MASK_ITEM_IS_READ;
			}
		} else if (STRCASECMP(*p1, "sent") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->is_send, (*p2));

			if (obj_stru->is_send != NULL) {
				obj_stru->mask |= MASK_ITEM_IS_SEND;
			}
		} else if (STRCASECMP(*p1, "protected") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->is_protected, (*p2));

			if (obj_stru->is_protected != NULL) {
				obj_stru->mask |= MASK_ITEM_IS_PROTECTED;
			}
		} else {
			known_key = 0;
		}

		if (known_key) {
			int len = strlen(*p1) + strlen(*p2) + 2;
			char *at_str = (char *) NEW(len);

			if (at_str) {
				memset(at_str, 0, len);

				if (strlen(*p2) > 0) {
					snprintf(at_str, len, "%s:%s", *p1, *p2);
				} else {
					snprintf(at_str, len, "%s:", *p1);
				}

				hlp_map_AT_event_indicate(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_OK, at_str, len);
				FREE(at_str);
			}
		}

		p1 += 2;
	}

	memset(header, 0, 16);
	strcpy(header, "MSG:END");
	hlp_map_AT_event_indicate(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_OK, header, strlen(header));
	//List_AddHead(&user_data->data, obj_stru);

	if (s_context && s_context->callback) {
		s_context->callback(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_OK, obj_stru, sizeof(map_message_list_item_t));
	}

	hlp_map_free_xml_object(obj_stru, IDX_MAP_FILE_MSG_LISTING);
}

static void map_xml_event_object_start_handler(MAPXMLFINDDATA *user_data, const char *name, const char **atts)
{
	map_event_object_t *obj_stru = (map_event_object_t *) List_NodeNew(sizeof(map_event_object_t));
	UINT8 **p1 = NULL;
	UINT8 **p2 = NULL;
	int known_key = 1;
	char header[16];

	p1 = (UINT8 **) atts;

	memset(header, 0, 16);
	strcpy(header, "MSG:BEGIN");
	hlp_map_AT_event_indicate(EV_MAP_NEW_SMS_ARRIVED, MAP_RESULT_OK, header, strlen(header));

	while (*p1 != NULL) {
		p2 = p1 + 1;

		if (STRCASECMP(*p1, "type") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->evt_type, (*p2));
		} else if (STRCASECMP(*p1, "handle") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->handle, (*p2));
		} else if (STRCASECMP(*p1, "folder") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->folder, (*p2));
		} else if (STRCASECMP(*p1, "old_folder") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->older_folder, (*p2));
		} else if (STRCASECMP(*p1, "msg_type") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->msg_type, (*p2));
		} else {
			known_key = 0;
		}

		if (known_key) {
			int len = strlen(*p1) + strlen(*p2) + 2;
			char *at_str = (char *) NEW(len);

			if (at_str) {
				memset(at_str, 0, len);

				if (strlen(*p2) > 0) {
					snprintf(at_str, len, "%s:%s", *p1, *p2);
				} else {
					snprintf(at_str, len, "%s:", *p1);
				}

				hlp_map_AT_event_indicate(EV_MAP_NEW_SMS_ARRIVED, MAP_RESULT_OK, at_str, len);
				FREE(at_str);
			}
		}

		p1 += 2;
	}

	memset(header, 0, 16);
	strcpy(header, "MSG:END");
	hlp_map_AT_event_indicate(EV_MAP_NEW_SMS_ARRIVED, MAP_RESULT_OK, header, strlen(header));
	List_AddHead(&user_data->data, obj_stru);

	if (s_context && s_context->callback) {
		s_context->callback(EV_MAP_GET_SMS_LIST_RESULT, MAP_RESULT_OK, obj_stru, sizeof(map_event_object_t));
	}
}

static void map_xml_folder_list_start_handler(MAPXMLFINDDATA *user_data, const char *name, const char **atts)
{
	map_folder_list_item_t *obj_stru = (map_folder_list_item_t *) List_NodeNew(sizeof(map_folder_list_item_t));
	UINT8 **p1 = NULL;
	UINT8 **p2 = NULL;
	p1 = (UINT8 **) atts;

	while (*p1 != NULL) {
		p2 = p1 + 1;

		if (STRCASECMP(*p1, "name") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->name, (*p2));

			if (STRCASECMP(name, "folder") == 0) {
				obj_stru->attrib |= MAP_ATTRIB_DIR; /*directory*/
			} else if (STRCASECMP(name, "file") == 0) {
				obj_stru->attrib |= MAP_ATTRIB_FILE;
			} else {
			}
		} else if (STRCASECMP(*p1, "size") == 0) {
			obj_stru->size = EnhancedAtoI(*p2);
			obj_stru->attrib |= MAP_ATTRIB_SIZE;
		} else if (STRCASECMP(*p1, "modified") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->write_time, (*p2));
		} else if (STRCASECMP(*p1, "created") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->create_time, (*p2));
		} else if (STRCASECMP(*p1, "accessed") == 0) {
			MAP_ALLOC_AND_COPY_STR(obj_stru->access_time, (*p2));
		} else {
		}

		p1 += 2;
	}

	List_AddHead(&user_data->data, obj_stru);
}

static void map_xml_start_handler(void *userdata, const char *name, const char **atts)
{
	MAPXMLFINDDATA *user_data;
	if (!userdata) {
		return;
	}

	user_data = (MAPXMLFINDDATA *) userdata;

	switch (user_data->file_type) {
	case IDX_MAP_FILE_FOLDER_LISTING:
		if ((STRCASECMP(name, "folder") == 0) || (STRCASECMP(name, "file") == 0)) {
			map_xml_folder_list_start_handler(userdata, name, atts);
		}
		break;

	case IDX_MAP_FILE_MSG_LISTING:
		if (STRCASECMP(name, "msg") == 0) {
			map_xml_message_list_start_handler(user_data, name, atts);
		}
		break;

	case IDX_MAP_FILE_EVENT_OBJECT:
		if (STRCASECMP(name, "event") == 0) {
			map_xml_event_object_start_handler(user_data, name, atts);
		}
		break;
	}

}

static void map_xml_end_handler(void *userdata, const char *name)
{
	MAPXMLFINDDATA *user_data;
	if (!userdata) {
		return;
	}

	user_data = (MAPXMLFINDDATA *) userdata;
	user_data->flag = 1;
}

/*
EV_MAP_GET_SMS_LIST_RESULT
EV_MAP_NEW_SMS_ARRIVED
*/
static void hlp_map_AT_event_indicate(int event, UINT8 result, UINT8 *pdu, UINT16 length)
{
	char param[HLP_EV_MAP_IND_MAX_LEN];
	int offset = 0;
	int len = 0;

	memset(param, 0, sizeof(char) * HLP_EV_MAP_IND_MAX_LEN);

	do {
		len = length - offset;
		len = len > (HLP_EV_MAP_IND_MAX_LEN - 1) ? (HLP_EV_MAP_IND_MAX_LEN - 1) : len;

		if (len > 0) {
			/* FIXME pdu should be textual and not include "\n" */
			memcpy(param, pdu + offset, len);
		}

		hlp_map_event_indication(event, param);
		offset += len;
	} while (offset < length);
}

/*
EV_MAP_CONNECTION_EVENT
EV_MAP_DELETE_SMS_RESULT
EV_MAP_SEND_SMS_RESULT
*/
static void hlp_map_trigger_evt_callback(int event, UINT8 result)
{
	if (s_context && s_context->callback) {
		s_context->callback(event, result, NULL, 0);
	}

	hlp_map_event_indication(event, result);
}

static void map_parser_free(XML_Parser *parser, int type)
{
	MAPXMLFINDDATA * user_data;
	if (!parser) {
		return;
	}

	user_data = XML_GetUserData(parser);

	if (user_data) {
		switch (type) {
		case IDX_MAP_FILE_FOLDER_LISTING: {
			map_folder_list_item_t *obj;
			while ((obj = List_RemoveHead(&user_data->data)) != NULL) {
				hlp_map_free_xml_object(obj, IDX_MAP_FILE_FOLDER_LISTING);
			}
		}
		break;

		case IDX_MAP_FILE_MSG_LISTING: {
			map_message_list_item_t *obj;
			while ((obj = List_RemoveHead(&user_data->data)) != NULL) {
				hlp_map_free_xml_object(obj, IDX_MAP_FILE_MSG_LISTING);
			}
		}
		break;

		case IDX_MAP_FILE_EVENT_OBJECT: {
			map_event_object_t *obj;
			while ((obj = List_RemoveHead(&user_data->data)) != NULL) {
				hlp_map_free_xml_object(obj, IDX_MAP_FILE_EVENT_OBJECT);
			}
		}
		break;

		default:
			break;
		}

		FREE(user_data);
	}

	XML_ParserFree(parser);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
    This function is to free data list number node.
Arguments:
    UINT8* data:[in] Data number node.
    UINT8 type:[in] Data node type.
Return:
    void
---------------------------------------------------------------------------*/
static void hlp_map_free_xml_object(UINT8 *data, UINT8 type)
{
	switch (type) {
	case IDX_MAP_FILE_FOLDER_LISTING: {
		map_folder_list_item_t *folder_obj = (map_folder_list_item_t *) data;

		if (folder_obj != NULL) {
			MAP_FREE(folder_obj->name);
			MAP_FREE(folder_obj->access_time);
			MAP_FREE(folder_obj->create_time);
			MAP_FREE(folder_obj->write_time);
			LFREE(folder_obj);
			break;
		}
	}

	case IDX_MAP_FILE_MSG_LISTING: {
		map_message_list_item_t *msg_list = (map_message_list_item_t *) data;

		if (msg_list != NULL) {
			MAP_FREE(msg_list->date_time);
			MAP_FREE(msg_list->is_protected);
			MAP_FREE(msg_list->is_read);
			MAP_FREE(msg_list->is_text);
			MAP_FREE(msg_list->is_send);
			MAP_FREE(msg_list->msg_priority);
			MAP_FREE(msg_list->msg_hdl);
			MAP_FREE(msg_list->msg_type);
			MAP_FREE(msg_list->reception_status);
			MAP_FREE(msg_list->recipient_addr);
			MAP_FREE(msg_list->recipient_name);
			MAP_FREE(msg_list->replyto_addr);
			MAP_FREE(msg_list->sender_addr);
			MAP_FREE(msg_list->sender_name);
			MAP_FREE(msg_list->subject);
			LFREE(msg_list);
		}

		break;
	}
	case IDX_MAP_FILE_EVENT_OBJECT: {
		map_event_object_t *obj = (map_event_object_t *) data;
		MAP_FREE(obj->evt_type);
		MAP_FREE(obj->folder);
		MAP_FREE(obj->handle);
		MAP_FREE(obj->msg_type);
		MAP_FREE(obj->older_folder);
		LFREE(obj);
	}

	break;

	default:
		break;
	}
}

UINT8 hlp_map_get_status(void)
{
	if (s_context == NULL) {
		return STATE_UNINIT;
	} else {
		return s_context->state;
	}
}

int map_parse(UINT8 *context_handle, tAppMACEvtParameters real_par)
{
	if(s_map_info_parsed) {
		int length = 0;
		int count = 0;
		UINT8 *tempcontont = NULL;
		UINT8 *temp = NULL;
		UINT8 *p = real_par.obj_data->data;

		// get length ,malloc memory;
		tempcontont = strstr(p,"LENGTH:");
		if (map_app_check_type(p, "TYPE:MMS")) { //check msg type is mms
			s_map_mms_msg = TRUE;
		}
		if (map_app_check_type(p, "FOLDER:telecom/msg/INBOX")) {
			s_map_inbox_msg = TRUE;
		}
		if (tempcontont) {
			UINT8 lenth_str[8] = {0};
			temp = strstr(tempcontont,"\r\n")+2;
			memcpy(lenth_str,tempcontont+strlen("LENGTH:"),temp-tempcontont-strlen("LENGTH:"));
			length = atoi(lenth_str);
			if ( length != 0 && temp) {
				s_map_contont = (UINT8 *)malloc(length+1);
				memset( s_map_contont, 0,length+1);
				count = temp - p;
				s_contont_length = length;
			}
		}
		if ( length <(real_par.obj_data->length - count)) {
			memcpy( s_map_contont, temp, length);
			s_map_info_parsed = 1;
		} else {
			memcpy( s_map_contont, temp, real_par.obj_data->length - count);
			s_contont_index = real_par.obj_data->length - count;
			s_map_info_parsed = 0;
		}
	} else {
		UINT8 *p = real_par.obj_data->data;
		UINT8 *tempcontont = NULL;

		// find end,and add it;
		tempcontont = strstr( p, "END:BBODY");
		if ( tempcontont) {
			memcpy(s_map_contont + s_contont_index, p, tempcontont - p);
			s_map_info_parsed = 1;
		}
	}
	if (s_map_info_parsed && s_map_contont) {
		UINT8 *p_end = NULL;
		int len = 0;
		map_message_info_t *p_msg = NULL;
		memset(s_map_contont+s_contont_length-11, 0 ,11);
		if (strlen(s_context->p_vcf_parser->p_pb_item->sNumber) == 0
		    && strlen(s_context->p_vcf_parser->p_pb_item->sName) == 0) {
			UINT8 *p_name = NULL;
			UINT8 *p_number = NULL;

			bse_log_info(TAG, "vcard 3.0 parse failed!!!\r\n");
			p_name = strstr(real_par.obj_data->data, "FN:");
			if (p_name) {
				p_name += 3;
				p_end = strstr(p_name,"\r\n");
				len = (p_end - p_name) < MAX_VCF_NAME_LEN ? (p_end - p_name) : (MAX_VCF_NAME_LEN-1);
				memset(s_context->p_vcf_parser->p_pb_item->sName, 0, MAX_VCF_NAME_LEN);
				memcpy(s_context->p_vcf_parser->p_pb_item->sName, p_name, len);
			}
			p_number = strstr(real_par.obj_data->data, "TEL:");
			if (p_number) {
				p_number += 4;
				p_end = strstr(p_number,"\r\n");
				len = (p_end - p_number) < MAX_VCF_NUMBER_LEN ? (p_end - p_number) : (MAX_VCF_NUMBER_LEN-1);
				memset(s_context->p_vcf_parser->p_pb_item->sNumber, 0, MAX_VCF_NUMBER_LEN);
				memcpy(s_context->p_vcf_parser->p_pb_item->sNumber, p_number, len);
			}
		}
		if((strlen(s_context->p_vcf_parser->p_pb_item->sNumber) > 0)
		   || (strlen(s_context->p_vcf_parser->p_pb_item->sName)  > 0)) {
			strcpy(g_new_msg.message_sender_name, s_context->p_vcf_parser->p_pb_item->sName);
			strcpy(g_new_msg.message_sender_addressing, s_context->p_vcf_parser->p_pb_item->sNumber);
			strcpy(g_new_msg.message_handle,context_handle);
			if (s_map_mms_msg) { //cut useless content for mms
				UINT8 *head = s_map_contont+11;
				UINT8 *p_value = NULL;

				if ((p_value = strstr(head,">\r\n"))) {
					//find last >\r\n, discard photo/media if exist
					p_value += 3;
					p_end = strstr(p_value,"\r\n");
					len = (p_end - p_value) < MAX_MAP_MSG_LEN ? (p_end - p_value) : (MAX_MAP_MSG_LEN-1);
					memset(g_new_msg.message_subject, 0, MAX_MAP_MSG_LEN);
					memcpy(g_new_msg.message_subject, p_value, len);
				} else {
					bse_log_info(TAG, "unkown mms\r\n");
					snprintf(g_new_msg.message_subject, sizeof(g_new_msg.message_subject), "%s", head);
				}

			} else {
				snprintf(g_new_msg.message_subject, sizeof(g_new_msg.message_subject), "%s", s_map_contont+11);
			}

			p_msg = (map_message_info_t *)NewListFind(s_context->msg_list, context_handle, MAX_MAP_HANDLE_LEN);
			if (p_msg) {
				//save the complete message content,because get message list function cannot download whole message content.
				bse_log_info(TAG, "old msg len(%d), current len(%d)\r\n", strlen(p_msg->message_subject), strlen(g_new_msg.message_subject));
				memset(p_msg->message_subject, 0, MAX_MAP_MSG_LEN);
				snprintf(p_msg->message_subject, MAX_MAP_MSG_LEN, "%s", g_new_msg.message_subject);

				int rel_len = strlen(g_new_msg.message_handle) + strlen(g_new_msg.message_sender_name) + strlen(g_new_msg.message_sender_addressing) + strlen(g_new_msg.message_subject);
				int len = rel_len > MAX_VCF_ITEM_LEN?MAX_VCF_ITEM_LEN:rel_len;
				char *msg = (char *)NEW(len + 1);
				snprintf(msg, len, "%s|%s|%s|%s",
				         g_new_msg.message_handle, g_new_msg.message_sender_name, g_new_msg.message_sender_addressing, g_new_msg.message_subject);
				hlp_map_event_indication(EV_MAP_BMSG_DATA_IND, msg);
				//bt_event_indicate(BT_EV_MAP_MESSAGE_INFO, p_msg);
			} else { //if (s_map_inbox_msg && !s_context->get_list_flag)
				//if message from inbox and current don't get list, so consider it is a new message!!!
				int rel_len = strlen(g_new_msg.message_handle) + strlen(g_new_msg.message_sender_name) + strlen(g_new_msg.message_sender_addressing) + strlen(g_new_msg.message_subject);
				int len = rel_len > MAX_VCF_ITEM_LEN?MAX_VCF_ITEM_LEN:rel_len;
				char *new_msg = (char *)NEW(len + 1);
				snprintf(new_msg, len, "%s|%s|%s|%s",
				         g_new_msg.message_handle, g_new_msg.message_sender_name, g_new_msg.message_sender_addressing, g_new_msg.message_subject);
				//bt_event_indicate(BT_EV_MAP_NEW_MESSAGE, new_msg);
				hlp_map_event_indication(EV_MAP_BMSG_DATA_IND, new_msg);
			}
			s_map_inbox_msg = FALSE;
			s_map_mms_msg = FALSE;
			if (s_map_contont) {
				free(s_map_contont);
				s_map_contont = NULL;
			}
		}
	}
	return 1;
}

BOOL map_app_check_type(UINT8 *org_msg, const char *type)
{
	UINT8 is_mms = FALSE;
	if (strstr(org_msg, type)) {
		bse_log_info(TAG, "check msg is mms!!!\r\n");
		is_mms = TRUE;
	}
	return is_mms;
}

void hlp_map_set_msg_list_param(int location, int offset, int counts)
{
	if (s_context) {
		bse_log_info(TAG, "hlp_map_set_msg_list_param (location %d, counts %d)\r\n", location, counts);
		s_context->store_type = location;
		s_context->max_list_counts = counts;
		s_context->offset = offset;
	}
}

UINT8 hlp_map_get_msg_list_location()
{
	if (s_context) {
		return s_context->store_type;
	}

	return 0;
}


void hlp_map_memory_list_init()
{
	if (s_context->msg_list == NULL) {
		s_context->msg_list = BtNewListNew(sizeof(map_message_info_t));
	}
	s_context->get_list_flag = FALSE;
}

void hlp_map_memory_list_done()
{
	if (s_context->msg_list) {
		struct BtList *list = s_context->msg_list;
		NewListRemoveAll(list);
	}
}

UINT8 hlp_map_memory_list_add(map_message_info_t *msg_info)
{
	struct BtList *p_device_list = s_context->msg_list;
	map_message_info_t *pDev = NULL;

	if (msg_info != NULL) {
		pDev = (map_message_info_t *)BtListMemberNew(sizeof(map_message_info_t));

		memcpy(pDev, msg_info, sizeof(map_message_info_t));

		NewListAddTail(p_device_list, (UINT8 *)pDev);
	}
	return 1;
}

#if 0
void map_info_feed(void *param)
{
	map_message_list_item_t *p = (map_message_list_item_t *)param;
	if (p) {
		map_message_info_t msg_info = {0};

		if(p->msg_hdl) memcpy(msg_info.message_handle, p->msg_hdl, strlen(p->msg_hdl));
		if(p->subject) {
			int nlen = strlen(p->subject) >= sizeof(msg_info.message_subject) ? (sizeof(msg_info.message_subject) - 1) : strlen(p->subject);
			memcpy(msg_info.message_subject, p->subject, nlen);
		}
		if(p->date_time) memcpy(msg_info.message_data_time, p->date_time, strlen(p->date_time));
		if(p->sender_name) memcpy(msg_info.message_sender_name, p->sender_name, strlen(p->sender_name));
		if(p->sender_addr) memcpy(msg_info.message_sender_addressing, p->sender_addr, strlen(p->sender_addr));
		if(p->recipient_name) memcpy(msg_info.message_recipient_name, p->recipient_name, strlen(p->recipient_name));
		if(p->recipient_addr) memcpy(msg_info.message_recipient_addressing, p->recipient_addr, strlen(p->recipient_addr));
		if(p->is_read && !strcmp(p->is_read, "yes")) msg_info.message_read = 1;
		if(p->is_send && !strcmp(p->is_send, "yes")) msg_info.message_sent = 1;
		msg_info.message_folder_type = hlp_map_get_msg_list_location();
		//s_bt_manager_data->event_cbk(BT_EV_MAP_MESSAGE_INFO, &msg_info);
		hlp_map_memory_list_add(&msg_info);
	}
}
#endif


#endif /* CONFIG_MAP */

