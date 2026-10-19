/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		hlp_a2dp.c
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "hci_ui.h"
#include "internal_a2dp.h"
#include "auto_a2dp.h"
#include "a2dp_ui.h"
#include "auto_commfunc.h"
#include "vendor_interface.h"
#include "brt_trace.h"
#include "parse_a2dp_data.h"
#include "bt_audio_hw.h"
#include "bt_power_manager.h"
#include "internal_manager.h"

#define TAG "bse_a2dp"

static struct A2DPUserDataStru *s_a2dp_user_data = NULL;
// TODO
extern vendor_interface_t *g_vendor_itf;

#define CASE_RETURN_STR(const) case const: return #const;

static char *get_state_string(int state)
{
	switch (state) {
		CASE_RETURN_STR(ST_A2DP_INIT);
		CASE_RETURN_STR(ST_A2DP_READY);
		CASE_RETURN_STR(ST_A2DP_CONNECTING);
		CASE_RETURN_STR(ST_A2DP_CONNECTED);
		CASE_RETURN_STR(ST_A2DP_PLAYING);
		CASE_RETURN_STR(ST_A2DP_END);
	default:
		return "Unknown state.";
	}
}

void event_a2dp_indication(UINT32 event, void *param)
{
	if(event == EV_A2DP_AV_PLAYING) {
		set_a2dp_state(STATE_PLAYING);
	} else if(event == EV_A2DP_AV_SUSPEND) {
		set_a2dp_state(STATE_PAUSED);
	}

	if (s_a2dp_user_data->event_cbk != NULL) {
		s_a2dp_user_data->event_cbk((PROFILE_ID_A2DP << 8) | event, param);
	}
}

void hlp_a2dp_init(module_event_cbk_t event_cbk)
{
	s_a2dp_user_data = (struct A2DPUserDataStru *)NEW(sizeof(struct A2DPUserDataStru));
	memset(s_a2dp_user_data, 0, sizeof(struct A2DPUserDataStru));

	s_a2dp_user_data->event_cbk = event_cbk;
	hlp_a2dp_set_status(ST_A2DP_INIT);
}

static void hlp_a2dp_set_security()
{
	struct HCI_SecurityInfoStru *a2dp_reg = NULL;

	a2dp_reg = List_NodeNew(sizeof(struct HCI_SecurityInfoStru));
	a2dp_reg->proto_id = PROT_L2CAP;
	a2dp_reg->channel = PSM_AVDTP;
	a2dp_reg->mask = HCI_SECURITY_AUTHORIZE|HCI_SECURITY_AUTHENTICATE|HCI_SECURITY_ENCRYPT|HCI_SECURITY_INCOMING;
	HCI_Security_RequireReg(a2dp_reg);
}

void hlp_a2dp_server_start(void)
{
	s_a2dp_user_data->svr_hdl = A2DP_RegisterService(A2DP_TSEP_SNK, A2DP_HEADPHONE);
	hlp_a2dp_register_cbk();
	hlp_a2dp_register_sep();
#ifdef CONFIG_BT_CHIP_CSR8350
	hlp_a2dp_register_sep_aptx();
#endif
#ifdef CONFIG_CONNECTION_ATHENTICATE_REQUIRED
	hlp_a2dp_set_security();
#endif
	hlp_a2dp_set_status(ST_A2DP_READY);
}

void hlp_a2dp_server_stop(void)
{
	if (s_a2dp_user_data->seid != 0) {
		A2DP_UnRegister_SEP(s_a2dp_user_data->seid);
		A2DP_UnregisterService(s_a2dp_user_data->svr_hdl);
		s_a2dp_user_data->seid = 0;
	}

	hlp_a2dp_set_status(ST_A2DP_INIT);
}
extern struct FsmTimer *s_timer_connect_a2dp;
void hlp_a2dp_done(void)
{
	if (s_a2dp_user_data != NULL) {
		FREE(s_a2dp_user_data);
		s_a2dp_user_data = NULL;
	}

	if (s_timer_connect_a2dp != NULL) {
				FSM_DelTimer(&s_timer_connect_a2dp);
				s_timer_connect_a2dp = NULL;
	}
}

