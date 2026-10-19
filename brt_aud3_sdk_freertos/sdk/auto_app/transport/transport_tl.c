#include "porting.h"
#include "sysdep.h"
#include "transport_tl.h"
#include "auto_gap.h"
#ifdef CONFIG_TL_BCSP
#include "bcsp/bcsptl.h"
#endif
#include "bt_audio_hw.h"
#include "brt_trace.h"

//#include "auto_at_internal.h"

#define LOG(fmt, ...) bse_log_info("hci_tl", fmt, ## __VA_ARGS__)

typedef struct _tl_user_data_stru {
	HANDLE tl_hdl;
	HANDLE host_hdl;
	Func_Receive rcv_func;
	TransportLayerStru tl_funcs;
} tl_user_data_t;

static void hlp_tl_start_completed_cbk(int result);
static void hlp_tl_stop_completed_cbk(int result);
static void hlp_tl_restart_completed_cbk(int result);
static void hlp_tl_data_received_cbk(unsigned char *buf, unsigned len);
static void hlp_tl_error_occurred_cbk(int result);

transport_callbacks_t tl_cbk = {
	hlp_tl_start_completed_cbk,
	hlp_tl_stop_completed_cbk,
	hlp_tl_restart_completed_cbk,
	hlp_tl_data_received_cbk,
	hlp_tl_error_occurred_cbk
};

static tl_user_data_t *s_tl_user_data = NULL;
transport_t *tl = NULL;
static transport_looper_callback tl_handle = NULL;
static SYSTEM_LOCK tl_handle_lock = NULL;

static void hlp_tl_start_completed_cbk(int result)
{
	LOG("[TL] hlp_tl_start_completed_cbk, %d", result);

	TimerHandle();//reset timer!!!
	LOG("[TL] hlp_tl_start_completed_cbk, reset timerhandle", result);
	if (result == TRANSPORT_START_OK) {
		app_gap_reset_hardware();
	} else {
		//bt_event_indicate(BT_EV_EVENT_PRIMITIVE, "+DEVSTAT=0");
		LOG("[TL] hlp_tl_start_completed_cbk failed,result:%d", result);
	}
}

static void hlp_tl_stop_completed_cbk(int result)
{
	LOG("[TL] hlp_tl_stop_completed_cbk, %d", result);
}


static void hlp_tl_restart_completed_cbk(int result)
{
	LOG("[TL] hlp_tl_restart_completed_cbk, %d", result);

	if (result == 0) {
		app_gap_reset_hardware();
	}
}

static void hlp_tl_data_received_cbk(unsigned char *buf, unsigned int len)
{
	if (s_tl_user_data != NULL && s_tl_user_data->rcv_func != NULL) {
#ifdef CONFIG_BT_AUDIO_CBK
#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_SCO
		if (buf && (buf[0] == 0x03/*HCI_TLPKT_SCODATA*/)) {
			//LOG("[TL] SCO date, %d", len -4);
			bt_porting_audio_data_ind(AUDIO_TYPE_SCO, buf + 4, len - 4);

			//if(!(porting_get_debug_level() & BSE_DEBUG_BTSNOOP_SCO_AUDIO))
			//{
			//	porting_hci_dump_data(1, len, buf);
			//}
			return;
		}
#endif
#endif
		s_tl_user_data->rcv_func(s_tl_user_data->host_hdl, buf, len);
	}
	return;
}

static void hlp_tl_error_occurred_cbk(int result)
{
	LOG("[TL] hlp_tl_error_occurred_cbk, %d", result);
}

static int hlp_tl_get_type(HANDLE tl_hdl)
{
	//tl_hdl;
	return HCI_AMPID_BREDR;
}

static int hlp_tl_startup(HANDLE tl_hdl, TL_CallbackStru *cb_func, HANDLE host_hdl)
{
	s_tl_user_data->tl_hdl = tl_hdl;
	s_tl_user_data->host_hdl = host_hdl;
	s_tl_user_data->rcv_func = cb_func->Receive;

	return 0;
}

static void hlp_tl_shut_down(HANDLE tl_hdl)
{
}

static int hlp_tl_start_sco(HANDLE tl_hdl, unsigned short sco_hdl)
{
	//tl_hdl;
	//sco_hdl;
	return 0;
}

static void hlp_tl_stop_sco(HANDLE tl_hdl, unsigned short sco_hdl)
{
}

int hlp_tl_send_data(HANDLE tl_hdl, unsigned char *data, unsigned long len)
{
	if (tl != NULL && tl->send != NULL) {
		return tl->send(tl, data, len);
	} else {
		return 0;
	}
}

static void hlp_gap_tl_done_cbk(HANDLE tl_handle, UINT16 result, void *cfm_par)
{
	if (tl_handle == TL_LOCAL_HANDLE_DEFAULT && s_tl_user_data != NULL) {
		FREE(s_tl_user_data);
		s_tl_user_data = NULL;
	}

	//tl->stop(tl);
	
	LOG("[TL] hlp_gap_tl_done_cbk -");
}

void hlp_gap_tl_init(void)
{
	if (s_tl_user_data == NULL) {
		s_tl_user_data = (tl_user_data_t *)NEW(sizeof(tl_user_data_t));
	}

	memset(s_tl_user_data, 0, sizeof(tl_user_data_t));
	s_tl_user_data->tl_funcs.GetType = hlp_tl_get_type;
	s_tl_user_data->tl_funcs.Startup = hlp_tl_startup;
	s_tl_user_data->tl_funcs.Shutdown = hlp_tl_shut_down;
	s_tl_user_data->tl_funcs.Send = hlp_tl_send_data;
	s_tl_user_data->tl_funcs.StartSco = hlp_tl_start_sco;
	s_tl_user_data->tl_funcs.StopSco = hlp_tl_stop_sco;
	app_gap_init_transport_layer(TL_LOCAL_HANDLE_DEFAULT, &s_tl_user_data->tl_funcs);
}

void hlp_gap_tl_start(void)
{
	if (tl && tl->start) {
		tl->start(tl);
	}
}

void hlp_gap_tl_stop(void)
{
	if (tl && tl->stop) {
		tl->stop(tl);
	}
}

void hlp_gap_tl_done(void)
{
	app_gap_destory_transport_layer(TL_LOCAL_HANDLE_DEFAULT, hlp_gap_tl_done_cbk);
}

void app_tl_init(void)
{
	if (tl_handle_lock == NULL) {
		tl_handle_lock = CREATE_CRITICAL("TL");
	}

	ENTER_CRITICAL(tl_handle_lock);

	if (tl == NULL) {
		tl = transport_new();
	}

	if (tl->init != NULL) {
		tl->init(tl, &tl_cbk);
		tl_handle = tl->get_looper_func(tl);
	}

	LEAVE_CRITICAL(tl_handle_lock);
}

void app_tl_done(void)
{
	ENTER_CRITICAL(tl_handle_lock);
	tl_handle = NULL;

	if (tl) {
		if (tl->done) {
			tl->done(tl);
		}

		transport_free(tl);
		tl = NULL;
	}

	LEAVE_CRITICAL(tl_handle_lock);
#if 0/*only init once, fix bt_runloop_execute thread locked in TLHandle(). unkown error in free lock */
	FREE_CRITICAL(tl_handle_lock);
	tl_handle_lock = NULL;
#endif
}

void TLHandle()
{
	ENTER_CRITICAL(tl_handle_lock);

	if (tl_handle) {
		tl_handle();
	}

	LEAVE_CRITICAL(tl_handle_lock);
}

