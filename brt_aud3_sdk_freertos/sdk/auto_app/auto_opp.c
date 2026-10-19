/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/
#include "auto_opp.h"
#include "sysdep.h"
#include "btlist.h"
//#include "fileio.h"
#include "opp_ui.h"

#ifdef CONFIG_OPP
#define TAG "bse_opp"

/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     app_opp.c
 * @Description:
        This file implements the OPP profile interfaces.

 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-04
 ---------------------------------------------------------------------------*/

typedef struct tag_app_opp_conn_instance {
	UINT8               bd[BD_ADDR_LEN];
	UINT8               connected;              /* 0 - connecting; 1 - connected */
	UINT8               is_server;              /* 0 - client; 1 - server */
	void                *inst_handle;
	struct ListStru     pending_push_list;
} app_opp_conn_instance_t;

typedef struct tag_app_opp_client_context {
	UCHAR *fpath;
	UCHAR *spath;
	void  *file_hdl;
	UCHAR first_req; /* mark whether is the first data request. */
	BtUINT64Stru file_size; /* the total size of file to be pushed */
	BtUINT64Stru remain_size; /* the remain size of file currently have not been pushed */
	UINT8       progress; /* percent of currently transferring progress: [0 ~ 100]% */
} app_opp_client_context_t;

/* NOTICE: below type is a union */
typedef union tag_app_opp_client_event_parameter {
	void                        *param;
	UINT8                       *bd;
	UINT16                      *result;
	tOPP_ConnectCompleteStru    *conn_cmp;
	tOPP_DisconnectCompleteStru *disc_cmp;
	tOPP_ObjectDataStru         *obj_data;
	tOPP_GetContinueFlag        *next_op;
	tOPP_Ask4PutRequestStru     *put_req;
} app_opp_client_event_param_t;

typedef struct tag_app_opp_context {
	char save_file_dir[OPP_PATH_MAX_LEN];
	module_event_cbk_t callback;
	/* current only support one OPP connection.
	   But we still use list to keep connections for sake of extension. */
	struct ListStru conn_list;

} app_opp_context_t;

static app_opp_context_t *s_context;

/* proxy callback: it will call the real callback and "bt_event_indicate" */
static void opp_event_callback(int event, long param);
static void opp_connection_event_callback(void *handle, void *context, tOPP_Event_Type event, void *param);
static void opp_push_object_confirm_callback(void *handle, app_opp_client_context_t *context, tOPP_Event_Type event, void *param);
static void opp_push_file_internal(app_opp_client_context_t *context, app_opp_conn_instance_t * conn_inst);
static void opp_update_transfer_progress(app_opp_client_context_t *context);

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
       initialize the OPP module. It will allocate neccessary resources.
 *
 */