void hlp_a2dp_register_sdp()
{
	if(s_a2dp_user_data->svr_hdl <= 0)
		s_a2dp_user_data->svr_hdl = A2DP_RegisterService(A2DP_TSEP_SNK, A2DP_HEADPHONE);
}

void hlp_a2dp_unregister_sdp()
{
	A2DP_UnregisterService(s_a2dp_user_data->svr_hdl);
}

void hlp_a2dp_register_sep()
{
	UINT8 *p;
	UINT8 len = 10;
	struct A2DP_SEPStru *reg_sep = List_NodeNew(sizeof(struct A2DP_SEPStru) + len);
	reg_sep->mask = 0x20;
	reg_sep->media_type = (0x00 << 4) | (0x01 << 3);
	reg_sep->sc.len = len;
	p = reg_sep->sc.service_capability;
	p[0] = 1;
	p[1] = 0;
	p[2] = 7;
	p[3] = 6;
	p[4] = 0 << 4;
	p[5] = 0;
	p[6] = 0x20 | 0x10 | 0x0F;
	p[7] = 0xF0 | 0x0C | 0x03;
	p[8] = 2;
	p[9] = 32;

	A2DP_Register_SEP(reg_sep);
}

#ifdef CONFIG_BT_CHIP_CSR8350
void hlp_a2dp_register_sep_aptx(void)
{
	struct A2DP_SEPStru *sep;
	UINT8 *p;
	UINT8 len;

	len = 13;
	sep = List_NodeNew(sizeof(struct A2DP_SEPStru) + len);
	sep->mask = 0x20;
	sep->media_type = (0x00 << 4) | (0x01 << 3);
	sep->sc.len = len;
	p = sep->sc.service_capability;
	p[0] = 1;
	p[1] = 0;
	p[2] = 7;
	p[3] = 9;
	p[4] = 0 << 4;
	p[5] = A2DP_CODEC_NONA2DP;
	p[6] = 0x4f;
	p[7] = 0x00;
	p[8] = 0x00;
	p[9] = 0x00;
	p[10] = 0x01;
	p[11] = 0x00;
	p[12] = A2DP_APTX_SF_ALL | A2DP_APTX_CHMODE_STEREO;
	A2DP_Register_SEP((struct A2DP_SEPStru *)sep);
}
#endif

void hlp_a2dp_register_cbk()
{
	struct A2DP_RegCbkStru *regCbk_in;

	regCbk_in = NEW(sizeof(struct A2DP_RegCbkStru));
	regCbk_in->media_type = A2DP_MEDIATYPE_AUDIO;
	regCbk_in->cbk = hlp_a2dp_cbk;
	A2DP_RegCbk(regCbk_in);
}

void hlp_a2dp_reg_sep_complete(void *param)
{
	if (param != NULL) {
		struct A2DP_SEIDStru * sep = (struct A2DP_SEIDStru *) param;
		s_a2dp_user_data->seid = sep->seid;
		hlp_a2dp_set_status(ST_A2DP_READY);
	}
}

void hlp_a2dp_stream_open(void *param)
{
	struct AVDTP_ReadyToStartStru *info =( struct AVDTP_ReadyToStartStru *)param;
	UINT16 conn_hdl = hlp_gap_get_connection_handle(A2DP_USER_DATA->remote_bd);

#ifdef CONFIG_OPEN_FILTER_DATA
    btsnoop_save_avdtp_cid(info->cid, (long)info->stream_handle);
#endif

	hlp_a2dp_set_status(ST_A2DP_CONNECTED);
	A2DP_USER_DATA->cid = info->cid;
	A2DP_USER_DATA->stream_handle = info->stream_handle;
	g_vendor_itf->vendor_on_stream_open(info->cid, conn_hdl);
	event_a2dp_indication(EV_A2DP_AV_OPEN, A2DP_USER_DATA->remote_bd);
}

