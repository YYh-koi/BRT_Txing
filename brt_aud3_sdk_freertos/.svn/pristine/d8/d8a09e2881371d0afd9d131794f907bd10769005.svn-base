#include "auto_ftp.h"

#ifdef CONFIG_FTP
#define TAG "bse_ftp"
void app_ftp_init(module_event_cbk_t p_event_cbk)
{
	hlp_ftp_init(p_event_cbk);
	hlp_ftp_set_status(ST_FTP_INIT);
	hlp_ftp_set_status(ST_FTP_READY);
}

UINT8 app_ftp_get_status(void)
{
	return hlp_ftp_get_status();
}

void app_ftp_done(void)
{
	hlp_ftp_done();
}

void app_ftp_server_start(void)
{
	//TODO
}

void app_ftp_server_stop(void)
{
	//TODO
}

UINT8 app_ftp_connect(UINT8* bd_addr)
{
	bse_log_api(TAG, "enter app ftp connect");
	if (hlp_ftp_get_status() == ST_FTP_READY) {
		if(bd_addr) {
			bse_log_info(TAG, "ftp connect addr: %02X:%02X:%02X:%02X:%02X:%02X", bd_addr[0], bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
			hlp_ftp_set_status(ST_FTP_CONNECTING);
			hlp_ftp_connect(bd_addr);
		} else {
			bse_log_info(TAG, "bd_addr is NULL");
		}
	} else {
		//TODO
		return -1;
	}

	return 0;
}

UINT8 app_ftp_disconnect(void)
{
	if (hlp_ftp_get_status() >= ST_FTP_CONNECTED) {
		hlp_ftp_disconnect();
	} else {
		//TODO
		return -1;
	}

	hlp_ftp_set_status(ST_FTP_READY);

	return 0;
}

/*list remote folder*/
UINT8 app_ftp_list_remote_folder(void)
{
	if (hlp_ftp_get_status() == ST_FTP_CONNECTED) {
		hlp_ftp_set_status(ST_FTP_LISTING);
		hlp_ftp_list_folder();

		return 0;
	}
	return -1;
}

UINT8 app_ftp_set_local_path(char * path)
{
	if (hlp_ftp_get_status() >= ST_FTP_READY) {
		hlp_ftp_set_local_path(path);
	}

	return 1;
}

/*browse remote folder*/
UINT8 app_ftp_browse_remote_folder(void)
{
	if (hlp_ftp_get_status() == ST_FTP_CONNECTED) {
		hlp_ftp_set_status(ST_FTP_LISTING);
		hlp_ftp_browse_remote_folder();
		return 0;
	}
	return -1;
}

/*change folder*/
UINT8 app_ftp_change_folder(char * path)
{
	if ((hlp_ftp_get_status() == ST_FTP_CONNECTED) && (path != NULL)) {
		hlp_ftp_set_status(ST_FTP_CHANGING);
		hlp_ftp_change_folder(path);
		return 0;
	}
	return -1;
}

UINT8 app_ftp_push_object(char * path)
{
	if (hlp_ftp_get_status() == ST_FTP_CONNECTED) {
		hlp_ftp_set_status(ST_FTP_PUSHING);
		hlp_ftp_push_object(path);
		return 0;
	}
	return -1;
}

UINT8 app_ftp_pull_object(char *path)
{
	if (hlp_ftp_get_status() == ST_FTP_CONNECTED) {
		hlp_ftp_set_status(ST_FTP_PULLING);
		hlp_ftp_pull_object(path);
		return 0;
	}
	return -1;
}

UINT8 app_ftp_delete_object(char * path)
{
	if (hlp_ftp_get_status() == ST_FTP_CONNECTED) {
		hlp_ftp_set_status(ST_FTP_DELETING);
		hlp_ftp_delete_object(path);
		return 0;
	}
	return -1;
}

UINT8 app_ftp_abort(void)
{
	if (hlp_ftp_get_status() >= ST_FTP_CONNECTED) {
		hlp_ftp_abort();
		hlp_ftp_set_status(ST_FTP_CONNECTED);
		return 0;
	}
	return -1;
}

profile_t g_ftp_profile = {
	app_ftp_init,
	app_ftp_done,
	app_ftp_server_start,
	app_ftp_server_stop,
};

#endif

