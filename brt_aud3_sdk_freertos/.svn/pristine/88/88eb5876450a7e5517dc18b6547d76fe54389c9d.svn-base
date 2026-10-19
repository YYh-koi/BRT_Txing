#include "vendor_interface.h"
#include "brt_trace.h"

#define TAG "vendor_dummy"

#ifdef TAG
#define LOG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ##__VA_ARGS__)
#else
#define LOG(...) ((void*)0)
#endif
static void vendor_dummy_init(vendor_interface_callback event_cbk);
static void vendor_dummy_done();
static void vendor_dummy_pre_reset();
static void vendor_dummy_post_reset();
static void vendor_dummy_cmd_cbk(void *context, UINT8 result, void *param);
static void vendor_dummy_event_ind(void *param, UINT32 size);
static void vendor_dummy_on_gap_started();
static char* vendor_dummy_get_firmware_version();
static UINT8* vendor_dummy_get_device_info(); /* get device information(Local config, Remote device list) from EEPROM.*/

static void vendor_dummy_ringtone_start(UINT8 count);
static void vendor_dummy_ringtone_stop();
static void vendor_dummy_ringtone_close();
static void vendor_dummy_on_sco_established(UINT16 handle);
static void vendor_dummy_on_sco_released();
static void vendor_dummy_on_codec_configured(UINT8 select_codec);

static void vendor_dummy_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle);
static void vendor_dummy_on_stream_start();
static void vendor_dummy_on_stream_suspend();
static void vendor_dummy_on_stream_done();
static void vendor_dummy_on_stream_configured(UINT8 codec, UINT16 sample);

static void vendor_dummy_set_mic_gain(UINT8 gain);
static void vendor_dummy_get_mic_gain();
static void vendor_dummy_mic_control(UINT8 flag); /* mute/unmute */
static void vendor_dummy_set_local_volume(UINT8 volume);
static int vendor_dummy_get_local_volume();
static int vendor_dummy_update_audio_param();
static void vendor_dummy_wbs_config(UINT8 val);

static vendor_interface_t s_vendor_itf;

vendor_interface_t *vendor_interface_create()
{
	memset(&s_vendor_itf, 0, sizeof(vendor_interface_t));

	s_vendor_itf.vendor_init = vendor_dummy_init;
	s_vendor_itf.vendor_done = vendor_dummy_done;
	s_vendor_itf.vendor_pre_reset = vendor_dummy_pre_reset;
	s_vendor_itf.vendor_post_reset = vendor_dummy_post_reset;
	s_vendor_itf.vendor_cmd_cbk = vendor_dummy_cmd_cbk;
	s_vendor_itf.vendor_event_ind = vendor_dummy_event_ind;
	s_vendor_itf.vendor_on_gap_started = vendor_dummy_on_gap_started;
	s_vendor_itf.vendor_get_firmware_version = vendor_dummy_get_firmware_version;
	s_vendor_itf.vendor_get_device_info = vendor_dummy_get_device_info;

	s_vendor_itf.vendor_ringtone_start = vendor_dummy_ringtone_start;
	s_vendor_itf.vendor_ringtone_stop = vendor_dummy_ringtone_stop;
	s_vendor_itf.vendor_ringtone_close = vendor_dummy_ringtone_close;
	s_vendor_itf.vendor_on_sco_established = vendor_dummy_on_sco_established;
	s_vendor_itf.vendor_on_sco_released = vendor_dummy_on_sco_released;
	s_vendor_itf.vendor_on_codec_configured = vendor_dummy_on_codec_configured;

	s_vendor_itf.vendor_on_stream_open = vendor_dummy_on_stream_open;
	s_vendor_itf.vendor_on_stream_start = vendor_dummy_on_stream_start;
	s_vendor_itf.vendor_on_stream_suspend = vendor_dummy_on_stream_suspend;
	s_vendor_itf.vendor_on_stream_done = vendor_dummy_on_stream_done;
	s_vendor_itf.vendor_on_stream_configured = vendor_dummy_on_stream_configured;

	s_vendor_itf.vendor_set_mic_gain = vendor_dummy_set_mic_gain;
	s_vendor_itf.vendor_get_mic_gain = vendor_dummy_get_mic_gain;
	s_vendor_itf.vendor_mic_control = vendor_dummy_mic_control;
	s_vendor_itf.vendor_set_local_volume = vendor_dummy_set_local_volume;
	s_vendor_itf.vendor_get_local_volume = vendor_dummy_get_local_volume;
	s_vendor_itf.vendor_update_audio_param = vendor_dummy_update_audio_param;
	s_vendor_itf.vendor_wbs_config = vendor_dummy_wbs_config;

	return &s_vendor_itf;
}

void vendor_interface_destroy()
{
	vendor_dummy_done();
}

static vendor_interface_callback s_event_cbk = NULL;
static void vendor_dummy_event_indcate(UINT32 event, void *param);

static void vendor_dummy_init(vendor_interface_callback callback)
{
	s_event_cbk = callback;
}

static void vendor_dummy_done()
{
}

static void vendor_dummy_pre_reset()
{
}

static void vendor_dummy_post_reset()
{
	vendor_dummy_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) TRUE);
}

static void vendor_dummy_cmd_cbk(void *context, UINT8 result, void *param)
{
}

static void vendor_dummy_event_ind(void *param, UINT32 size)
{
}

static void vendor_dummy_on_gap_started()
{
}

static char* vendor_dummy_get_firmware_version()
{
	return NULL;
}

static UINT8* vendor_dummy_get_device_info()
{
	return NULL;
}

static void vendor_dummy_ringtone_start(UINT8 count)
{
}

static void vendor_dummy_ringtone_stop()
{
}

static void vendor_dummy_ringtone_close()
{
}

static void vendor_dummy_on_sco_established(UINT16 handle)
{
}

static void vendor_dummy_on_sco_released()
{
}

static void vendor_dummy_on_codec_configured(UINT8 select_codec)
{
}


static void vendor_dummy_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle)
{
}

static void vendor_dummy_on_stream_start()
{
}

static void vendor_dummy_on_stream_suspend()
{
}

static void vendor_dummy_on_stream_done()
{
}

static void vendor_dummy_on_stream_configured(UINT8 codec, UINT16 sample)
{
}

static void vendor_dummy_event_indcate(UINT32 event, void *param)
{
	if (s_event_cbk != NULL) {
		s_event_cbk(event, param);
	}
}

static void vendor_dummy_set_mic_gain(UINT8 gain)
{
}

static void vendor_dummy_get_mic_gain()
{
}

static void vendor_dummy_mic_control(UINT8 flag)
{
}

static void vendor_dummy_set_local_volume(UINT8 volume)
{
}

static int vendor_dummy_get_local_volume()
{
	return 0;
}

static int vendor_dummy_update_audio_param()
{
	return 0;
}
static void vendor_dummy_wbs_config(UINT8 val)
{
}