void hlp_a2dp_stream_start(void)
{
	if (hlp_a2dp_get_status() == ST_A2DP_CONNECTED) {
		hlp_a2dp_set_status(ST_A2DP_PLAYING);
		hlp_gap_QoS_setup(hlp_gap_get_connection_handle(A2DP_USER_DATA->remote_bd));
	}
	event_a2dp_indication(EV_A2DP_AV_PLAYING, NULL);
    bse_log_api(TAG, "hlp_a2dp_stream_start 1\r\n, vendor p=0x%04x", g_vendor_itf);
	g_vendor_itf->vendor_on_stream_start();
    bse_log_api(TAG, "hlp_a2dp_stream_start 2\r\n");
}

void hlp_a2dp_stream_suspend(void)
{
	if (hlp_a2dp_get_status() == ST_A2DP_PLAYING) {
		hlp_a2dp_set_status(ST_A2DP_STREAMING);
	}
	g_vendor_itf->vendor_on_stream_suspend();
	event_a2dp_indication(EV_A2DP_AV_SUSPEND, NULL);
}

void hlp_a2dp_stream_create(void *param)
{
	struct AVDTP_StreamCreateStru *out = (struct AVDTP_StreamCreateStru *)param;

	memcpy(A2DP_USER_DATA->remote_bd, out->remote_bd, BD_ADDR_LEN);
}

void hlp_a2dp_stream_done(void *param)
{
	struct A2DP_Stream_DoneStru *out = (struct A2DP_Stream_DoneStru *)param;

	if(out->reason != 0x0d) {
		if (A2DP_STATE >= ST_A2DP_CONNECTED) {
			// XXX history: if((A2DP_USER_DATA->stream_handle!=NULL) && (A2DP_USER_DATA->stream_handle==out->stream_handle))
			g_vendor_itf->vendor_on_stream_done();
		}

		
#ifdef CONFIG_OPEN_FILTER_DATA
        btsnoop_clean_avdtp_cid((long)out->stream_handle);
        btsnoop_cleanup_all_avdtp();
#endif

		hlp_a2dp_set_status(ST_A2DP_READY);
		event_a2dp_indication(EV_A2DP_DISCONNECTED, A2DP_USER_DATA->remote_bd);
	}
	A2DP_USER_DATA->stream_handle = NULL;
}

static void hlp_a2dp_stream_data_ind(void *param)
{
#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP
	struct AVDTP_Stream_DataIndStru *p_data = (struct AVDTP_Stream_DataIndStru *)param;

	if (p_data != NULL && p_data->data != NULL) {
		event_a2dp_indication(EV_A2DP_AV_DATA_IND, p_data->data);
#ifdef CHECK_EMPTY_DATA
		int ret = check_empty_data(p_data->data);//add for parse empty data
		if(ret == STATE_PAUSED) {
			event_a2dp_indication(EV_A2DP_AV_SUSPEND, NULL);
		} else if(ret == STATE_PLAYING) {
			event_a2dp_indication(EV_A2DP_AV_PLAYING, NULL);
		}
#endif	//CHECK_EMPTY_DATA
		BuffFree(p_data->data);
	}
#endif
}

static void hlp_a2dp_stream_set_configuration(void *param)
{
	struct AVDTP_SetConfigStru * config = (struct AVDTP_SetConfigStru *) param;
	struct AVDTP_ServiceCapStru *cap;
	UINT8 codec;
	UINT8 sample_byte;
	UINT16 sample = 16000;

	if (!config || config->sc.len < 7) {
		return;
	}

	cap = &config->sc;

	codec = cap->service_capability[5];
	// FIXME consider other codec type other than SBC and aptX?
	sample_byte = (codec == 0xFF ? cap->service_capability[12] : cap->service_capability[6]);

	switch (sample_byte & 0xF0) {
	case A2DP_SBC_SF_16000:
		sample = 16000;
		break;
	case A2DP_SBC_SF_32000:
		sample = 32000;
		break;
	case A2DP_SBC_SF_44100:
		sample = 44100;
		break;
	case A2DP_SBC_SF_48000:
		sample = 48000;
		break;
	}

	g_vendor_itf->vendor_on_stream_configured(codec, sample);
	event_a2dp_indication(EV_A2DP_APTX_STATUS, (codec == 0xFF) ? (void *)TRUE : (void *)FALSE);

	event_a2dp_indication(EV_A2DP_SET_SAMP, (void *)(UINT32) sample);

#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_A2DP
	event_a2dp_indication(EV_A2DP_AV_SET_CONFIG, (void *)(UINT32) sample);
#endif
}


