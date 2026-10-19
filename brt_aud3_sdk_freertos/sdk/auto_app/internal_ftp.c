#include "internal_ftp.h"

#define TAG "bse_ftp"
#ifdef CONFIG_FTP
static struct FTPUserDataStru *s_ftp_user_data = NULL;
static struct FsmTimer *s_timer_notify_state = NULL;
static UINT8 s_get_folder_size = 0;

void event_ftp_indication(UINT8 event, void *param)
{
	if (FTP_USER_DATA != NULL) {
		if (s_ftp_user_data->event_cbk != NULL) {
			s_ftp_user_data->event_cbk((PROFILE_ID_FTP << 8) | event, param);
		}
	}
}

static void hlp_ftp_display_folder(void);
static void hlp_ftp_push_file(tAppClntContextStru *context);
static void hlp_ftp_push_first_object(tAppClntContextStru *context);
static void hlp_ftp_push_next_object(tAppClntContextStru *context, UINT32 prompt);
static void hlp_ftp_free_push_context(tAppClntContextStru *context);
static void hlp_ftp_pull_first_object(tAppClntContextStru *context);
static void hlp_ftp_pull_next_object(tAppClntContextStru *context, UINT32 prompt);
static void hlp_ftp_pull_file_internal(tAppClntContextStru *context);
static void hlp_ftp_pull_folder_internal(tAppClntContextStru *context);
static void hlp_ftp_free_pull_context(tAppClntContextStru *context);
static void hlp_ftp_get_remote_folder_size_cbk(void *handle, void *context, tFTP_Event_Type event, void *param);
static void hlp_ftp_change_folder_down_cbk(void *handle, void *context, tFTP_Event_Type event, UINT16 *result);
static void hlp_ftp_change_folder_up_cbk(void *handle, void *context, tFTP_Event_Type event, UINT16 *result);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Create a new directory.
------------------------------------------------------------------------------------*/
int APP_CreateFolderW(WCHAR *path)
{
	UCHAR file_path[MAX_FTP_PATH_LEN];
	int f_len = 0;
	int r = 0;

	memset(file_path, 0, sizeof(file_path));
	WideToByte(path, file_path, 0);

	r = mkdir(file_path, 0776);

	if ((r!=0) && (errno==EEXIST) && (App_IsFolderW(path))) {
		r = 0;
	}

	return r;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Convert time.
------------------------------------------------------------------------------------*/
void App_ConvertFileTime(struct tm *file_time, tApp_FileTimeStru *app_time)
{
	app_time->year = (UINT16)(file_time->tm_year+1900);
	app_time->month = (UINT8)file_time->tm_mon;
	app_time->day = (UINT8)file_time->tm_mday;
	app_time->hour = (UINT8)file_time->tm_hour;
	app_time->minute = (UINT8)file_time->tm_min;
	app_time->second = (UINT8)file_time->tm_sec;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Convert data.
------------------------------------------------------------------------------------*/
void App_ConverFileDataW(struct stat *sys_data, tApp_FileDataStru *app_data, char* file_path)
{
	bse_log_api(TAG, "App_ConverFileDataW Find file %s\r\n", file_path);//temp test for debug
	char * file_name = App_GetFileName(file_path);
	BtUINT64Stru file_length;
	FILE * fd = NULL;
	fd = App_OpenFile(file_path);

	if (fd == NULL) {
		bse_log_error(TAG, "App_ConverFileDataW file path is invaild.\r\n");
		return;
	}

	App_GetFileSize(fd , &file_length);
	app_data->file_size_high = file_length.high32;
	app_data->file_size_low = file_length.low32;

	App_CloseFile(fd);
	fd = NULL;

	app_data->file_name = NEW((strlen(file_name) + 1) * sizeof(WCHAR));
	memset(app_data->file_name, 0, (strlen(file_name) + 1) * sizeof(WCHAR));
	ByteToWide(file_name, app_data->file_name, strlen(file_name)+1);
	App_ConvertFileTime(&sys_data->st_mtime, &app_data->created_time);//there is not creat time in LINUX OS.
	App_ConvertFileTime(&sys_data->st_mtime, &app_data->modified_time);
	App_ConvertFileTime(&sys_data->st_atime, &app_data->accessed_time);
	app_data->attributes = FTP_OBJECT_ATTRIBUTE_USER_PERM_VALID | FTP_OBJECT_ATTRIBUTE_USER_PERM_READ;
	if ((sys_data->st_mode&S_IFMT)&S_IFDIR) {
		app_data->attributes |= FTP_OBJECT_ATTRIBUTE_FOLDER;
	}
	if (((sys_data->st_mode& S_IREAD) && (sys_data->st_mode& S_IWRITE))) {
		app_data->attributes |= (FTP_OBJECT_ATTRIBUTE_USER_PERM_WRITE|FTP_OBJECT_ATTRIBUTE_USER_PERM_DELETE);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Continues a file search from a previous call to the APP_FindFirstFile function.
0 - Succeed.
others, error code.
------------------------------------------------------------------------------------*/
int App_FindNextFileW(DIR * hdl, tApp_FileDataStru *file_data, const WCHAR *path)
{
	struct stat data_info;
	int r = -1;
	char  file_path[MAX_FILE_PATH_LEN];
	struct dirent* ent=NULL;
	int f_len = 0;

	memset(file_path, 0, sizeof(file_path));
	memset(&data_info, 0, sizeof(data_info));
	WideToByte(path, file_path, 0);

	while ((ent=readdir(hdl)) != NULL) {
		if ((strcmp(ent->d_name, ".")==0) || (strcmp(ent->d_name, "..")==0)) {
			continue;
		}

		if ((strlen(ent->d_name)+strlen(file_path))>(MAX_FTP_PATH_LEN-2)) {
			continue;
		}

		strcat(file_path, "/");
		strcat(file_path, ent->d_name);
		file_path[strlen(file_path)+1] = 0;

		if (stat(file_path, &data_info) == -1) {
			continue;
		}

		App_ConverFileDataW(&data_info, file_data, file_path);

		r = 0;
		break;
	}
	return r;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Searches a directory for a file or a sub-folder.
------------------------------------------------------------------------------------*/
DIR* App_FindFirstFileW(const WCHAR *path, tApp_FileDataStru *file_data)
{
	struct stat data_info;
	DIR* pDir = NULL;
	struct dirent* ent=NULL;
	UCHAR file_path[MAX_FTP_PATH_LEN];
	int f_len = 0;

	memset(file_path, 0, sizeof(file_path));
	WideToByte(path, file_path, 0);

	bse_log_api(TAG, "App_FindFirstFileW file_path: %s", file_path);

	if ((pDir=opendir(file_path) )!= NULL) {
		while ((ent=readdir(pDir)) != NULL) {
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0) {
				continue;
			}

			if ((strlen(ent->d_name)+strlen(file_path))>(MAX_FTP_PATH_LEN-2)) {
				continue;
			}

			strcat(file_path, "/");
			strcat(file_path, ent->d_name);

			if (stat(file_path, &data_info) == -1) {
				continue;
			}

			App_ConverFileDataW(&data_info, file_data, file_path);

			return pDir;
		}
		pDir = NULL;
	}
	return pDir;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Closes the specified search handle.
------------------------------------------------------------------------------------*/
void App_FindFileClose(DIR* hdl)
{
	closedir(hdl);
	hdl = NULL;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Append the file or sub-folder name to the current path.
------------------------------------------------------------------------------------*/
static void hlp_ftp_down_path(tAppClntContextStru *context)
{
	if (context == NULL) {
		return;
	}

	App_DownPath(&context->fpath, &context->fpath_size, context->file_data->file_name);
	FREE(context->file_data->file_name);
	context->file_data->file_name = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Convert file data.
------------------------------------------------------------------------------------*/
static void hlp_ftp_convert_string_time(UCHAR *time_str, tApp_FileTimeStru *file_time)
{
	if (time_str != NULL) {
		file_time->year = (time_str[0] - '0') * 1000 + (time_str[1] - '0') * 100 + (time_str[2] - '0') * 10 + (time_str[3] - '0');
		file_time->month = (time_str[4] - '0') * 10 + (time_str[5] - '0');
		file_time->day = (time_str[6] - '0') * 10 + (time_str[7] - '0');
		file_time->hour = (time_str[9] - '0') * 10 + (time_str[10] - '0');
		file_time->minute = (time_str[11] - '0') * 10 + (time_str[12] - '0');
		file_time->second = (time_str[13] - '0') * 10 + (time_str[14] - '0');
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Convert file data.
------------------------------------------------------------------------------------*/
static void hlp_ftp_convert_file_data(tFTP_ObjectInformationStru *src, tApp_FileDataStru *dest)
{
	int slen;
	if (src->name != NULL) {
		slen = strlen(src->name) + 1;
		dest->file_name = NEW(slen * sizeof(WCHAR));
		memset(dest->file_name, 0, slen * sizeof(WCHAR));
		UTF8ToWide(src->name, (UCHAR *)dest->file_name, slen);
	} else {
		dest->file_name = NULL;
	}
	dest->attributes = src->attributes;
	dest->file_size_high = src->length.high32;
	dest->file_size_low = src->length.low32;
	hlp_ftp_convert_string_time(src->modified_time, &dest->modified_time);
	hlp_ftp_convert_string_time(src->created_time, &dest->created_time);
	hlp_ftp_convert_string_time(src->accessed_time, &dest->accessed_time);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Update transfer progress.
                    flag  1: use in push object
					      2: use in pull object
------------------------------------------------------------------------------------*/
static void ftp_update_transfer_progress(tAppClntContextStru *context, int flag)
{
	UINT32 t;
	UINT32 r;
	UINT8 p;

	if (context == NULL) {
		return;
	}

	if (flag == 1) {
		if (context->folder_size.high32 > 0) { //remote object MAX size is 16G.
			t = context->folder_size.low32>>2 + context->folder_size.high32<<30;
			r = context->remain_size.low32>>2 + context->remain_size.high32<<30;
		} else {
			t = context->folder_size.low32;
			r = context->remain_size.low32;
		}
	} else {
		if (context->file_size.high32 > 0) { //remote object MAX size is 16G.
			t = context->file_size.low32>>2+(context->file_size.high32<<30);
			r = context->remain_size.low32>>2+(context->remain_size.high32<<30);
		} else {
			t = context->file_size.low32;
			r = context->remain_size.low32;
		}
	}

	if (t == 0) {
		context->progress = 100;
		return;
	} else if (t>0 && ((t-r)<(0xFFFFFFFF/100))) {
		p = (t-r)*100/t;
	} else {
		p = (t-r)/(t/100);
	}

	if (p != context->progress) {
		context->progress = p;
	}
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Connection events.
------------------------------------------------------------------------------------*/
static void hlp_ftp_connection_cbk(void *handle, void *context, tFTP_Event_Type event, void *param)
{
	bse_log_api(TAG, "hlp_ftp_connection_cbk event: %d", event);

	tAppClntEvtParameters real_par;
	tApp_FileDataStru *item;
	real_par.param = param;
	switch (event) {
	case FTP_EVT_CLIENT_START_CFM:
		if (FTP_USER_DATA) {
			memcpy(FTP_USER_DATA->bd, param, BDLENGTH);
			FTP_USER_DATA->ftp_handle= handle;
			FTP_USER_DATA->fpath = NEW(16 * sizeof(WCHAR));/* initial is root "\\" */
			memset(FTP_USER_DATA->fpath, 0, 16 * sizeof(WCHAR));
			FTP_USER_DATA->fpath[0] = OS_PATH_SEPARATOR;
			FTP_USER_DATA->fpath_size = 16;
			if (FTP_USER_DATA->l_path == NULL) {
				FTP_USER_DATA->l_path = NEW(strlen("sdcard/Download/")+1);
				memset(FTP_USER_DATA->l_path, 0, strlen("sdcard/Download/")+1);
				strncpy(FTP_USER_DATA->l_path, "sdcard/Download/", strlen("sdcard/Download/"));
			}
		}
		break;

	case FTP_EVT_CONNECT_COMPLETE:
		if (real_par.conn_cmp->result == ER_FTP_SUCCESS) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
			event_ftp_indication(EV_FTP_CONNECTED, real_par.conn_cmp->bd);
		} else {
			hlp_ftp_set_status(ST_FTP_READY);
			FTP_USER_DATA->ftp_handle = NULL;
			event_ftp_indication(EV_FTP_DISCONNECTED, real_par.conn_cmp->bd);
		}
		break;

	case FTP_EVT_DISCONNECT_COMPLETE:
		if (FTP_USER_DATA) {
			FREE(FTP_USER_DATA->fpath);
			FTP_USER_DATA->fpath = NULL;
			item = List_RemoveHead(&FTP_USER_DATA->folder_list);
			while (item != NULL) {
				FREE(item->file_name);
				LFREE(item);
				item = List_RemoveHead(&FTP_USER_DATA->folder_list);
			}
			if (FTP_USER_DATA->l_path != NULL) {
				FREE(FTP_USER_DATA->l_path);
				FTP_USER_DATA->l_path = NULL;
			}
		}
		bse_log_info(TAG, "> Connection with FTS is disconnected, reason is %04X!",  real_par.conn_cmp->result);
		hlp_ftp_set_status(ST_FTP_READY);
		event_ftp_indication(EV_FTP_DISCONNECTED, real_par.conn_cmp->bd);
		break;
	default:
		break;

	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Folder Listing object and Browse Folder complete event.
------------------------------------------------------------------------------------*/
static void hlp_ftp_browse_folder_cfm_cbk(void *handle, void *context, tFTP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;
	tApp_FileDataStru *folder_item;
	struct FTPUserDataStru * ftp_user;

	ftp_user = (struct FTPUserDataStru *)context;
	real_par.param = param;
	switch (event) {
	case FTP_EVT_ASK4_GET_REQUEST_IND:
#if 0
		/* Test pending */
		real_par.next_op = FTP_OP_PENDING;
		FTP_GetContinueReq(handle);
#endif
		break;
	case FTP_EVT_OBJECT_INFORMATION_IND: {
		folder_item = List_NodeNew(sizeof(tApp_FileDataStru));
		hlp_ftp_convert_file_data(real_par.obj_info, folder_item);
		if (FTP_USER_DATA) {
			List_AddTail(&FTP_USER_DATA->folder_list, folder_item);
		}
	}
	break;
	case FTP_EVT_PULL_OBJECT_COMPLETE: {
		hlp_ftp_set_status(ST_FTP_CONNECTED);
		if (ftp_user == NULL) {
			return;
		}
		ftp_user->op_context = NULL;
		if (*real_par.result == 0) {
			hlp_ftp_display_folder();
		} else {
			event_ftp_indication(EV_FTP_DISPLAY_COMPLETE, *real_par.result);
		}
	}
	break;
	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Navigate to sub folder complete event.
------------------------------------------------------------------------------------*/
static void hlp_ftp_down_folder_cfm(void *handle, void *context, tFTP_Event_Type event, UINT16 *result)
{
	if (*result == ER_FTP_SUCCESS) {
		if (FTP_USER_DATA != NULL) {
			if (FTP_USER_DATA->ftp_handle == handle) {
				FTP_USER_DATA->op_context = NULL;
				if (context != NULL) {
					App_DownPath(&FTP_USER_DATA->fpath, &FTP_USER_DATA->fpath_size, context);
				}
			}
		}
	}
	event_ftp_indication(EV_FTP_CHANGE_FOLDER_COMPLETE, *result);
	if (hlp_ftp_get_status()==ST_FTP_CHANGING) {
		hlp_ftp_set_status(ST_FTP_CONNECTED);
	}
	FREE(context);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Navigate to up folder complete event.
------------------------------------------------------------------------------------*/
static void hlp_ftp_up_folder_cfm(void *handle, void *context, tFTP_Event_Type event, UINT16 *result)
{
	if (*result == ER_FTP_SUCCESS) {
		if (FTP_USER_DATA!= NULL) {
			if (FTP_USER_DATA->ftp_handle == handle) {
				FTP_USER_DATA->op_context = NULL;
				App_UpPath(FTP_USER_DATA->fpath);
			}
		}
	}
	event_ftp_indication(EV_FTP_CHANGE_FOLDER_COMPLETE, *result);
	if (hlp_ftp_get_status()==ST_FTP_CHANGING) {
		hlp_ftp_set_status(ST_FTP_CONNECTED);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Back to root complete event.
------------------------------------------------------------------------------------*/
static void hlp_ftp_set_root_cfm(void *handle, void *context, tFTP_Event_Type event, UINT16 *result)
{
	if (*result == ER_FTP_SUCCESS) {
		if (FTP_USER_DATA != NULL) {
			if (FTP_USER_DATA->ftp_handle == handle) {
				FTP_USER_DATA->op_context = NULL;
				memset(FTP_USER_DATA->fpath, 0, FTP_USER_DATA->fpath_size * sizeof(WCHAR));
				FTP_USER_DATA->fpath[0] = '/';
			}
		}
	}
	event_ftp_indication(EV_FTP_CHANGE_FOLDER_COMPLETE, *result);
	if (hlp_ftp_get_status()==ST_FTP_CHANGING) {
		hlp_ftp_set_status(ST_FTP_CONNECTED);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Set parent folder complete event - Pushing folder
------------------------------------------------------------------------------------*/
static void hlp_ftp_up_folder_cfm_push(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, UINT16 *result)
{
	if (context == NULL) {
		return;
	}

	if (*result == ER_FTP_SUCCESS) {
		hlp_ftp_push_next_object(context, 1);
	} else {
		/* Failed to back to parent folder. */
		hlp_ftp_free_push_context(context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Down (Create) folder complete event - Pushing folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_down_folder_cfm_push(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, UINT16 *result)
{
	if (context == NULL) {
		return;
	}

	if (*result == ER_FTP_SUCCESS) {
		hlp_ftp_push_first_object(context);
	} else {
		/* Finish pushing current folder, navigate local to the parent. */
		if (*result != ER_FTP_NO_CONNECTION) {
			hlp_ftp_push_next_object(context, 0);
		} else {
			/* Link disconnected during folder push */
			hlp_ftp_free_push_context(context);
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Set parent folder complete event - Pushing folder
------------------------------------------------------------------------------------*/
static void hlp_ftp_folder_cfm_push(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, UINT16 *result)
{
	if (context == NULL) {
		return;
	}

	if (*result == ER_FTP_SUCCESS) {
		hlp_ftp_push_next_object(context, 1);
	} else {
		/* Failed to back to parent folder. */
		hlp_ftp_free_push_context(context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Object data indication and Push File complete event
------------------------------------------------------------------------------------*/
static void hlp_ftp_push_file_cfm_cbk(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;

	tProgressStru pg;

	real_par.param = param;
	switch (event) {
	case FTP_EVT_ASK4_PUT_REQUEST_IND:
#if 0
		if (FTP_USER_DATA->first_push_req == 1) {
			FTP_USER_DATA->first_push_req = 0;
			event_ftp_indication(EV_FTP_PUSH_OBJECT_STATED, NULL);
		}
#endif
		if (context == NULL)
			return;
		if ((context->file_size.high32 == 0) && (context->file_size.low32 < real_par.put_req->max_data_size)) {
			real_par.put_req->max_data_size = context->file_size.low32;
		}
		/* else, keep real_par.put_req->max_data_size unchanged */
#if 0
		/* Test pending */
		real_par.put_req->next_op = FTP_OP_PENDING;
		FTP_PutContinueReq(handle, real_par.put_req->max_data_size);
#endif
		break;

	case FTP_EVT_ASK4_OBJECT_DATA_IND:
		if (context == NULL)
			return;
		real_par.obj_data->length = (UINT16)App_ReadFile(context->file_hdl, real_par.obj_data->data, real_par.obj_data->length);
		if ((context->remain_size.high32 > 0) || (real_par.obj_data->length <= context->remain_size.low32)) {
			if (context->remain_size.low32 >= real_par.obj_data->length) {
				context->remain_size.low32 -= real_par.obj_data->length;
			} else {
				context->remain_size.high32--;
				context->remain_size.low32 += (0xFFFFFFFF - real_par.obj_data->length);
			}
		} else {
			context->remain_size.low32 = 0;
		}
		if ((context->file_size.high32 > 0) || (real_par.obj_data->length <= context->file_size.low32)) {
			if (context->file_size.low32 >= real_par.obj_data->length) {
				context->file_size.low32 -= real_par.obj_data->length;
			} else {
				context->file_size.high32-- ;
				context->file_size.low32 += (0xFFFFFFFF - real_par.obj_data->length);
			}
		} else {
			context->file_size.low32 = 0;
			real_par.obj_data->is_final = FTP_DATA_COMPLETE;
		}
		//ftp_update_transfer_progress(context, 1);
		pg.obj_name = NEW(strlen(context->obj_name)+1);
		memset(pg.obj_name, 0, strlen(context->obj_name)+1);
		strncpy(pg.obj_name, context->obj_name, strlen(context->obj_name));
		pg.cur_size = context->folder_size.low32-context->remain_size.low32;
		pg.obj_size = context->folder_size.low32;
		event_ftp_indication(EV_FTP_PUSH_IN_PROGRESS, &pg);

		FREE(pg.obj_name);
		break;

	case FTP_EVT_PUSH_OBJECT_COMPLETE:
		if (*real_par.result != ER_FTP_SUCCESS) {
			if(hlp_ftp_get_status() == ST_FTP_PUSHING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
		}
		if (*real_par.result != ER_FTP_NO_CONNECTION) {
			if (context == NULL)
				return;
			hlp_ftp_push_next_object(context, *real_par.result == ER_FTP_SUCCESS);
		} else {
			if(hlp_ftp_get_status() == ST_FTP_PUSHING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			hlp_ftp_free_push_context(context);
		}
		break;
	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Folder Listing object and Browse Folder complete event.
------------------------------------------------------------------------------------*/
static void hlp_ftp_browse_folder_cfm_pull(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;
	tAppRmtFolderListStru *items;
	WCHAR *file_name;
	int str_len;

	real_par.param = param;
	switch (event) {
	case FTP_EVT_ASK4_GET_REQUEST_IND:
		break;
	case FTP_EVT_OBJECT_INFORMATION_IND:
		if (context == NULL)
			return;
		items = context->sub_list.tail;
		if (!(real_par.obj_info->attributes & FTP_OBJECT_ATTRIBUTE_PARENT)) {
#if 0
			if (real_par.obj_info->length.low32 >= 0) {
				context->file_size.low32=real_par.obj_info->length.low32;
				context->file_size.high32=real_par.obj_info->length.high32;
			}
#endif
			str_len = strlen(real_par.obj_info->name) + 1;
			file_name = List_NodeNew(str_len * sizeof(WCHAR));
			UTF8ToWide(real_par.obj_info->name, (UCHAR *)file_name, str_len);
			if (real_par.obj_info->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
				if (items->folder_start == NULL) {
					items->folder_start = file_name;
				}
				List_AddTail(&items->sub_list, file_name);
			} else {
				List_InsertAt(&items->sub_list, items->folder_start, file_name);
			}
		}
		break;
	case FTP_EVT_PULL_OBJECT_COMPLETE:
		if (*real_par.result != ER_FTP_SUCCESS) {
			if(hlp_ftp_get_status()==ST_FTP_PULLING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
		}
		if (*real_par.result != ER_FTP_NO_CONNECTION) {
			/* Try pulling other items if not disconnected */
			if (context == NULL)
				return;
			hlp_ftp_pull_first_object(context);
		} else {
			if(hlp_ftp_get_status()==ST_FTP_PULLING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			hlp_ftp_free_pull_context(context);
		}
		break;
	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Set parent folder complete event - Pulling folder
------------------------------------------------------------------------------------*/
static void hlp_ftp_up_folder_cfm_pull(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, UINT16 *result)
{
	if (context == NULL) {
		return;
	}

	if (*result == ER_FTP_SUCCESS) {
		hlp_ftp_pull_next_object(context, 1);
	} else {
		/* Failed to back to parent folder. */
		hlp_ftp_free_pull_context(context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Down (Create) folder complete event - Pulling folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_down_folder_cfm_pull(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, UINT16 *result)
{
	if (context == NULL) {
		return;
	}

	if (*result == ER_FTP_SUCCESS) {
		if (!context->abort) {
			tAppRmtFolderListStru *items = List_NodeNew(sizeof(tAppRmtFolderListStru));
			List_AddTail(&context->sub_list, items);
			FTP_BrowseFolderReq(handle, context, hlp_ftp_browse_folder_cfm_pull);
		} else {
			/* Folder pull is aborted, need navigate back to the starting folder */
			FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_up_folder_cfm_pull);
		}
	} else {
		/* Pulling next item in the same folder. */
		if (*result != ER_FTP_NO_CONNECTION) {
			hlp_ftp_pull_next_object(context, 1);
		} else {
			/* Link disconnected during folder pull */
			hlp_ftp_free_pull_context(context);
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Object data indication and Pull File complete event
------------------------------------------------------------------------------------*/
static void hlp_ftp_pull_file_cfm_cbk(void *handle, tAppClntContextStru *context, tFTP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;
	int wr_size = 0;
	tProgressStru pg;

	real_par.param = param;
	switch (event) {
	case FTP_EVT_OBJECT_DATA_IND:
		if (real_par.obj_data->data != NULL) {
			if (context == NULL)
				return;
			wr_size = App_WriteFile(context->file_hdl, real_par.obj_data->data, real_par.obj_data->length);
			if ((context->remain_size.low32-wr_size) < 0) {
				if (context->remain_size.high32 > 0) {
					context->remain_size.high32--;
					context->remain_size.low32 = context->remain_size.low32-wr_size+0xFFFFFFFF;
				} else {
					context->remain_size.low32 = 0;
				}
			} else {
				context->remain_size.low32 -= wr_size;
			}
			//	ftp_update_transfer_progress(context, 2);
			pg.obj_name = NEW(strlen(context->obj_name)+1);
			memset(pg.obj_name, 0, strlen(context->obj_name)+1);
			strncpy(pg.obj_name, context->obj_name, strlen(context->obj_name));
			pg.cur_size = context->file_size.low32-context->remain_size.low32;
			pg.obj_size = context->file_size.low32;
			event_ftp_indication(EV_FTP_PULL_IN_PROGRESS, &pg);
			FREE(pg.obj_name);
		}
		break;
	case FTP_EVT_ASK4_GET_REQUEST_IND:
		if (FTP_USER_DATA) {
			if (FTP_USER_DATA->first_pull_req) {
				FTP_USER_DATA->first_pull_req = 0;
				event_ftp_indication(EV_FTP_PULL_OBJECT_STATED, 0);
			}
		}
#if 0
		/* Test pending */
		real_par.next_op = FTP_OP_PENDING;
		FTP_GetContinueReq(handle);
#endif
		break;
	case FTP_EVT_PULL_OBJECT_COMPLETE:
		if (*real_par.result != ER_FTP_SUCCESS) {
#if 0
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, *real_par.result);
			if (hlp_ftp_get_status() == ST_FTP_PULLING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
#endif
		}
		if (*real_par.result != ER_FTP_NO_CONNECTION) {
			if (context == NULL)
				return;
			hlp_ftp_pull_next_object(context, *real_par.result == ER_FTP_SUCCESS);
		} else {
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, *real_par.result);
			if (hlp_ftp_get_status() == ST_FTP_PULLING) {
				hlp_ftp_set_status(ST_FTP_READY);
			}
			hlp_ftp_free_pull_context(context);
		}
		break;
	default:
		break;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTP application event callback - Delete Object complete event
------------------------------------------------------------------------------------*/
static void hlp_ftp_delete_object_cfm_cbk(void *handle, WCHAR *fname, tFTP_Event_Type event, UINT16 *result)
{
	bse_log_api(TAG, "hlp_ftp_delete_object_cfm_cbk event: %d, result: %d", event, *result);

	/* update local stored folder items */
	tApp_FileDataStru *folder_item;

	if (FTP_USER_DATA!= NULL) {
		if (FTP_USER_DATA->ftp_handle == handle) {
			FTP_USER_DATA->op_context = NULL;
			if (*result == ER_FTP_SUCCESS) {
				folder_item = FTP_USER_DATA->folder_list.head;
				while (folder_item != NULL) {
					if (wcscmp(folder_item->file_name, fname) == 0) {
						List_RemoveAt(&FTP_USER_DATA->folder_list, folder_item);
						FREE(folder_item->file_name);
						LFREE(folder_item);
						break;
					}
					folder_item = LNEXT(folder_item);
				}
			}
		}
	}
	event_ftp_indication(EV_FTP_DELETE_OBJECT_COMPLETE, *result);
	if (hlp_ftp_get_status()==ST_FTP_DELETING) {
		hlp_ftp_set_status(ST_FTP_CONNECTED);
	}
	FREE(fname);
}

/* Internal functions for process event of hfp and application */
void hlp_ftp_init(module_event_cbk_t p_event_cbk)
{
	s_ftp_user_data = (struct FTPUserDataStru *)NEW(sizeof(struct FTPUserDataStru));
	memset(s_ftp_user_data, 0, sizeof(struct FTPUserDataStru));
	s_ftp_user_data->l_path = NULL;
	s_ftp_user_data->fpath = NULL;
	s_ftp_user_data->event_cbk = p_event_cbk;
}

void hlp_ftp_done(void)
{
	tApp_FileDataStru *item = NULL;

	if (s_ftp_user_data) {
		if (s_ftp_user_data->op_context) {
			if (hlp_ftp_get_status() == ST_FTP_PUSHING) {
				hlp_ftp_free_push_context(s_ftp_user_data->op_context);
			} else if (hlp_ftp_get_status() == ST_FTP_PULLING) {
				hlp_ftp_free_pull_context(s_ftp_user_data->op_context);
			}

			s_ftp_user_data->op_context = NULL;
		}

		item = List_RemoveHead(&FTP_USER_DATA->folder_list);
		while (item != NULL) {
			FREE(item->file_name);
			LFREE(item);
			item = List_RemoveHead(&FTP_USER_DATA->folder_list);
		}

		if (s_ftp_user_data->fpath) {
			FREE(s_ftp_user_data->fpath);
		}

		if (s_ftp_user_data->l_path) {
			FREE(s_ftp_user_data->l_path);
		}
	}

	FREE(s_ftp_user_data);
	s_ftp_user_data = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Convert file data.
------------------------------------------------------------------------------------*/
static void hlp_ftp_display_folder(void)
{
	int wfile_name = 0;
	long file_size = 0;
	char name[MAX_FTP_PATH_LEN];
	tApp_FileDataStru *item = NULL;
	if (FTP_USER_DATA) {
		item = FTP_USER_DATA->folder_list.head;
	} else {
		event_ftp_indication(EV_FTP_DISPLAY_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}
	int file_count = 0, folder_count = 0;
	char  object[MAX_FTP_OBJECT_LEN];// object_attr:2 object_size: 8 time 14, name_len: 2 , path:MAX_FTP_PATH_LEN,
	unsigned char object_attr;
	int name_len;

	while (item != NULL) {
		memset(object, 0, sizeof(object));
		object_attr = 0;
		if (item->attributes & FTP_OBJECT_ATTRIBUTE_PARENT) {
			folder_count++;
		} else {
			if (item->attributes & FTP_OBJECT_ATTRIBUTE_USER_PERM_VALID) {
				if (item->attributes & FTP_OBJECT_ATTRIBUTE_USER_PERM_READ) {
					object_attr |= 0x01;
				}
				if (item->attributes & FTP_OBJECT_ATTRIBUTE_USER_PERM_WRITE) {
					object_attr |= 0x01 << 1;
				}
				if (item->attributes & FTP_OBJECT_ATTRIBUTE_USER_PERM_DELETE) {
					object_attr |= 0x01 << 2;
				}
			} else {
				object_attr |= 0x07; //111
			}
			if (item->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
				object_attr |= 0x01 << 3;
				folder_count++;
			} else {
				file_count++;
			}
		}
		memset(name, 0, sizeof(name));
		name_len = WideToByte(item->file_name, name, 0);
		sprintf(object, "%02X%08X%04X%02X%02X%02X%02X%02X%02X%s", object_attr, item->file_size_low, item->modified_time.year,
		        item->modified_time.month,
		        item->modified_time.day,
		        item->modified_time.hour,
		        item->modified_time.minute,
		        item->modified_time.second, name_len-1, name);

		event_ftp_indication(EV_FTP_DISPLAY_OBJECT_IND, object);
		item = LNEXT(item);
	}
	event_ftp_indication(EV_FTP_DISPLAY_COMPLETE, ER_FTP_SUCCESS);
	hlp_ftp_set_status(ST_FTP_CONNECTED);
	bse_log_info(TAG, "                        %d files\r\n", file_count);
	bse_log_info(TAG, "                        %d folders\r\n", folder_count);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Free context for push operation.
------------------------------------------------------------------------------------*/
static void hlp_ftp_free_push_context(tAppClntContextStru *context)
{
	//tAppConnInstanceStru *clnt_inst = s_conn_list.head;
	void **pfhdl;

	if (context == NULL) {
		return;
	}

	pfhdl = List_RemoveHead(&context->sub_list);

	if (FTP_USER_DATA!= NULL) {
		if (FTP_USER_DATA->op_context == context) {
			FTP_USER_DATA->op_context = NULL;
		}
	}
	while (pfhdl != NULL) {
		App_FindFileClose((DIR*)*pfhdl);
		LFREE(pfhdl);
		pfhdl = List_RemoveHead(&context->sub_list);
	}
	App_CloseFile(context->file_hdl);
	if (context->file_data != NULL) {
		if (context->file_data->file_name != NULL) {
			FREE(context->file_data->file_name);
		}
		FREE(context->file_data);
	}
	FREE(context->fpath);
	FREE(context->obj_name);
	FREE(context);

	context = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Find and push the next object of the current folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_push_next_object(tAppClntContextStru *context, UINT32 prompt)
{
	void  **pfhdl;

	if (context == NULL) {
		return;
	}

	if ((pfhdl = context->sub_list.tail) != NULL) {
		App_CloseFile(context->file_hdl);
		context->file_hdl = NULL;
		App_UpPath(context->fpath);
		if (!context->abort && !App_FindNextFileW(*pfhdl, context->file_data, context->fpath)) {
			hlp_ftp_down_path(context);
			if (context->file_data->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
				context->file_data->attributes = 0;
				FTP_CreateFolderReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_down_folder_cfm_push);
			} else {
				hlp_ftp_push_file(context);
			}
		} else {
			/* else, folder push is aborted or nothing to send. Navigate both local and remote to parent. */
			List_RemoveTail(&context->sub_list);
			App_FindFileClose(*pfhdl);
			LFREE(pfhdl);
			FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_folder_cfm_push);
		}
	} else {
		/* The last object has been pushed */
		//	if (prompt) {
		if (!context->abort) {
			event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_SUCCESS);
		} else {
			event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_LOCAL_ABORT);
		}
		if(hlp_ftp_get_status() == ST_FTP_PUSHING) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}
		hlp_ftp_free_push_context(context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Find and push the first object of the current folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_push_first_object(tAppClntContextStru *context)
{
	void ** pfhdl = NULL;
	DIR* fhdl = NULL;

	if (context == NULL) {
		return;
	}

	if (!context->abort) {
		fhdl = App_FindFirstFileW(context->fpath, context->file_data);
	}
	if (fhdl != NULL) {
		pfhdl = List_NodeNew(sizeof(void**));
		*pfhdl = fhdl;
		List_AddTail(&context->sub_list, pfhdl);
		hlp_ftp_down_path(context);
		if (context->file_data->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
			context->file_data->attributes = 0;
			FTP_CreateFolderReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_down_folder_cfm_push);
		} else {
			hlp_ftp_push_file(context);
		}
	} else {
		/* else, folder push is aborted or nothing to send. Navigate the remote to its parent. */
		FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_up_folder_cfm_push);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get push folder size.
------------------------------------------------------------------------------------*/
void App_GetPushFolderSize(tAppClntContextStru *context, char * folder_path)
{
	DIR   *dp = NULL;
	struct   dirent   *   dirp;
	char  file_path[MAX_FILE_PATH_LEN];
	struct stat buf;

	if (context == NULL) {
		return;
	}

	if ((dp=opendir(folder_path))==NULL) {
		closedir(dp);
		dp = NULL;
		return 0;
	}
	while((dirp=readdir(dp))!=NULL) {
		if ((strcmp(dirp->d_name, ".")==0) || (strcmp(dirp->d_name, "..")==0)) {
			continue;
		}
		if ((strlen(dirp->d_name)+strlen(file_path))>(MAX_FTP_PATH_LEN-2)) {
			continue;
		}

		memset(&buf, 0, sizeof(buf));
		memset(file_path, 0, sizeof(file_path));

		sprintf(file_path, "%s/%s", folder_path, dirp->d_name);

		if (stat(file_path,   &buf) < 0) {
			continue;
		}

		if ((buf.st_mode & S_IFMT) == S_IFDIR) { //ÊÇÄ¿Â¼
			App_GetPushFolderSize(context, file_path);
		} else {
			if (context->folder_size.low32+buf.st_size > 0xFFFFFFFF) {
				context->folder_size.high32 ++;
				context->folder_size.low32 = context->folder_size.low32+buf.st_size - 0xFFFFFFFF;
			} else {
				context->folder_size.low32 += buf.st_size;
			}
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get push object size.
ob_flag:
           1: folder
           0: file
------------------------------------------------------------------------------------*/
void App_GetPushObjectSize(tAppClntContextStru *context, BOOL ob_flag)
{
	char file_path[MAX_FTP_PATH_LEN];
	FILE * fp;

	if (context == NULL) {
		return;
	}

	memset(file_path, 0, sizeof(file_path));
	WideToByte(context->fpath, file_path, 0);

	if (ob_flag) {
		App_GetPushFolderSize(context, file_path);
	} else {
		fp = App_OpenFile(file_path);
		App_GetFileSize(fp, &context->folder_size);
		App_CloseFile(fp);
		fp = NULL;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Push a file.
------------------------------------------------------------------------------------*/
static void hlp_ftp_push_file(tAppClntContextStru *context)
{
	tFTP_FileInformationStru obj_info;
	char file_path[MAX_FTP_PATH_LEN];
	int path_len = 0;
	int file_len = 0;

	if (context == NULL) {
		return;
	}

	memset(file_path, 0, sizeof(file_path));
	WideToByte(context->fpath, file_path, 0);

	context->file_hdl = App_OpenFileW(file_path);
	if (context->file_hdl != NULL) {
		obj_info.name = App_GetFileNameW_NEW(context->fpath);
		if (context->file_data == NULL) {
			App_GetFileSize(context->file_hdl, &context->file_size);
			//		context->remain_size.high32 = context->file_size.high32;
			//		context->remain_size.low32 = context->file_size.low32;
		} else {
			context->file_size.high32 = context->file_data->file_size_high;
			context->file_size.low32 = context->file_data->file_size_low;
			//		context->remain_size.high32 = context->file_size.high32;
			//		context->remain_size.low32 = context->file_size.low32;
		}
		obj_info.length.high32 = context->file_size.high32;
		obj_info.length.low32 = context->file_size.low32;
		FTP_PushFileReq(context->conn_hdl, context, &obj_info, hlp_ftp_push_file_cfm_cbk);
	} else {
		bse_log_info(TAG, "> %s can't be opened!", file_path);
		//btmShowCommandPrompt();
		if(hlp_ftp_get_status() == ST_FTP_PUSHING) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}
		hlp_ftp_push_next_object(context, 0);
	}
}

static void hlp_ftp_free_pull_context(tAppClntContextStru *context)
{
	tAppRmtFolderListStru *items;

	if (context == NULL) {
		return;
	}

	items = List_RemoveHead(&context->sub_list);

	if (FTP_USER_DATA != NULL) {
		if (FTP_USER_DATA->op_context == context) {
			FTP_USER_DATA->op_context = NULL;
		}
		//	clnt_inst = LNEXT(clnt_inst);
	}
	while (items != NULL) {
		List_RemoveAll(&items->sub_list);
		LFREE(items);
		items = List_RemoveHead(&context->sub_list);
	}
	App_CloseFile(context->file_hdl);

	FREE(context->fpath);
	FREE(context->obj_name);
	FREE(context);
	context = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Pull a folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_pull_folder_internal(tAppClntContextStru *context)
{
	if (context == NULL) {
		return;
	}

	if (!APP_CreateFolderW(context->fpath)) {
		FTP_SetChildFolderReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_down_folder_cfm_pull);
	} else {
		hlp_ftp_pull_next_object(context, 0);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Pull a file.
------------------------------------------------------------------------------------*/
static void hlp_ftp_pull_file_internal(tAppClntContextStru *context)
{
	char file_path[MAX_FTP_PATH_LEN];
	int path_len = 0;
	int file_len = 0;

	if (context == NULL) {
		return;
	}

	memset(file_path, 0, sizeof(file_path));
	WideToByte(context->fpath, file_path, 0);

	context->file_hdl = App_CreateFile(file_path);
	if (context->file_hdl != NULL) {
		FTP_PullFileReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_pull_file_cfm_cbk);
	} else {
		hlp_ftp_pull_next_object(context, 0);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Find and pull the next object of the current folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_pull_next_object(tAppClntContextStru *context, UINT32 prompt)
{
	WCHAR *file_name;
	UCHAR *obj_name;

	if (context == NULL) {
		return;
	}

	tAppRmtFolderListStru *items = context->sub_list.tail;
	if (items != NULL) {
		App_CloseFile(context->file_hdl);
		context->file_hdl = NULL;

		if (context->abort) {
			obj_name = NEW(bt_wcslen(context->fpath)+1);
			memset(obj_name, 0, (bt_wcslen(context->fpath))+1);
			WideToByte(context->fpath, obj_name, 0);
			bse_log_info(TAG, "abort pull object, remove obj_name: %s", obj_name);
			remove(obj_name);
			FREE(obj_name);
		}

		App_UpPath(context->fpath);

		file_name = List_RemoveHead(&items->sub_list);
		if (context->abort) {
			/* Folder pull is aborted, stop pulling the items left */
			while (file_name != NULL) {
				LFREE(file_name);
				file_name = List_RemoveHead(&items->sub_list);
			}
		}
		if (file_name != NULL) {
			/* More items to be pulled */
			App_DownPath(&context->fpath, &context->fpath_size, file_name);
			if (file_name == items->folder_start) {
				/* Pull sub folder */
				items->folder_start = items->sub_list.head;
				hlp_ftp_pull_folder_internal(context);
			} else {
				/* Pull file */
				hlp_ftp_pull_file_internal(context);
			}
			LFREE(file_name);
		} else {
			/* All items have be pulled */
			List_RemoveTail(&context->sub_list);
			LFREE(items);
			FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_up_folder_cfm_pull);
		}
	} else {
		/* The last object has been pulled */
		//	if (prompt) {}

		if (!context->abort) {
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_SUCCESS);
		} else {
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_LOCAL_ABORT);
		}

		if (hlp_ftp_get_status() == ST_FTP_PULLING) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}

		hlp_ftp_free_pull_context(context);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Find and pull the first object of the current folder.
------------------------------------------------------------------------------------*/
static void hlp_ftp_pull_first_object(tAppClntContextStru *context)
{
	char * p;
	if (context==NULL)
		return;

	tAppRmtFolderListStru *items = context->sub_list.tail;
	WCHAR *file_name = List_RemoveHead(&items->sub_list);	/* items shall not be NULL */

	if (context->abort) {
		/* Folder pull is aborted, stop pulling the items */
		while (file_name != NULL) {
			LFREE(file_name);
			file_name = List_RemoveHead(&items->sub_list);
		}
	}
	if (file_name != NULL) {
		App_DownPath(&context->fpath, &context->fpath_size, file_name);
		if (file_name == items->folder_start) {
			/* Pull sub folder */
			items->folder_start = items->sub_list.head;
			hlp_ftp_pull_folder_internal(context);
		} else {
			/* Pull file */
			hlp_ftp_pull_file_internal(context);
		}
		LFREE(file_name);
	} else {
		/* Empty folder */
		List_RemoveTail(&context->sub_list);
		LFREE(items);
		FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_up_folder_cfm_pull);
	}
}

static void hlp_ftp_get_next_object_size(tAppClntContextStru *context, UINT32 prompt)
{
	WCHAR *file_name;

	if (context == NULL) {
		return;
	}

	tAppRmtFolderListStru *items = context->sub_list.tail;
	if (items != NULL) {
		file_name = List_RemoveHead(&items->sub_list);
#if 1
		if (context->abort) {
			/* Folder pull is aborted, stop pulling the items left */
			while (file_name != NULL) {
				LFREE(file_name);
				file_name = List_RemoveHead(&items->sub_list);
			}
		}
#endif
		if (file_name != NULL) {
			if (file_name == items->folder_start) {
				items->folder_start = items->sub_list.head;
				FTP_SetChildFolderReq(context->conn_hdl, context, file_name, hlp_ftp_change_folder_down_cbk);
			}
			LFREE(file_name);
		} else {
			/* All items have be pulled */
			List_RemoveTail(&context->sub_list);
			LFREE(items);
			FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_change_folder_up_cbk);
		}
	} else {
		s_get_folder_size = 1;
	}
}

static void hlp_ftp_get_first_object_size(tAppClntContextStru *context)
{
	if (context==NULL)
		return;

	tAppRmtFolderListStru *items = context->sub_list.tail;
	WCHAR *file_name = List_RemoveHead(&items->sub_list);	/* items shall not be NULL */
#if 1
	if (context->abort) {
		/* Folder pull is aborted, stop pulling the items */
		while (file_name != NULL) {
			LFREE(file_name);
			file_name = List_RemoveHead(&items->sub_list);
		}
	}
#endif
	if (file_name != NULL) {
		if (file_name == items->folder_start) {
			/* Pull sub folder */
			items->folder_start = items->sub_list.head;
			FTP_SetChildFolderReq(context->conn_hdl, context, file_name, hlp_ftp_change_folder_down_cbk);
		}
		LFREE(file_name);
	} else {
		/* Empty folder */
		List_RemoveTail(&context->sub_list);
		LFREE(items);
		FTP_SetParentFolderReq(context->conn_hdl, context, hlp_ftp_change_folder_up_cbk);
	}
}

static void hlp_ftp_change_folder_down_cbk(void *handle, void *context, tFTP_Event_Type event, UINT16 *result)
{
	tAppClntContextStru * ct;

	if (context == NULL) {
		return;
	}

	ct = (tAppClntContextStru *)context;
	if (*result == ER_FTP_SUCCESS) {
		if (!ct->abort) {
			tAppRmtFolderListStru * items = List_NodeNew(sizeof(tAppRmtFolderListStru));
			List_AddTail(&ct->sub_list, items);
			FTP_BrowseFolderReq(ct->conn_hdl, ct, hlp_ftp_get_remote_folder_size_cbk);
		} else {
			FTP_SetParentFolderReq(ct->conn_hdl, ct, hlp_ftp_change_folder_up_cbk);
		}
	} else {
		if (*result !=ER_FTP_NO_CONNECTION) {
			hlp_ftp_get_next_object_size(ct, 1);
		}
	}
}

static void hlp_ftp_change_folder_up_cbk(void *handle, void *context, tFTP_Event_Type event, UINT16 *result)
{
	tAppClntContextStru * ct;

	if (context == NULL) {
		return;
	}

	ct = (tAppClntContextStru *)context;

	if (*result == ER_FTP_SUCCESS) {
		hlp_ftp_get_next_object_size(ct, 1);
	}
}

static void hlp_ftp_get_remote_folder_size_cbk(void *handle, void *context, tFTP_Event_Type event, void *param)
{
	tAppClntEvtParameters real_par;
	tAppRmtFolderListStru * items;

	real_par.param = param;
	tAppClntContextStru * ct;

	if (context == NULL) {
		return;
	}

	ct = (tAppClntContextStru *)context;

	switch (event) {
	case FTP_EVT_ASK4_GET_REQUEST_IND:
		break;
	case FTP_EVT_OBJECT_INFORMATION_IND: {
		items = ct->sub_list.tail;
		WCHAR * file_name;
		int slen = strlen(real_par.obj_info->name)+1;

		if ((strcmp(real_par.obj_info->name, ".")==0) || ((strcmp(real_par.obj_info->name, "..")==0))) {
			return;
		}
		if (real_par.obj_info->length.high32 > 0) {
			ct->file_size.high32 += real_par.obj_info->length.high32;
		}
		if ((ct->file_size.low32 + real_par.obj_info->length.low32) >= 0xFFFFFFFF) {
			ct->file_size.high32 ++;
			ct->file_size.low32 = ct->file_size.low32+real_par.obj_info->length.low32-0xFFFFFFFF;
		} else {
			ct->file_size.low32 += real_par.obj_info->length.low32;
		}

		file_name = List_NodeNew(slen*sizeof(WCHAR));
		UTF8ToWide(real_par.obj_info->name, (UCHAR *)file_name, slen);
		if (real_par.obj_info->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
			if (items->folder_start == NULL) {
				items->folder_start = file_name;
			}
			List_AddTail(&items->sub_list, file_name);
		}
	}
	break;
	case FTP_EVT_PULL_OBJECT_COMPLETE: {
		if (*real_par.result != ER_FTP_NO_CONNECTION) {
			hlp_ftp_get_first_object_size(ct);
		}
	}
	break;
	default:
		break;
	}
}

void hlp_ftp_get_folder_size_in_pull(tAppClntContextStru * context)
{
	if (context == NULL) {
		if (s_timer_notify_state != NULL) {
			FSM_DelTimer(&s_timer_notify_state);
			s_timer_notify_state = NULL;
		}
		return;
	}

	if (s_get_folder_size == 1) {
		context->remain_size.high32 = context->file_size.high32;
		context->remain_size.low32 = context->file_size.low32;
		hlp_ftp_pull_folder_internal(context);
		if (s_timer_notify_state != NULL) {
			FSM_DelTimer(&s_timer_notify_state);
			s_timer_notify_state = NULL;
		}
		s_get_folder_size = 0;
		return;
	}
	s_timer_notify_state = FsmAddTimer(NULL, 300, 0, context, NULL, (UINT32)hlp_ftp_get_folder_size_in_pull);
}

void hlp_ftp_connect(UINT8* bd_addr)
{
	FTP_ClientStartReq(NULL, bd_addr, NULL, hlp_ftp_connection_cbk);
}

void hlp_ftp_disconnect(void)
{
	if (FTP_USER_DATA) {
		FTP_DisconnectReq(FTP_USER_DATA->ftp_handle);
	} else {
		event_ftp_indication(EV_FTP_DISCONNECTED, ER_FTP_BTAPP_DONE);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
List remote folder.
------------------------------------------------------------------------------------*/
void hlp_ftp_list_folder(void)
{
	if ((FTP_USER_DATA!=NULL) && (hlp_ftp_get_status()>=ST_FTP_CONNECTED)) {
		if (FTP_USER_DATA->folder_list.head != NULL) {
			hlp_ftp_display_folder();
		} else {
			if (FTP_USER_DATA->op_context == NULL) {
				FTP_USER_DATA->op_context = FTP_USER_DATA;
				FTP_BrowseFolderReq(FTP_USER_DATA->ftp_handle, FTP_USER_DATA, hlp_ftp_browse_folder_cfm_cbk);
			}
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Update remote folder.
------------------------------------------------------------------------------------*/
void hlp_ftp_browse_remote_folder(void)
{
	if (FTP_USER_DATA == NULL) {
		event_ftp_indication(EV_FTP_DISPLAY_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if ((FTP_USER_DATA->op_context==NULL) && (hlp_ftp_get_status()>=ST_FTP_CONNECTED)) {
		tApp_FileDataStru *item = List_RemoveHead(&FTP_USER_DATA->folder_list);
		while (item != NULL) {
			FREE(item->file_name);
			LFREE(item);
			item = List_RemoveHead(&FTP_USER_DATA->folder_list);
		}
		FTP_USER_DATA->op_context = FTP_USER_DATA;
		FTP_BrowseFolderReq(FTP_USER_DATA->ftp_handle, FTP_USER_DATA, hlp_ftp_browse_folder_cfm_cbk);
	}
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Change folder.
------------------------------------------------------------------------------------*/
void hlp_ftp_change_folder(char * path)
{
	UINT32 str_size;
	WCHAR *folder; /* used as the context */

	if (FTP_USER_DATA == NULL) {
		event_ftp_indication(EV_FTP_CHANGE_FOLDER_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if ((FTP_USER_DATA->op_context==NULL) && (hlp_ftp_get_status()>=ST_FTP_CONNECTED)) {
		if (!strcmp("..", path)) {
			FTP_SetParentFolderReq(FTP_USER_DATA->ftp_handle, NULL, hlp_ftp_up_folder_cfm);
		} else if (!strcmp(".", path)) {
			/* keep this folder */
		} else if (!strcmp("/", path) || !strcmp("\\", path)) {
			FTP_SetRootFolderReq(FTP_USER_DATA->ftp_handle, NULL, hlp_ftp_set_root_cfm);
		} else {
			str_size = strlen(path) + 1;
			folder = NEW(sizeof(WCHAR) * str_size);
			memset(folder, 0, sizeof(WCHAR) * str_size);
			ByteToWide(path, (char *)folder, str_size);
			FTP_SetChildFolderReq(FTP_USER_DATA->ftp_handle, folder, folder, hlp_ftp_down_folder_cfm);
		}
	} else {
		FTP_SetRootFolderReq(FTP_USER_DATA->ftp_handle, NULL, hlp_ftp_set_root_cfm);
	}
//		FTP_USER_DATA->op_context = FTP_USER_DATA;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Push object.
------------------------------------------------------------------------------------*/
void hlp_ftp_push_object(char *obj_name)
{
	UINT32 str_size;
	char push_path[MAX_FTP_PATH_LEN];
	tAppClntContextStru *context = NULL;

	if (FTP_USER_DATA == NULL) {
		event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if (obj_name == NULL) {
		if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}
		event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
		return;
	}

	if (FTP_USER_DATA->op_context == NULL) {
		memset(push_path, 0, sizeof(push_path));

		if ((strlen(FTP_USER_DATA->l_path)+strlen(obj_name)) > (sizeof(push_path)-1)) {
			if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			//		FREE(context);
			event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
			return;
		}
		strcat(push_path, FTP_USER_DATA->l_path);
		strcat(push_path, obj_name);
		push_path[strlen(push_path)] = '\0';

		context = NEW(sizeof(tAppClntContextStru));
		memset(context, 0, sizeof(tAppClntContextStru));

		context->obj_name = NEW(strlen(obj_name)+1);
		memset(context->obj_name, 0, strlen(obj_name)+1);
		strncpy(context->obj_name, obj_name, strlen(obj_name));

		FTP_USER_DATA->op_context = context;
		context->conn_hdl = FTP_USER_DATA->ftp_handle;
		str_size = strlen(push_path) - 1;
		if (*(push_path+str_size) == '/') {
			*(push_path+str_size) = '\0';
			str_size--;
		}
		str_size += 2;
		context->fpath = NEW(str_size * sizeof(WCHAR));
		memset(context->fpath, 0, str_size * sizeof(WCHAR));
		ByteToWide(push_path, (UCHAR *)context->fpath, str_size);
		context->fpath_size = str_size;
		FTP_USER_DATA->first_push_req=1;

		switch (App_IsFolderW(context->fpath)) {
		case 1: //folder
			context->file_data = NEW(sizeof(tApp_FileDataStru));
			memset(context->file_data, 0, sizeof(tApp_FileDataStru));
			App_GetPushObjectSize(context, 1);
			context->remain_size.low32 = context->folder_size.low32;
			context->remain_size.high32 = context->folder_size.high32;

			FTP_CreateFolderReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_down_folder_cfm_push);
			return;

		case 0://file
			App_GetPushObjectSize(context, 0);
			context->remain_size.low32 = context->folder_size.low32;
			context->remain_size.high32 = context->folder_size.high32;

			hlp_ftp_push_file(context);
			break;

		default:
			if (hlp_ftp_get_status() == ST_FTP_PUSHING) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			bse_log_api(TAG, "ftp file doesn't exist!\n");
			FTP_USER_DATA->op_context = NULL;
			FREE(context->fpath);
			FREE(context);
			event_ftp_indication(EV_FTP_PUSH_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
			break;
		}
	}
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Pull remote folder or file.
------------------------------------------------------------------------------------*/
void hlp_ftp_pull_object( char * obj_name)
{
	tAppClntContextStru *context = NULL;
	tApp_FileDataStru *item = NULL;
	char pull_path[MAX_FTP_PATH_LEN];
	char name[MAX_FTP_PATH_LEN];

	memset(pull_path, 0, sizeof(pull_path));

	if (FTP_USER_DATA == NULL) {
		event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if (obj_name == NULL) {
		if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}
		event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
		return;
	}

	if (FTP_USER_DATA->op_context == NULL) {
		item = FTP_USER_DATA->folder_list.head;
		while (item != NULL) {
			memset(name, 0, sizeof(name));
			WideToByte(item->file_name, name, 0);
			if (strcmp(name, obj_name) == 0) {
				break;
			}
			item = LNEXT(item);
		}

		if (item == NULL) {
			if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
			return;
		}

		context = NEW(sizeof(tAppClntContextStru));
		memset(context, 0, sizeof(tAppClntContextStru));
		context->obj_name = NEW(strlen(obj_name)+1);
		memset(context->obj_name, 0, strlen(obj_name)+1);
		strncpy(context->obj_name, obj_name, strlen(obj_name));

		if ((strlen(FTP_USER_DATA->l_path)+strlen(obj_name)) > (sizeof(pull_path)-1)) {
			if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
				hlp_ftp_set_status(ST_FTP_CONNECTED);
			}
			//	FREE(context);
			event_ftp_indication(EV_FTP_PULL_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
			return;
		}
		strcat(pull_path, FTP_USER_DATA->l_path);
		strcat(pull_path, obj_name);

		FTP_USER_DATA->op_context = context;
		context->conn_hdl = FTP_USER_DATA->ftp_handle;
		context->fpath_size = strlen(pull_path) + 1;

		context->fpath = NEW(context->fpath_size * sizeof(WCHAR));
		memset(context->fpath, 0, context->fpath_size * sizeof(WCHAR));
		ByteToWide(pull_path, (UCHAR *)context->fpath, context->fpath_size);

		FTP_USER_DATA->first_pull_req=1;

		if (item->attributes & FTP_OBJECT_ATTRIBUTE_FOLDER) {
			FTP_SetChildFolderReq(context->conn_hdl, context, App_GetFileNameW_NEW(context->fpath), hlp_ftp_change_folder_down_cbk);
			hlp_ftp_get_folder_size_in_pull(context);
		} else {
			context->file_size.high32 = item->file_size_high;
			context->file_size.low32 = item->file_size_low;
			context->remain_size.high32 = context->file_size.high32;
			context->remain_size.low32 = context->file_size.low32;
			hlp_ftp_pull_file_internal(context);
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Delete remote file or folder.
------------------------------------------------------------------------------------*/
void hlp_ftp_delete_object(char * path)
{
	WCHAR *fname;
	int str_size;

	if (FTP_USER_DATA == NULL) {
		event_ftp_indication(EV_FTP_DELETE_OBJECT_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if (path == NULL) {
		bse_log_info(TAG, "input obj path is invalid when delete object");
		if (hlp_ftp_get_status() > ST_FTP_CONNECTED) {
			hlp_ftp_set_status(ST_FTP_CONNECTED);
		}
		event_ftp_indication(EV_FTP_DELETE_OBJECT_COMPLETE, ER_FTP_BTAPP_CMD_ERROR);
		return;
	}

	if ((FTP_USER_DATA != NULL) && (FTP_USER_DATA->op_context == NULL)) {
		FTP_USER_DATA->op_context = FTP_USER_DATA;
		str_size = strlen(path) + 1;
		fname = NEW(str_size * sizeof(WCHAR));
		memset(fname, 0, str_size * sizeof(WCHAR));
		ByteToWide(path, (UCHAR *)fname, str_size);
		FTP_DeleteObjectReq(FTP_USER_DATA->ftp_handle, fname, fname, hlp_ftp_delete_object_cfm_cbk);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Abort current Get or Put operation.
------------------------------------------------------------------------------------*/
void hlp_ftp_abort(void)
{
	if (FTP_USER_DATA == NULL) {
		//event_ftp_indication(EV_FTP_DELETE_OBJECT_COMPLETE, ER_FTP_BTAPP_DONE);
		return;
	}

	if ((FTP_USER_DATA->op_context != NULL) && (FTP_USER_DATA->op_context != FTP_USER_DATA)) {
		((tAppClntContextStru *)FTP_USER_DATA->op_context)->abort = 1;
		FTP_AbortReq(FTP_USER_DATA->ftp_handle);
	}
}

void hlp_ftp_set_local_path(char * path)
{
	int p_len = 0;
	if ((FTP_USER_DATA != NULL) && (*path != '\0')) {
		if(FTP_USER_DATA->l_path != NULL) {
			p_len = strlen(FTP_USER_DATA->l_path);
			if (*(path+strlen(path)-1) == '/') {
				if (strlen(FTP_USER_DATA->l_path) < strlen(path)) {
					p_len = strlen(path);
					FTP_USER_DATA->l_path = realloc(FTP_USER_DATA->l_path, p_len+1);
				}
				memset((char *)FTP_USER_DATA->l_path, 0, p_len+1);
				strncpy((char *)FTP_USER_DATA->l_path, path, strlen(path));
			} else {
				if (strlen(FTP_USER_DATA->l_path) <= strlen(path)) {
					p_len = strlen(path)+1; //to add '/'
					FTP_USER_DATA->l_path = realloc(FTP_USER_DATA->l_path, p_len+1);
				}
				memset((char *)FTP_USER_DATA->l_path, 0, p_len+1);
				strncpy((char *)FTP_USER_DATA->l_path, path, strlen(path));
				*(FTP_USER_DATA->l_path+strlen(path)) = '/';
			}
		}
	}
}

void hlp_ftp_set_status(UINT8 status)
{
	bse_log_api(TAG, "hlp_ftp_set_status, you set %d\r\n", status);
	if (FTP_USER_DATA) {
		FTP_STATE = status;
	}
}

UINT8 hlp_ftp_get_status(void)
{
	if (FTP_USER_DATA) {
		return FTP_STATE;
	}

	return ST_FTP_UNINIT;
}

UINT8 *hlp_ftp_get_handle(void)
{
	if (FTP_USER_DATA) {
		return FTP_USER_DATA->ftp_handle;
	} else {
		return NULL;
	}
}

#endif

