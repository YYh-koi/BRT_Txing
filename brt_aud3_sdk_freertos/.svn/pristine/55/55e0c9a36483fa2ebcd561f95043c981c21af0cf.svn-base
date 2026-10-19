/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
* Copyright (c) 1999-2014 BARROT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/**********************************************************************
*	File Name:		app_hf.c
*
*	Description:
*
*
*	Create Date:	2014/05/29
***********************************************************************/

#include "hfp_ui.h"
#include "internal_hf.h"
#include "auto_hf.h"
#include "brt_trace.h"
#include "porting_utils.h"
#ifdef PATCH_RECONN
#include "patch_reconn.h"
#endif
#define TAG "bse_hfp"

static int s_hf_redial_tick = 0;

void app_hf_init(module_event_cbk_t p_event_cbk)
{
	if (hlp_hf_get_status() != ST_HF_UNINIT) {
		return;
	}
	hlp_hf_init(p_event_cbk);
}

void app_hf_server_start(void)
{
	if (hlp_hf_get_status() != ST_HF_INIT) {
		return;
	}

	hlp_hf_server_start();
}

void app_hf_server_stop(void)
{
	if (hlp_hf_get_status() < ST_HF_READY) {
		return;
	}

	hlp_hf_server_stop();
}

void app_hf_done(void)
{
	if (hlp_hf_get_status() == ST_HF_UNINIT) {
		return;
	} else {
		app_hf_server_stop();
	}

	hlp_hf_done();
}

void app_get_config_hfp()
{
	hlp_get_config_hfp();
}

UINT8 app_hf_connect(UINT8* bd_addr)
{

	if (hlp_hf_get_status() == ST_HF_READY) {
#ifdef PATCH_RECONN
		pr_clear_reconn_mgr();
		pr_set_reconn_mgr(PR_CONNECT_BY_APP);
#endif
		hlp_hf_connect(bd_addr);
	} else {
		//TODO
		return -1;
	}

	return 0;
}
extern struct FsmTimer *s_timer_disconnect_a2dp;
UINT8 app_hf_disconnect(void)
{
	if (hlp_hf_get_status() >= ST_HF_CONNECTING) {
		if(s_timer_disconnect_a2dp != NULL)
		{
			FSM_DelTimer(&s_timer_disconnect_a2dp);
			s_timer_disconnect_a2dp = NULL;
		}
		hlp_hf_set_status(ST_HF_DISCONNECTING);
		hlp_hf_disconnect(hlp_hf_get_handle());
	} else {
		//TODO
		return -1;
	}

	return 0;
}

void app_hf_dial(UINT8 *number)
{
#ifdef CONFIG_HF_SUPPORT_3_WAY_CALL
	if (hlp_hf_get_status() > ST_HF_CONNECTING && strlen(number) < 43) {
#else
	if (hlp_hf_get_status() == ST_HF_CONNECTED && strlen(number) < 43) {
#endif
		hlp_hf_event_OK();
		HFP_HF_Dial(hlp_hf_get_handle(), number);
	} else {
		//TODO
	}
}

void app_hf_send_dtmf(UINT8 *dtmf)
{
	if (hlp_hf_get_status() >= ST_HF_ONGOING) {
		UINT8 *p_dtmf = (UINT8 *)NEW(strlen(dtmf)+1);

		if (dtmf != NULL) {
			strcpy(p_dtmf, dtmf);
			hlp_hf_event_OK();
			HFP_HF_TxDTMF(hlp_hf_get_handle(), *(p_dtmf));
		} else {
			//TODO
		}
	} else {
		//TODO
	}
}

void app_hf_answer_call()
{
	//if (hlp_hf_get_status() == ST_HF_INCOMING) {
		bse_log_api(TAG, "app_hf_answer_call start = %d %d\n", hlp_hf_get_status()), hlp_hf_get_handle();
		HFP_HF_AnswerCall(hlp_hf_get_handle());
	//} else {
		//TODO
	//}
}

void app_hf_reject_call()
{
	bse_log_api(TAG, "app_hf_reject_call start = %d\n", hlp_hf_get_status());
	if (hlp_hf_get_status() > ST_HF_CONNECTED) {//hlp_hf_get_status() == ST_HF_INCOMING
		//hlp_hf_event_OK();
		HFP_HF_CancelCall(hlp_hf_get_handle());
	} else {
		//TODO
	}
}

void app_hf_redial()
{
	if (hlp_hf_get_status() >= ST_HF_CONNECTED) {//==
	    if (abs(get_system_time() - s_hf_redial_tick) >= 1000) {
		    HFP_HF_LastNumRedial(hlp_hf_get_handle());
		    s_hf_redial_tick = get_system_time();
		} else {
            bse_log_api(TAG,  "redial overquick");
		}
	} else {
		//TODO
	}
}

void app_hf_audio_transfer()
{
	/*sco and hfp are independent, cancel association  by hualiangyu 20150828*/
	hlp_hf_audio_transfer();
}

void app_hf_hangup()
{
	if (hlp_hf_get_status() > ST_HF_CONNECTED) {
		if (!hlp_hf_exist_3rd_call()) {
			HFP_HF_CancelCall(hlp_hf_get_handle());
			hlp_hf_set_status(ST_HF_HUNGUPING);
		} else {
			hlp_hf_cancel_active_call();
			HFP_HF_CancelCall(hlp_hf_get_handle());
		}

	} else {
		//TODO
	}
}

void app_hf_voice_recognition_req(UINT8 ctl)
{
	HFP_HF_VoiceRecognitionReq(hlp_hf_get_handle(), ctl);
	hlp_hf_event_OK();
}

#ifdef CONFIG_HF_SUPPORT_3_WAY_CALL
void app_hf_3_way_control(UINT8 idx, UINT8 ctl)
{
	if (hlp_hf_get_status() > ST_HF_CONNECTED) {
		hlp_hf_3_way_control(idx, ctl);
	}
}
#endif

void app_hf_query_status(void)
{
    hlp_hf_query_status();
}

profile_t g_hf_profile = {
	app_hf_init,
	app_hf_done,
	app_hf_server_start,
	app_hf_server_stop,
};


