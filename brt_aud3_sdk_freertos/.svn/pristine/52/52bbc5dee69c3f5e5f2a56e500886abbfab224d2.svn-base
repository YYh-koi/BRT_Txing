#define TAG "hlp_csr_dsp_8350"

#include "internal_csr_dsp_8350.h"
#include "internal_stream.h"
#include "auto_stream.h"
#include "auto_a2dp.h" /*FIXME EV_A2DP_APTX_STATUS*/
#include "brt_trace.h"

#define TAG "bse_csr"
static UINT16 s_a2dp_cid = 0;
static UINT16 s_acl_hdl = 0;

void csr_dsp_8350_init()
{
	app_stream_init();
}

void csr_dsp_8350_on_hardware_reset()
{
	app_stream_set_status(APP_ST_STREAM_INIT);
	hlp_stream_set_type(STREAM_IDLE);
}

void csr_dsp_8350_on_gap_started()
{
	hlp_set_patchindex_0();
	hlp_audio_8350patch();
}

void csr_dsp_8350_start_ringtone()
{
#ifdef USEFSM_CONFIG_8350DSP
	bse_log_api(TAG, "[hlp_hf]ringind start ringtone");
	app_stream_open(STREAM_RINGTONE, (UINT8 *) hlp_findname(STREAM_RINGTONE), 0, 0);
	app_stream_start(STREAM_RINGTONE);
	app_stream_fadeout_ringtone();
#else
	hlp_audio_open(0, 0, STREAM_RINGTONE);
	hlp_audio_start(STREAM_RINGTONE);
#endif
}

void csr_dsp_8350_stop_ringtone()
{
#ifdef USEFSM_CONFIG_8350DSP
	app_stream_stop(STREAM_RINGTONE);
	app_stream_close(STREAM_RINGTONE, (UINT8 *) hlp_findname(STREAM_RINGTONE));
#else
	hlp_audio_stop(STREAM_RINGTONE);
#endif
}

void csr_dsp_8350_close_ringtone()
{
	app_stream_close_ringtone();
}

void csr_dsp_8350_on_sco_established(UINT16 sco_handle)
{
#ifdef USEFSM_CONFIG_8350DSP
	bse_log_api(TAG, "[dsp]hlp_hf sco connected");
	app_stream_open(STREAM_SCO, (UINT8 *) hlp_findname(STREAM_SCO), 0, sco_handle);
	app_stream_start(STREAM_SCO);
#else
	hlp_audio_open(uid, 0, STREAM_SCO);
	hlp_audio_start(STREAM_SCO);
#endif
}

void csr_dsp_8350_on_sco_released()
{
#ifdef USEFSM_CONFIG_8350DSP
	bse_log_api(TAG, "[dsp]hlp_hf sco disconnected set cvsd");
	//set_codec_type(HFP_CODECID_CVSD);
	app_stream_stop(STREAM_SCO);
	app_stream_close(STREAM_SCO, (UINT8 *) hlp_findname(STREAM_SCO));
#else
	hlp_audio_stop(STREAM_SCO);
#endif
}

void csr_dsp_8350_set_codec_type(UINT8 codec)
{
	set_codec_type(codec);
}

void csr_dsp_8350_on_vendor_event_cbk(void *par, UINT32 size)
{
	//for the "00 02" SETREQ_MSG from controller,BCCMD process,par from C3
	UINT8 ret = -1;
	UINT8 *cmd_par_vendor = (UINT8 *) NEW(size + 1);

	if (cmd_par_vendor != NULL) {
		cmd_par_vendor[0] = size;
		memcpy(cmd_par_vendor + 1, par, size);
		ret = hlp_audio_vendorCmd_getreq(cmd_par_vendor);
		FREE(cmd_par_vendor);
	}
}

void csr_dsp_8350_on_stream_open(UINT16 cid, UINT16 conn_hdl)
{
	s_a2dp_cid = cid;
	s_acl_hdl = conn_hdl;
	//a2dp_resrc to be ready, not use to re_load all resource.avoid hci data loopup
//	hlp_a2dp_dsp_ready(cid, conn_hdl);
	//do not operate the app_streamlist
	//app_a2dp_dsp_init(s_a2dp_cid, s_acl_hdl);
}

void csr_dsp_8350_on_stream_start()
{
	app_stream_open(STREAM_A2DP, (UINT8 *) hlp_findname(STREAM_A2DP), s_acl_hdl, s_a2dp_cid);
	app_stream_start(STREAM_A2DP);
}

void csr_dsp_8350_on_stream_suspend()
{
	app_stream_stop(STREAM_A2DP);
	app_stream_close(STREAM_A2DP, (UINT8 *) hlp_findname(STREAM_A2DP));
}

void csr_dsp_8350_on_stream_done()
{
	//when disconn need release all a2dp_dsp resource and clear app_streamlist
	int stream_type = app_stream_get_type();
	UINT8 stream_status = app_stream_get_status();
	bse_log_api(TAG, "[dsp]a2dp disc stream_type(%d),stream_status(%d)", stream_type, stream_status);

	if (stream_type == STREAM_A2DP) {
		//    app_a2dp_dsp_uninit();
		app_stream_stop(STREAM_A2DP);
		app_stream_close(STREAM_A2DP, (UINT8 *) hlp_findname(STREAM_A2DP));
	}
	//hlp_a2dp_dsp_done();
}


int csr_dsp_8350_on_stream_set_config(UINT8 codec, UINT16 sample)
{
	hlp_SetA2DPStream_AptxMode(codec == 0xFF ? TRUE : FALSE, sample);

	return 0;
}

int hlp_a2dp_get_cid(void)
{
	if (hlp_a2dp_get_status() > ST_A2DP_CONNECTING) {
		return s_a2dp_cid;
	}
	return 0;
}

