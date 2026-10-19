/*************************************************************************
    > File Name: hlp_dun.c
    >
    >
    > Created Time: Fri 07 Aug 2015 11:37:23 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/route.h>
#include <net/if_arp.h>

#include "btdatatype.h"
#include "global.h"
#include "rfcomm_ui.h"
#include "auto_rfcomm.h"
#include "auto_gap.h"
#include "auto_dun.h"
#include "internal_dun.h"
#include "bt_power_manager.h"
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("dun_thread", fmt,  ## __VA_ARGS__)

UINT32 g_ulGateway[1024] = {0};
static int g_nGatewayCount = 0;
static UINT8 s_current_bd[6] = {0};
static UINT8 s_dun_uuid[16] = {0x00, 0x00, 0x11, 0x03, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

typedef struct _dun_data_stru {
	UINT32 svr_hdl;
	UINT32 conn_hdl;
	UINT32 local_srv_chl;
	UINT8 conn_bd[BD_ADDR_LEN];
	UINT8 flag_out_index;
	UINT8 out_index;
	UINT8 flag_fc;
	UINT8 state;
	UINT16 out_size;
	module_event_cbk_t event_cbk;
} dun_data_t;

typedef struct _dun_tty_stru {
	int masterfd;
	int slavefd;
//	char slavename[256];

} dun_tty_t;
static UINT8 s_dun_state = 0;
static dun_data_t *s_dun_data = NULL;

static dun_tty_t *s_dun_tty = NULL;

static void hlp_dun_dial_ex();
static UINT32 hlp_dun_hangup();
static UINT32 hlp_dun_dial();
static void hlp_dun_set_status(UINT8 status);

static void event_dun_indication(UINT8 event, void *param)
{
	if (s_dun_data != NULL) {
		if (s_dun_data->event_cbk != NULL) {
			s_dun_data->event_cbk((PROFILE_ID_DUN << 8 ) | event, param);
		}
	}
}

void hlp_dun_init(module_event_cbk_t event_cbk)
{
	if (s_dun_data == NULL) {
		s_dun_data = (dun_data_t *)NEW(sizeof(dun_data_t));
	}

	if(s_dun_tty == NULL) {
		s_dun_tty = (dun_tty_t *)NEW(sizeof(dun_tty_t));
	}

	memset(s_dun_tty, 0, sizeof(dun_tty_t));
	memset(s_dun_data, 0, sizeof(dun_data_t));
	s_dun_data->event_cbk = event_cbk;
	memset(s_current_bd, 0, 6);
	hlp_dun_set_status(ST_DUN_READY);

}
void hlp_dun_done()
{
	if(s_dun_data != NULL) {
		FREE(s_dun_data);
		s_dun_data = NULL;
	}

	if(s_dun_tty != NULL) {
		FREE(s_dun_tty);
		s_dun_tty = NULL;
	}

	memset(s_current_bd, 0, 6);
}

void hlp_dun_service_start()
{
	s_dun_data->local_srv_chl= app_spp_get_server_channel();
	app_spp_server_start_ex(s_dun_data->local_srv_chl, "DUN", s_dun_uuid);
}

void hlp_dun_service_stop()
{
	app_spp_server_stop_ex(s_dun_data->svr_hdl);
}

void hlp_dun_connect(UINT8 *bd)
{
	memcpy(s_current_bd, bd, 6);
	app_spp_connect_ex(0, bd, s_dun_uuid);
	event_dun_indication(EV_DUN_CONNECTING, s_current_bd);
	hlp_dun_set_status(ST_DUN_CONNECTING);
}

void hlp_dun_disconnect()
{
	if(hlp_dun_get_status() >= ST_DUN_CONNECTING) {
		event_dun_indication(EV_DUN_DISCONNECTING, s_current_bd);
		app_spp_disconnect(s_dun_data->conn_hdl);
	}
}

static void hlp_dun_set_status(UINT8 status)
{
	s_dun_data->state = status;
}

UINT8 hlp_dun_get_status()
{
	if(s_dun_data) {
		return s_dun_data->state;
	} else {
		return ST_DUN_UNINIT;
	}
}

UINT8 bt_write2ppp(UINT8 *data, UINT16 data_len)
{
	//print_hex(data, data_len);
	int ret = write(s_dun_tty->masterfd, data, data_len);
	return ret;
}
static void hlp_dun_timer_report_tl_event(void *arg)
{
	if((UINT32)arg == EV_SPP_CONNECT_RET) {
		int masterfd = 0, slavefd = 0;
		char slavename[256];

//		if(access("/dev/dun_tty", 0) == 0)
		{
			system("rm /dev/dun_tty");
		}
		if (pty_allocate(&masterfd, &slavefd, slavename, 256) == 0) {
			return 0;
		} else {
			fork_dun_recive_thread(masterfd);
			s_dun_tty->masterfd = masterfd;
		}

		pthread_t dun_dial_thread_id;
		pthread_create(&dun_dial_thread_id, NULL, hlp_dun_dial_ex, NULL);
		pthread_detach(dun_dial_thread_id);
	} else if((UINT32)arg == EV_SPP_DISCONNECT_RET) {
		memset(s_current_bd, 0, 6);
		memset(s_dun_tty, 0, sizeof(dun_tty_t));
		memset(s_dun_data, 0, sizeof(dun_data_t));
	}
}

static void hlp_dun_dial_ex()
{
	int ret = hlp_dun_dial();
	if(ret == 0) {
		event_dun_indication(EV_DUN_CONNECTED, s_current_bd);
	} else {
		hlp_dun_disconnect();
	}
}
UINT8 hlp_dun_spp_event_cbk(UINT8 event, void *param)
{
	UINT8 ret = 0;
	LOG("hlp_dun_spp_event_cbk event is %d\r\n", event);
	if (hlp_dun_get_status() == ST_DUN_UNINIT) {
		return 0;
	}

	if(!param) {
		LOG("param is NULL, return");
		return 0;
	}

	switch(event) {
	case EV_SPP_DATA_IND: {
		//TODO
		struct app_spp_data_ind_stru *data_ind = (struct app_spp_data_ind_stru *)param;
		if((s_dun_data->state == ST_DUN_CONNECTED || s_dun_data->state == ST_DUN_STARTED) && s_dun_data->conn_hdl == data_ind->handle) {
			ret = 1;
			DUN_TL_DataReceive(data_ind->data, data_ind->data_len);
		}
		break;
	}
	case EV_SPP_CONNECT_RET: {
		rfc_con_info_t *p_conn_info = (rfc_con_info_t *)param;
		if(hlp_dun_get_status() == ST_DUN_CONNECTING && !memcmp(s_current_bd, p_conn_info->bd, BD_ADDR_LEN)) {
			if ((p_conn_info->conn_state == rfc_status_success) || (p_conn_info->conn_state == rfc_conn_state_connected)) {
				s_dun_data->conn_hdl = p_conn_info->handle;
				memcpy(s_dun_data->conn_bd, p_conn_info->bd, BD_ADDR_LEN);
				hlp_dun_set_status(ST_DUN_CONNECTED);
				hlp_dun_timer_report_tl_event(EV_SPP_CONNECT_RET);
			} else {
				hlp_dun_set_status(ST_DUN_READY);
				hlp_dun_timer_report_tl_event(EV_SPP_DISCONNECT_RET);
				event_dun_indication(EV_DUN_DISCONNECTED, p_conn_info->bd);
			}
		} else {
			LOG("not dun");
			ret = 0;
		}
		break;
	}
	case EV_SPP_DISCONNECT_RET: {
		rfc_con_info_t *p_conn_info = (rfc_con_info_t *)param;

		if (s_dun_data->conn_hdl == p_conn_info->handle) {
			hlp_dun_set_status(ST_DUN_READY);
			event_dun_indication(EV_DUN_DISCONNECTED, p_conn_info->bd);
			hlp_dun_timer_report_tl_event(event);
			hlp_dun_hangup();
		}

		break;
	}
	default:
		break;
	}

	FREE(param);
	return ret;
}

UINT32 hlp_get_dun_connhdl()
{
	return s_dun_data->conn_hdl;
}


static void creat_dun_dialer()
{
	if (0 == access("/etc/ppp/dunDialer.sh", R_OK)) {
		return ;
	}
//	system("rm /etc/ppp/dunDialer.sh");
	system("echo 'pppd /dev/dun_tty defaultroute usepeerdns noauth debug \\'>/etc/ppp/dunDialer.sh");
	system("echo 'logfile /etc/ppp/ppp10.log connect \"/system/bin/chat -v -f /etc/ppp/connectScript\"'>>/etc/ppp/dunDialer.sh");
	chmod("/etc/ppp/dunDialer.sh", 0777);
}

static int creat_connect_script()
{
	if (0 == access("/etc/ppp/connectScript", R_OK)) {
		return 1;
	}
#if 1
	FILE *fp = fopen("/etc/ppp/connectScript", "w+");
	chmod("/etc/ppp/connectScript", 0777);
	if(!fp) {
		return -1;
	}
	fwrite("TIMEOUT         10\n", 1, 19, fp);
	fwrite("ECHO            ON\n", 1, 19, fp);
	fwrite("ABORT           '\\nABORT\\r'\n", 1, 28, fp);
	fwrite("ABORT           '\\nERROR\\r'\n", 1, 28, fp);
	fwrite("ABORT           '\\nNO ANSWER\\r'\n", 1, 32, fp);
	fwrite("ABORT           '\\nNO CARRIER\\r'\n", 1, 33, fp);
	fwrite("ABORT           '\\nNO DIALTONE\\r'\n", 1, 34, fp);
	fwrite("ABORT           '\\nRINGING\\r\\n\\r\\nRINGING\\r'\n", 1, 45, fp);
	fwrite("''              \\rAT\n", 1, 21, fp);
	fwrite("TIMEOUT         15\n", 1, 19, fp);
	fwrite("OK              ATE1\n", 1, 21, fp);
	fwrite("OK              ATD*99#\n", 1, 24, fp);
	fwrite("CONNECT\n", 1, 7, fp);
	fclose(fp);
	return 1;
#else
	system("echo 'TIMEOUT         10'>/etc/ppp/connectScript");
	system("echo 'ECHO            ON'>>/etc/ppp/connectScript");
	system("echo \"ABORT           '\nABORT\r'\">>/etc/ppp/connectScript");
	system("echo 'ABORT           '\\nERROR\\r''>>/etc/ppp/connectScript");
	system("echo 'ABORT           '\\nNO ANSWER\\r''>>/etc/ppp/connectScript");
	system("echo 'ABORT           '\\nNO CARRIER\\r''>>/etc/ppp/connectScript");
	system("echo 'ABORT           '\\nNO DIALTONE\\r''>>/etc/ppp/connectScript");
	system("echo 'ABORT           '\\nRINGING\\r\\n\\r\\nRINGING\\r''>>/etc/ppp/connectScript");
	system("echo '''              \\rAT'>>/etc/ppp/connectScript");
	system("echo 'TIMEOUT         15'>>/etc/ppp/connectScript");
	system("echo 'OK              ATE1'>>/etc/ppp/connectScript");
	system("echo 'OK              ATD*99#'>>/etc/ppp/connectScript");
	system("echo 'CONNECT'>>/etc/ppp/connectScript");
	system("chmod +x /etc/ppp/connectScript");
#endif
}

static void creat_ip_up()
{
	chmod("/etc/ppp/ip-up", 0777);
}
static void creat_ip_down()
{
	chmod("/etc/ppp/ip-down", 0777);
}

void KillExistedPppd()
{
	system("killall pppd");
	sleep(1);
}

static UINT32 hlp_dun_dial()
{
	//0 kill pppd
	KillExistedPppd();

	//1 creat dunDialer.sh
	creat_dun_dialer();

	//2 creat connectScript
	creat_connect_script();

	//3 ip-up
	creat_ip_up();

	//4 ip-down
	creat_ip_down();

	//5 pppd dial
	int ret = system("/etc/ppp/dunDialer.sh");
	return ret;
}

int PingOk(int packetSize, char *pszDns)
{
	int ret = 0;
	char cmd_str[200] = { 0 };
	int count = 2;

	sprintf(cmd_str, "ping -w 2 -c %d -M do -s %d  %s > /tmp/bspr.ini", count, packetSize, pszDns);
	DBG_PRINT("ping string=%s\n", cmd_str);
	system(cmd_str);

	{
		FILE *pfile = fopen("/tmp/bspr.ini", "r");
		if (NULL != pfile) {
			char szline[150] = { 0 };
			char strSearch[20] = { 0 };
			sprintf(strSearch, "%d received", count);
			while (fgets(szline, 150, pfile)) {
				/*DBG_PRINT("szline=%s,strSearch=%s\n", szline,strSearch);*/
				if (NULL != strstr(szline, strSearch)) {
					ret = 1;
					break;
				}
			}
			fclose(pfile);
			pfile = NULL;
		}
	}

	return ret;
}

static UINT32 hlp_dun_hangup()
{
	UINT32 ret = 0;
	ret = system("killall pppd");
	interrupt_dun_recive_thread();
	return ret;
};