void hlp_a2dp_cbk(UINT8 event, void *param)
{
	if (hlp_a2dp_get_status() == ST_A2DP_UNINIT) {
		bse_log_api(TAG, "hlp_avrcp_cbk, profile application not initialized!\r\n");
		return;
	}
	if (event != A2DP_EV_DATAIND) {
		bse_log_api(TAG, "hlp_a2dp_cbk event = %d\r\n", event);
	}
	switch (event) {
	case A2DP_EV_REGSEP:
		hlp_a2dp_reg_sep_complete(param);
		break;
	case A2DP_EV_STREAMOPEN:
		hlp_a2dp_stream_open(param);
		break;
	case A2DP_EV_STREAMSTART:
		hlp_a2dp_stream_start();
		break;
	case A2DP_EV_STREAMSUSPEND:
		hlp_a2dp_stream_suspend();
		break;
	case A2DP_EV_STREAMCREATE:
		hlp_a2dp_stream_create(param);
		break;
	case A2DP_EV_STREAMDONE:
		hlp_a2dp_stream_done(param);
		break;
	case A2DP_EV_DATAIND:
		hlp_a2dp_stream_data_ind(param);
		break;
	case A2DP_EV_SETCONFIGURATION:
		hlp_a2dp_stream_set_configuration(param);
		break;
	default:
		break;
	}

	if (param != NULL) {
		FREE(param);
	}

	if (event != A2DP_EV_DATAIND) {
		bse_log_api(TAG, "hlp_a2dp_cbk event = %d Exit\r\n", event);
	}
}

void  *hlp_a2dp_get_handle(void)
{
	return A2DP_USER_DATA->stream_handle;
}

void hlp_a2dp_set_status(unsigned char status)
{
	bse_log_api(TAG, "hlp_a2dp_set_status(%d)\r\n", status);
	int even_status = status;
	int play_stat = status;
	A2DP_STATE = status;

	if(even_status > ST_A2DP_INIT)
		even_status = status - 1;
	bse_log_api(TAG, "hlp_a2dp_set_status even_status = (%d)\r\n", even_status);
	if(get_g_bt_opened_status())
	{
		/*if(status < ST_A2DP_CONNECTED){
			event_a2dp_indication(EV_A2DP_PROFILESTAT, (void *) even_status);
		}else if(status > ST_A2DP_CONNECTED){
			play_stat = status - ST_A2DP_CONNECTED;
			event_a2dp_indication(EV_A2DP_PLAY_STAT, (void *) play_stat);
		}else{
			event_a2dp_indication(EV_A2DP_PROFILESTAT, (void *) even_status);
			event_a2dp_indication(EV_A2DP_PLAY_STAT, (void *) 0);
			bt_remote_device_t * p_even = hlp_manager_find_target_device(A2DP_USER_DATA->remote_bd);
			if(p_even)
				event_a2dp_indication(EV_A2DP_GET_RDEVICE_INFO, (void *) p_even);
		}*/
		event_a2dp_indication(EV_A2DP_PROFILESTAT, (void *) even_status);
	}
}

UINT8 hlp_a2dp_get_status(void)
{
	if (s_a2dp_user_data == NULL) {
		return ST_A2DP_UNINIT;
	}
	return A2DP_STATE;
}
void hlp_a2dp_query_status(void)
{
	a2dp_status_t status;
	memset(&status, 0, sizeof(a2dp_status_t));

	status.status = A2DP_STATE;
	if(A2DP_STATE > ST_A2DP_INIT)
		status.status = A2DP_STATE - 1;
	bse_log_api(TAG, "hlp_a2dp_get_status(%d)\r\n", status.status);
	memcpy(status.bd, A2DP_USER_DATA->remote_bd, BD_ADDR_LEN);
	event_a2dp_indication(EV_A2DP_PROFILESTAT, (void *) status.status);
}


void hlp_a2dp_event_OK()
{
	event_a2dp_indication(EV_A2DP_OK, NULL);
}

void hlp_a2dp_event_mute(int value)
{
	event_a2dp_indication(EV_A2DP_SET_MUTE, (void *)value);	
}