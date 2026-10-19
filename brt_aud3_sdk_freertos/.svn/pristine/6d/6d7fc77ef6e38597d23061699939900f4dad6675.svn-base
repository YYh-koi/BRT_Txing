#ifndef _HLP_FTP_H_
#define _HLP_FTP_H_

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <wchar.h>
#include <errno.h>
#include "autoconf_bt.h"
#include "ftp_ui.h"
#include "bt_event.h"
#include "auto_ftp.h"
#include "brt_trace.h"
#include "fileio.h"

#ifdef CONFIG_FTP

#ifndef S_IREAD
#define S_IREAD 0X0400
#endif
#ifndef S_IWRITE
#define S_IWRITE 0X0200
#endif

#ifndef tAPP_FIND_HANDLE
#define tAPP_FIND_HANDLE void*
#endif

#ifdef CONFIG_UNLIMIT_RESOURCE
#define MAX_FTP_PATH_LEN 256
#define MAX_FTP_PATH_HEAD_LEN 64
#define MAX_FTP_OBJECT_LEN (MAX_FTP_PATH_LEN+8+14+2+2)
#else
#define MAX_FTP_PATH_LEN 128
#define MAX_FTP_PATH_HEAD_LEN 32
#define MAX_FTP_OBJECT_LEN (MAX_FTP_PATH_LEN+8+14+2+2)
#endif

struct FTPUserDataStru {
	module_event_cbk_t event_cbk;

	void *ftp_handle;
	UINT8 bd[BDLENGTH];
	UINT8 state;
	UINT32	fpath_size;		/* Maximum number of characters in fpath */
	WCHAR   *fpath;
	UINT8	is_server;		/* 0 - client; 1 - server */
	UINT8   first_push_req;
	UINT8   first_pull_req;
	UINT32 pull_size;
	UINT32 push_remain_size;
	UCHAR * l_path;

#ifdef CONFIG_FTP_CLIENT
	void	*op_context;
	struct ListStru folder_list;
#endif
};

enum {
    ER_FTP_BTAPP_CMD_ERROR = 0X0B,
    ER_FTP_BTAPP_DONE
};

#define FTP_USER_DATA			(s_ftp_user_data)
#define FTP_STATE				(s_ftp_user_data->state)

void event_ftp_indication(UINT8 event, void *param);
void hlp_ftp_init(module_event_cbk_t p_event_cbk);
void hlp_ftp_done(void);
void hlp_ftp_connect(UINT8* bd_addr);
void hlp_ftp_disconnect(void);
void hlp_ftp_list_folder(void);
void hlp_ftp_browse_remote_folder(void);
void hlp_ftp_change_folder(char * path);
void hlp_ftp_push_object(char *path);
void hlp_ftp_pull_object(char * path);
void hlp_ftp_delete_object(char * path);
void hlp_ftp_abort(void);

void hlp_ftp_set_status(UINT8 status);
UINT8 hlp_ftp_get_status(void);
UINT8 *hlp_ftp_get_handle(void);

#endif
#endif

