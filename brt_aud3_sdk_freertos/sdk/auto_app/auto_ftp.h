#ifndef _APP_FTP_H_
#define _APP_FTP_H_
#include "bt_event.h"
#include "ftp_ui.h"
#include "internal_ftp.h"
#include "brt_trace.h"
#include "autoconf_bt.h"
#include "fileio.h"
#include "profile.h"

#ifdef CONFIG_FTP
/* State */
enum {
    ST_FTP_UNINIT,
    ST_FTP_INIT,
    ST_FTP_READY,
    ST_FTP_CONNECTING,
    ST_FTP_CONNECTED,
    ST_FTP_LISTING,
    ST_FTP_CHANGING,
    ST_FTP_DELETING,
    ST_FTP_PULLING,
    ST_FTP_PUSHING,
    ST_FTP_END,
};

/* Event */
enum {
    EV_FTP_CONNECTING,
    EV_FTP_DISCONNECTED,
    EV_FTP_CONNECTED,
    EV_FTP_CONN_CFM,
    EV_FTP_CHANGE_FOLDER_COMPLETE,
    EV_FTP_DISPLAY_OBJECT_IND,
    EV_FTP_DISPLAY_COMPLETE,
    EV_FTP_PUSH_OBJECT_STATED,
    EV_FTP_PUSH_IN_PROGRESS,
    EV_FTP_PUSH_OBJECT_COMPLETE,
    EV_FTP_DELETE_OBJECT_STATED,
    EV_FTP_DELETE_OBJECT_COMPLETE,
    EV_FTP_PULL_OBJECT_STATED,
    EV_FTP_PULL_IN_PROGRESS,
    EV_FTP_PULL_OBJECT_COMPLETE,
};

static struct ListStru s_input_list;

#ifdef CONFIG_FTP_CLIENT

#define OS_PATH_SEPARATOR		'/'
#define UNC_PATH_SEPARATOR		'/'
typedef union {
	void								*param;
	UINT8								*bd;
	UINT16								*result;
	tFTP_ConnectCompleteStru			*conn_cmp;
	tFTP_DisconnectCompleteStru			*disc_cmp;
	tFTP_ObjectDataStru					*obj_data;
	tFTP_GetContinueFlag				*next_op;
	tFTP_Ask4PutRequestStru				*put_req;

	tFTP_ServerStartCompleteStru		*svr_start;
	tFTP_ObjectInformationStru			*obj_info;
	tFTP_Ask4GetResponseStru			*get_ind;
} tAppClntEvtParameters;

typedef struct {
	struct ListStru sub_list;
	void *folder_start;
} tAppRmtFolderListStru;

typedef struct {
	UINT16  year;
	UINT8	month;
	UINT8	day;
	UINT8	hour;
	UINT8	minute;
	UINT8	second;
} tApp_FileTimeStru;

typedef struct {
	UINT32				attributes;	/* FTP_OBJECT_ATTRIBUTE_PARENT etc. */
	UINT32				file_size_high;
	UINT32				file_size_low;
	WCHAR				*file_name;
	tApp_FileTimeStru	modified_time;
	tApp_FileTimeStru	created_time;
	tApp_FileTimeStru	accessed_time;
} tApp_FileDataStru;

typedef struct {
	void  *conn_hdl;
	WCHAR *fpath;
	UINT32 fpath_size; /* Maximum number of characters in fpath */
	/* Folder */
	UINT8  abort;
	struct ListStru sub_list;	/* PushFolder: tAPP_FIND_HANDLE. PullFolder: tAppRmtFolderListStru */
	tApp_FileDataStru *file_data;
	/* File */
	void  *file_hdl;
	UINT8 progress;
	BtUINT64Stru file_size;
	BtUINT64Stru folder_size; /*  */
	BtUINT64Stru remain_size;
	UCHAR * obj_name;
	UCHAR * l_path;
} tAppClntContextStru;

typedef struct {
	UINT8  *obj_name;
	UINT32 cur_size;
	UINT32 obj_size;
} tProgressStru;

#endif

#ifdef CONFIG_FTP_SERVER
typedef struct {
	WCHAR *fpath;
	WCHAR *spath;
	void  *file_hdl;
	BtUINT64Stru file_size;
} tAppSvrContextStru;

#endif

extern profile_t g_ftp_profile;

void app_ftp_init(module_event_cbk_t p_event_cbk);
void app_ftp_done(void);
UINT8 app_ftp_connect(UINT8* bd_addr);
UINT8 app_ftp_disconnect(void);
UINT8 app_ftp_push_object(char * file_path);
UINT8 app_ftp_list_remote_folder(void);
UINT8 app_ftp_set_local_path(char * path);
UINT8 app_ftp_browse_remote_folder(void);
UINT8 app_ftp_change_folder(char * path);
UINT8 app_ftp_push_object(char * file_path);
UINT8 app_ftp_pull_object(char *path);
UINT8 app_ftp_delete_object(char * path);
UINT8 app_ftp_abort(void);

#endif
#endif
