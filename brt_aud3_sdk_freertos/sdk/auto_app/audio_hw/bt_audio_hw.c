#include "btdatatype.h"
#include <stdio.h>
#include <string.h>
#include "bt_audio_hw.h"
#include "sysdep.h"
#include "porting_utils.h"
#include "tl.h" /* FIMXE */

#include "global.h"
#include <errno.h>
#include <pthread.h>
#include "internal_hf.h"
#include "auto_hf.h"

#include "audio_ring_buffer.h"

#include "bt_event.h"

#define MODULE_TAG "bt_audio_hw"
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("bt_audio_hw", fmt, ## __VA_ARGS__)

#define SCO_PKT_PREFIX_LEN 4
#define SCO_SEND_BUFFER_SIZE 256
#define SCO_SEND_DATA_SIZE (SCO_SEND_BUFFER_SIZE - SCO_PKT_PREFIX_LEN)

#define SCO_PKT_DEFAULT_SIZE 48
static unsigned char tx_sco_send_buf[SCO_SEND_DATA_SIZE];

static audio_sco_callbacks_t *s_sco_callback;
static int micmute_f = 0;
static int s_sco_pkt_len = SCO_PKT_DEFAULT_SIZE;
static unsigned char s_sco_send_buf[SCO_SEND_BUFFER_SIZE];

static int s_sco_sample = 8000;
static UINT16 s_sco_handle = 0;

static audio_a2dp_callbacks_t *s_a2dp_callback;
extern BOOL s_a2dp_mute;
static int is_paused = 0;
static int s_a2dp_sample = 44100;

#ifdef BRT_DBG
#undef BRT_DBG
#define BRT_DBG(...) ((void *) 0)
#endif

#define HAL_CBACK(P_CB, P_CBACK, ...)           \
    if (P_CB && P_CB->P_CBACK) {                \
        BRT_DBG("HAL %s->%s", #P_CB, #P_CBACK); \
        P_CB->P_CBACK(__VA_ARGS__);             \
    }                                           \
    else {                                      \
        BRT_DBG("%s", "Callback is NULL"); \
    }


static bt_play_state_callback s_play_state_cb = NULL;
void	bt_porting_register_play_state_callback_internal(bt_play_state_callback cb )
{
	s_play_state_cb = cb;
}

static bt_pcm_data_callback s_a2dp_data_cb = NULL;
void bt_porting_register_a2dp_data_callback_internal(bt_pcm_data_callback cb)
{
	s_a2dp_data_cb = cb;
}

static bt_pcm_data_callback s_hfp_spk_cb = NULL;
void bt_porting_register_hfp_spk_callback_internal(bt_pcm_data_callback cb)
{
	s_hfp_spk_cb = cb;
}

static void set_paused(int val)
{
#ifdef CHECK_EMPTY_DATA
    is_paused = val;
#endif
}

int send_sco_data(UINT16 handle, const void *data, unsigned int len)
{
    //LOG("play_sample len:%d\r\n",len);
#ifdef CONFIG_SCO_OVER_UART
    int slen = len;
    unsigned char *pdata = (unsigned char *) data;
    handle = s_sco_handle;

    if (handle == 0 || data == NULL || len == 0) {
        LOG("invalid param: handle:%x, len:%d, s_sco_pkt_len : %d.", handle, len, s_sco_pkt_len);
		return 0;
	}
	memset(s_sco_send_buf, 0, SCO_SEND_BUFFER_SIZE);
	s_sco_send_buf[0] = 3;
	s_sco_send_buf[1] = (unsigned char) handle & 0xFF;
	s_sco_send_buf[2] = (unsigned char) ((handle >> 8) & 0xFF);

//	LOG("handle:%x, len:%d, s_sco_pkt_len : %d.", handle, len, s_sco_pkt_len);
	s_sco_send_buf[3] = slen;
	memcpy(s_sco_send_buf + SCO_PKT_PREFIX_LEN, pdata, slen);
	/* FIXME Do NOT call TL function */
	hlp_tl_send_data(handle, s_sco_send_buf, slen + SCO_PKT_PREFIX_LEN);
#endif
	return 1;
}


static void bt_porting_audio_track_switch(int val)
{
	
}
static void bt_porting_audio_recorder_switch(int val)
{
	
}
void bt_porting_audio_switch(unsigned char type, int val)
{
	if (type == AUDIO_TYPE_A2DP) {
		bt_porting_audio_track_switch(val);
	} else { /* AUDIO_TYPE_SCO */
		bt_porting_audio_recorder_switch(val);
	}
}
void bt_porting_audio_open(unsigned char type, void *param)
{
	LOG("%s type = %d",__func__, type);
	if (type == AUDIO_TYPE_SCO) {
		audio_sco_param_t *sco_param = (audio_sco_param_t *) param;
		//HAL_CBACK(s_sco_callback, open_audio, sco_param->handle, sco_param->is_wbs);
		s_sco_handle = sco_param->handle;
		if (sco_param->is_wbs) {
			s_sco_sample = 16000;
		} else {
			s_sco_sample = 8000;
		}
		if(s_play_state_cb != NULL) {
			s_play_state_cb(BT_HFP_PLAY_START, s_sco_sample, 1);
		}
	} else {
		set_paused(0);
		//HAL_CBACK(s_a2dp_callback, open_audio, s_a2dp_sample);
		//s_play_state_cb(BT_A2DP_PLAY_START, s_a2dp_sample, 2); //cbk when start
	}
}

void bt_porting_audio_close(unsigned char type)
{
	LOG("bt_porting_audio_close -------------------------------- type = %d \n", type);

	if(type == AUDIO_TYPE_SCO) {
		//HAL_CBACK(s_sco_callback, close_audio);
		if(s_play_state_cb != NULL) {
			s_play_state_cb(BT_HFP_PLAY_STOP, s_sco_sample, 1);
		}
	} else {
		set_paused(0);
		//HAL_CBACK(s_a2dp_callback, close_audio, AUDIO_TYPE_A2DP);
		if(s_play_state_cb != NULL) {
			s_play_state_cb(BT_A2DP_PLAY_STOP, s_a2dp_sample, 2);
		}
	}
}

void bt_porting_audio_start(unsigned char type)
{
	if(type == AUDIO_TYPE_SCO) {
		//TODO
	} else {
		set_paused(0);
		//HAL_CBACK(s_a2dp_callback, start_audio);
		if(s_play_state_cb != NULL) {
			s_play_state_cb(BT_A2DP_PLAY_START, s_a2dp_sample, 2);
		}
	}
}

void bt_porting_audio_pause(unsigned char type)
{
	if(type == AUDIO_TYPE_SCO) {
		//TODO
	} else {
		set_paused(1);
		//HAL_CBACK(s_a2dp_callback, stop_audio);
		if(s_play_state_cb != NULL) {
			s_play_state_cb(BT_A2DP_PLAY_STOP, s_a2dp_sample, 2);
		}
	}
}

void bt_porting_audio_cfg(unsigned char type, unsigned char *param)
{
	s_a2dp_sample = (int) param;
}

extern UINT8 abs_volume;
extern UINT8 iphone_flag_g;
void bt_porting_audio_data_ind(unsigned char type, unsigned char *data, unsigned int len)
{
	if (type == AUDIO_TYPE_A2DP) {
		if (!is_paused) {
			//HAL_CBACK(s_a2dp_callback, write_audio, data, len);
			if(s_a2dp_data_cb != NULL) {
				LOG("bt_porting_audio_data_ind iphone_flag_g = %d abs_volume = %d\n", iphone_flag_g, abs_volume);

				if(iphone_flag_g && (abs_volume >= 0)) {
					
					//bt_event_indicate(BT_EV_PRT_DEBUG_LOG, NULL);
					float tmpe_abs_volume = abs_volume;
					int samplesize = 2;//double audio
					int i;
					for(i=0; i<len/samplesize; i++) {
						
						float tmpe_data = ((int16_t *)data)[i] * (tmpe_abs_volume / 127);

						data[i*2] = ((int16_t)tmpe_data) & 0xff;
						data[i*2 + 1] = (((int16_t)tmpe_data) >> 8) & 0xff;
					}
				}			
				s_a2dp_data_cb(data, len);
			}
		}
	} else {
			//HAL_CBACK(s_sco_callback, write_audio, data, len);
			if(s_hfp_spk_cb != NULL) {
				s_hfp_spk_cb(data, len);
			}
	}
}

void app_register_audio_sink_callback(audio_a2dp_callbacks_t *cb)
{
	s_a2dp_callback = cb;
}

void app_register_audio_sco_callback(audio_sco_callbacks_t *cb)
{
	printf("app_register_audio_sco_callback------------------------------\n");
#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_SCO
	s_sco_callback = cb;
	if (cb) {
		HAL_CBACK(s_sco_callback, init, send_sco_data);
	} else {
		HAL_CBACK(s_sco_callback, done, send_sco_data);
	}
#endif
}

int app_get_micmute_stat()
{
	//printf("micmute_f = %d\n", micmute_f);
	return micmute_f;
}

void app_set_micmute_stat(int flag)
{
	printf("app_set_micmute_stat flag = %d\n", flag);
	micmute_f = flag;
}