/* ---------------------------------------------------------------------------*/
void app_opp_init(module_event_cbk_t event_cbk)
{
	if (s_context == NULL) {
		s_context = (app_opp_context_t *) NEW(sizeof(app_opp_context_t));

		if (!s_context) {
			return;
		}

		memset(s_context, 0, sizeof(app_opp_context_t));
		s_context->callback = event_cbk;
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
       set the path of received files.
 *
 * @Arguments:
        save_path   The path which will keep the received files(the pulled Vcard files
                    and the files be pushed by remote peer).
 */
/* ---------------------------------------------------------------------------*/
void app_opp_set_save_path(const char *save_path)
{
	if (save_path) {
		strncpy(s_context->save_file_dir, save_path, OPP_PATH_MAX_LEN - 1);
		s_context->save_file_dir[OPP_PATH_MAX_LEN - 1] = 0;
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Push a specified file into remote device.
 *
 * @Arguments:
        path    The absolute path of file to be pushed.
 * @Arguments:
        bd      The Bluetooth address of the remote device to which the file be pushed.
 */
/* ---------------------------------------------------------------------------*/
void app_opp_push_file(const char *path, UINT8 *bd)
{
	int err = OPP_RESULT_OK;
	app_opp_client_context_t *context;
	app_opp_conn_instance_t *conn_inst = NULL;

	if (!s_context || !path || !bd) {
		err = OPP_RESULT_ERROR_INVALID_PARAMETER;
		goto ret;
	}

	context = NEW(sizeof(app_opp_client_context_t));
	if (!context) {
		err = OPP_RESULT_ERROR_MEMORY_UNAVAILABLE;
		goto ret;
	}

	context->file_hdl = App_OpenFile(path);

	if (context->file_hdl != NULL) {
		BtUINT64Stru file_length;

		context->fpath = NEW((strlen(path) + 1) * sizeof(UCHAR));
		strcpy(context->fpath, path);
		context->spath = NULL;

		App_GetFileSize(context->file_hdl, &file_length);
		context->file_size.high32 = file_length.high32;
		context->file_size.low32 = file_length.low32;
		context->remain_size.high32 = file_length.high32;
		context->remain_size.low32 = file_length.low32;
		context->progress = 0;
		context->first_req = 1;
	} else {
		FREE(context);
		err = OPP_RESULT_ERROR_NOT_PERMISSION;
		goto ret;
	}

	if (s_context->conn_list.head == s_context->conn_list.tail) {
		conn_inst = s_context->conn_list.head;
	}

	// FIXME if want to support multiple connections, we should traverse the list to find the target connection
	if (!conn_inst) {
		// No connection exists, try to connect.
		OPP_ClientStartReq(context, bd, NULL, opp_connection_event_callback);
	} else {
		opp_push_file_internal(context, conn_inst);
	}

ret:
	if (err != OPP_RESULT_OK) {
		opp_event_callback(EV_OPP_PUSH_COMPLETED, (long) err);
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Pull the Vcard file in a remote device.
 *
 * @Arguments:
        bd      The Bluetooth address of the remote device from which the Vcard be pulled.
 */
/* ---------------------------------------------------------------------------*/
void app_opp_pull_vcard(UINT8 *bd)
{
	// TODO
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Abort an ongoing file transferring procedure.
 */
/* ---------------------------------------------------------------------------*/
void app_opp_abort()
{
	app_opp_conn_instance_t *clnt_inst = (app_opp_conn_instance_t *)s_context->conn_list.head;

	if (clnt_inst != NULL) {
		OPP_AbortReq(clnt_inst->inst_handle);
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
 * @Description:
        Destroy the OPP module. Release resources allocated in initialization function.
 */
/* ---------------------------------------------------------------------------*/
void app_opp_done()
{
	app_opp_conn_instance_t *conn_inst;

	if (s_context) {
		while ((conn_inst = List_RemoveHead(&s_context->conn_list)) != NULL) {
			LFREE(conn_inst);
		}

		FREE(s_context);
		s_context = NULL;
	}
}

void app_opp_server_start(void)
{
	//TODO
}

void app_opp_server_stop(void)
{
	//TODO
}

static void opp_event_callback(int event, long param)
{
	if (s_context && s_context->callback) {
		(s_context->callback)((PROFILE_ID_OPP << 8) | event, (void *) param);

	}
}

static void opp_push_file_internal(app_opp_client_context_t *context, app_opp_conn_instance_t * conn_inst)
{
	/* context and conn_inst SHOULD be valid. */
	tOPP_ObjectInformationStru obj_info;
	UCHAR *file_name;
	UINT32 str_size;

	file_name = App_GetFileName(context->fpath);
	str_size = strlen(file_name) + 1;
	obj_info.name = NEW(sizeof(WCHAR) * str_size);
	ByteToWide(file_name, (UCHAR *)obj_info.name, str_size);
	obj_info.type = NULL;
	obj_info.length.high32 = context->file_size.high32;
	obj_info.length.low32 = context->file_size.low32;

	OPP_PushObjectReq(conn_inst->inst_handle, context, &obj_info, opp_push_object_confirm_callback);
	FREE(obj_info.name);
}

static void opp_connection_event_callback(void *handle, void *context, tOPP_Event_Type event, void *param)
{
	app_opp_client_context_t *opp_context = (app_opp_client_context_t *) context;
	app_opp_conn_instance_t *clnt_inst;
	app_opp_client_event_param_t real_par;
	real_par.param = param;

	switch (event) {
	case OPP_EVT_CLIENT_START_CFM:
		clnt_inst = List_NodeNew(sizeof(app_opp_conn_instance_t));
		memcpy(clnt_inst->bd, param, BD_ADDR_LEN);
		clnt_inst->inst_handle = handle;
		List_AddHead(&s_context->conn_list, clnt_inst); // FIXME directly access global OBJECT "s_context"
		break;

	case OPP_EVT_CONNECT_COMPLETE:
		clnt_inst = LIST_FIND(&s_context->conn_list, app_opp_conn_instance_t *, inst_handle, handle);

		if (real_par.conn_cmp->result == ER_OPP_SUCCESS) {
			clnt_inst->connected = TRUE;

			if (!opp_context) {
				/* This error should not take place */
				opp_event_callback(EV_OPP_PUSH_COMPLETED, (long) OPP_RESULT_ERROR_UNKNOWN);
				return;
			}
			opp_push_file_internal(opp_context, clnt_inst);
		} else {
			if (clnt_inst != NULL) {
				List_RemoveAt(&s_context->conn_list, clnt_inst);
				LFREE(clnt_inst);

				if (opp_context) {
					App_CloseFile(opp_context->file_hdl);
					FREE(opp_context->fpath);
					FREE(opp_context->spath);
					FREE(opp_context);
				}
				opp_event_callback(EV_OPP_PUSH_COMPLETED, (long) OPP_RESULT_ERROR_CONNECT_FAILED);
			}
		}
		break;

	case OPP_EVT_DISCONNECT_COMPLETE:
		clnt_inst = LIST_FIND(&s_context->conn_list, app_opp_conn_instance_t *, inst_handle, handle);

		if (clnt_inst != NULL) {
			List_RemoveAt(&s_context->conn_list, clnt_inst);
			LFREE(clnt_inst);
		}
		break;
	case OPP_EVT_ASK4_GET_REQUEST_IND:
		break;
	case OPP_EVT_OBJECT_DATA_IND:
		break;
	case OPP_EVT_PULL_VCARD_COMPLETE:
		break;
	case OPP_EVT_ASK4_PUT_REQUEST_IND:
		break;
	case OPP_EVT_ASK4_OBJECT_DATA_IND:
		break;
	case OPP_EVT_PUSH_OBJECT_COMPLETE:
		break;
	default:
		break;
	}
}

static void opp_push_object_confirm_callback(void *handle, app_opp_client_context_t *context, tOPP_Event_Type event, void *param)
{
	app_opp_client_event_param_t real_par;
	app_opp_conn_instance_t *clnt_inst = NULL;

	real_par.param = param;
	switch (event) {
	case OPP_EVT_ASK4_PUT_REQUEST_IND:
		if (context->first_req) {
			context->first_req = 0;
			opp_event_callback(EV_OPP_PUSH_STARTED, 0);
		}

		if ((context->remain_size.high32 == 0) && (context->remain_size.low32 < real_par.put_req->max_data_size)) {
			real_par.put_req->max_data_size = context->remain_size.low32;
		}
#ifdef CONFIG_MEMORY_STATIC
		if (real_par.put_req->max_data_size > CONFIG_OPP_SEND_MOPL) {
			real_par.put_req->max_data_size = CONFIG_OPP_SEND_MOPL;
		}
#endif
		/* else, keep real_par.put_req->max_data_size unchanged */
		{
			real_par.put_req->next_op = OPP_OP_PENDING;
			OPP_PutContinueReq(handle, real_par.put_req->max_data_size);
		}
		break;

	case OPP_EVT_ASK4_OBJECT_DATA_IND:
		opp_update_transfer_progress(context);
		real_par.obj_data->length = (UINT16)App_ReadFile(context->file_hdl, real_par.obj_data->data, real_par.obj_data->length);

		if ((context->remain_size.high32 != 0) || (real_par.obj_data->length < context->remain_size.low32)) {
			if (context->remain_size.low32 >= real_par.obj_data->length) {
				context->remain_size.low32 -= real_par.obj_data->length;
			} else {
				context->remain_size.high32--;
				context->remain_size.low32 += (0xFFFFFFFF - real_par.obj_data->length) + 1;
			}
		} else {
			context->remain_size.low32 = 0;
			real_par.obj_data->is_final = OPP_DATA_COMPLETE;
		}

		break;

	case OPP_EVT_PUSH_OBJECT_COMPLETE: {
		UINT16 result;
		App_CloseFile(context->file_hdl);
		FREE(context->fpath);
		FREE(context->spath);
		FREE(context);

		result = (real_par.result ? *(real_par.result) : 0);
		opp_event_callback(EV_OPP_PUSH_COMPLETED, result);

		// FIXME disconnect the OPP connection.
		if (s_context->conn_list.head == s_context->conn_list.tail) {
			clnt_inst = s_context->conn_list.head;
			if (clnt_inst != NULL) {
				OPP_DisconnectReq(clnt_inst->inst_handle);
			}
		}
	}
	break;

	default:
		break;
	}
}

static void opp_update_transfer_progress(app_opp_client_context_t *context)
{
	long t = context->file_size.low32;
	long r = context->remain_size.low32;
	UINT8 p;

	if (t <= 0) return;

	p = (t - r) * 100 / t;
	if (p != context->progress) {
		context->progress = p;
		opp_event_callback(EV_OPP_PUSH_IN_PROGRESS, (long) context->progress);
	}
}

profile_t g_opp_profile = {
	app_opp_init,
	app_opp_done,
	app_opp_server_start,
	app_opp_server_stop,
};

#endif

