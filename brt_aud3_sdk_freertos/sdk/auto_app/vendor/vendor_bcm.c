#include "vendor_interface.h"
#include "auto_gap.h"
#include "internal_gap.h"
#include "btdatatype.h"
#include "brt_trace.h"
#include "gap_ui.h"
#include "hci_stru.h"
#include "auto_commfunc.h"
#define TAG "vendor_43353"

#ifdef TAG
#define LOG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ##__VA_ARGS__)
#else
#define LOG(...) ((void*)0)
#endif

enum { /* A2DP ROLES */
    A2DP_ROLE_SOURCE,
    A2DP_ROLE_SINK
};

/*OCF for Brcm*/
#define BCM_VENDOR_CMD_WRITE_A2DP_PRIORITY   0x11A
#define BCM_VENDOR_CMD_WRITE_PCM_DATA_FORMAT 0X01E
#define BCM_VENDOR_CMD_WRITE_SCO_TIME_SLOT   0X022
#define BCM_VENDOR_CMD_WRITE_PCM_ENABLE_WBS  0X07E

static UINT16 s_acl_hdl = 0;

static void vendor_bcm_init(vendor_interface_callback event_cbk);
static void vendor_bcm_done();
static void vendor_bcm_pre_reset();
static void vendor_bcm_post_reset();
static void vendor_bcm_cmd_cbk(void *context, UINT8 result, void *param);
static void vendor_bcm_event_ind(void *param, UINT32 size);
static void vendor_bcm_on_gap_started();
static char* vendor_bcm_get_firmware_version();
static UINT8* vendor_bcm_get_device_info(); /* get device information(Local config, Remote device list) from EEPROM.*/

static void vendor_bcm_ringtone_start(UINT8 count);
static void vendor_bcm_ringtone_stop();
static void vendor_bcm_ringtone_close();
static void vendor_bcm_on_sco_established(UINT16 handle);
static void vendor_bcm_on_sco_released();
static void vendor_bcm_on_codec_configured(UINT8 select_codec);

static void vendor_bcm_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle);
static void vendor_bcm_on_stream_start();
static void vendor_bcm_on_stream_suspend();
static void vendor_bcm_on_stream_done();
static void vendor_bcm_on_stream_configured(UINT8 codec, UINT16 sample);

static void vendor_bcm_set_mic_gain(UINT8 gain);
static void vendor_bcm_get_mic_gain();
static void vendor_bcm_mic_control(UINT8 flag); /* mute/unmute */
static void vendor_bcm_set_local_volume(UINT8 volume);
static int vendor_bcm_get_local_volume();
static int vendor_bcm_update_audio_param();
static void vendor_bcm_wbs_config(BOOL val);

static vendor_interface_t s_vendor_itf;

vendor_interface_t *vendor_interface_create()
{
	memset(&s_vendor_itf, 0, sizeof(vendor_interface_t));

	s_vendor_itf.vendor_init = vendor_bcm_init;
	s_vendor_itf.vendor_done = vendor_bcm_done;
	s_vendor_itf.vendor_pre_reset = vendor_bcm_pre_reset;
	s_vendor_itf.vendor_post_reset = vendor_bcm_post_reset;
	s_vendor_itf.vendor_cmd_cbk = vendor_bcm_cmd_cbk;
	s_vendor_itf.vendor_event_ind = vendor_bcm_event_ind;
	s_vendor_itf.vendor_on_gap_started = vendor_bcm_on_gap_started;
	s_vendor_itf.vendor_get_firmware_version = vendor_bcm_get_firmware_version;
	s_vendor_itf.vendor_get_device_info = vendor_bcm_get_device_info;

	s_vendor_itf.vendor_ringtone_start = vendor_bcm_ringtone_start;
	s_vendor_itf.vendor_ringtone_stop = vendor_bcm_ringtone_stop;
	s_vendor_itf.vendor_ringtone_close = vendor_bcm_ringtone_close;
	s_vendor_itf.vendor_on_sco_established = vendor_bcm_on_sco_established;
	s_vendor_itf.vendor_on_sco_released = vendor_bcm_on_sco_released;
	s_vendor_itf.vendor_on_codec_configured = vendor_bcm_on_codec_configured;

	s_vendor_itf.vendor_on_stream_open = vendor_bcm_on_stream_open;
	s_vendor_itf.vendor_on_stream_start = vendor_bcm_on_stream_start;
	s_vendor_itf.vendor_on_stream_suspend = vendor_bcm_on_stream_suspend;
	s_vendor_itf.vendor_on_stream_done = vendor_bcm_on_stream_done;
	s_vendor_itf.vendor_on_stream_configured = vendor_bcm_on_stream_configured;

	s_vendor_itf.vendor_set_mic_gain = vendor_bcm_set_mic_gain;
	s_vendor_itf.vendor_get_mic_gain = vendor_bcm_get_mic_gain;
	s_vendor_itf.vendor_mic_control = vendor_bcm_mic_control;
	s_vendor_itf.vendor_set_local_volume = vendor_bcm_set_local_volume;
	s_vendor_itf.vendor_get_local_volume = vendor_bcm_get_local_volume;
	s_vendor_itf.vendor_update_audio_param = vendor_bcm_update_audio_param;
	s_vendor_itf.vendor_wbs_config = vendor_bcm_wbs_config;

	return &s_vendor_itf;
}

static void vendor_bcm_cmd_cbk(void *context, UINT8 result, void *cfm_par)
{
}

static void app_com_wirte_a2dp_connection_priority(UINT16 conn_hdl, BOOL is_high)
{
	unsigned char param_len = 4;
	unsigned char *param = NEW(param_len);
	unsigned short ocf = BCM_VENDOR_CMD_WRITE_A2DP_PRIORITY;
	unsigned char priority_val = is_high;
	unsigned char a2dp_role = A2DP_ROLE_SINK;

	param[0] = LOBYTE(conn_hdl);
	param[1] = HIBYTE(conn_hdl);
	param[2] = is_high;
	param[3] = a2dp_role;
	void *gap_handle = hlp_get_gap_handle();
	GAP_VendorCommandA(gap_handle, (void *)(UINT32) ocf, HCI_EVI_VENDOR, ocf, param, param_len, (GAP_VendorCommandCfmCbk *)vendor_bcm_cmd_cbk);

	FREE(param);
}

void vendor_interface_destroy()
{
	vendor_bcm_done();
}

static vendor_interface_callback s_event_cbk = NULL;
static void vendor_bcm_event_indcate(UINT32 event, void *param);

static void vendor_bcm_init(vendor_interface_callback callback)
{
	s_event_cbk = callback;
}

static void vendor_bcm_done()
{
}

static void vendor_bcm_pre_reset()
{
}

static void vendor_bcm_post_reset()
{
	vendor_bcm_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) TRUE);
}

static void vendor_bcm_event_ind(void *param, UINT32 size)
{
}

static void vendor_bcm_on_gap_started()
{
}

static char* vendor_bcm_get_firmware_version()
{
	return NULL;
}

static UINT8* vendor_bcm_get_device_info()
{
	return NULL;
}

static void vendor_bcm_ringtone_start(UINT8 count)
{
}

static void vendor_bcm_ringtone_stop()
{
}

static void vendor_bcm_ringtone_close()
{
}

static void vendor_bcm_on_sco_established(UINT16 handle)
{
}

static void vendor_bcm_on_sco_released()
{
}

static void vendor_bcm_on_codec_configured(UINT8 select_codec)
{
}


static void vendor_bcm_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle)
{
	s_acl_hdl = acl_handle;
	app_com_wirte_a2dp_connection_priority(s_acl_hdl, TRUE);
}

static void vendor_bcm_on_stream_start()
{
}

static void vendor_bcm_on_stream_suspend()
{
}

static void vendor_bcm_on_stream_done()
{
	app_com_wirte_a2dp_connection_priority(s_acl_hdl, FALSE);
	s_acl_hdl = 0;
}

static void vendor_bcm_on_stream_configured(UINT8 codec, UINT16 sample)
{
}

static void vendor_bcm_event_indcate(UINT32 event, void *param)
{
	if (s_event_cbk != NULL) {
		s_event_cbk(event, param);
	}
}

static void vendor_bcm_set_mic_gain(UINT8 gain)
{
}

static void vendor_bcm_get_mic_gain()
{
}

static void vendor_bcm_mic_control(UINT8 flag)
{
}

static void vendor_bcm_set_local_volume(UINT8 volume)
{
}

static int vendor_bcm_get_local_volume()
{
	return 0;
}

static int vendor_bcm_update_audio_param()
{
	return 0;
}

static void vendor_bcm_write_pcm_data_format()
{
	unsigned char param_len = 5;
	unsigned char *param = NEW(param_len);
	unsigned short ocf = BCM_VENDOR_CMD_WRITE_PCM_DATA_FORMAT;

	param[0] = 0x00;
	param[1] = 0x00;
	param[2] = 0x03;
	param[3] = 0x03;
	param[4] = 0x00;
	void *gap_handle = hlp_get_gap_handle();
	GAP_VendorCommandA(gap_handle, (void *)(UINT32) ocf, HCI_EVI_VENDOR, ocf, param, param_len, (GAP_VendorCommandCfmCbk *)vendor_bcm_cmd_cbk);

	FREE(param);
}

static void vendor_bcm_write_sco_time_slot()
{
	unsigned char param_len = 1;
	unsigned char *param = NEW(param_len);
	unsigned short ocf = BCM_VENDOR_CMD_WRITE_SCO_TIME_SLOT;

	param[0] = 0x00;
	void *gap_handle = hlp_get_gap_handle();
	GAP_VendorCommandA(gap_handle, (void *)(UINT32) ocf, HCI_EVI_VENDOR, ocf, param, param_len, (GAP_VendorCommandCfmCbk *)vendor_bcm_cmd_cbk);
}

static void vendor_bcm_enable_wbs(BOOL val)
{
	unsigned char param_len = 2;
	unsigned char *param = NEW(param_len);
	unsigned short ocf = BCM_VENDOR_CMD_WRITE_PCM_ENABLE_WBS;

	if(val == TRUE) {
		param[0] = 0x01;
	} else {
		param[0] = 0x00;
	}
	param[1] = 0x02;
	void *gap_handle = hlp_get_gap_handle();
	GAP_VendorCommandA(gap_handle, (void *)(UINT32) ocf, HCI_EVI_VENDOR, ocf, param, param_len, (GAP_VendorCommandCfmCbk *)vendor_bcm_cmd_cbk);
}

static void vendor_bcm_wbs_config(BOOL val)
{
	bse_log_info(TAG, "vendor_bcm_wbs_config Start");
	vendor_bcm_write_pcm_data_format();
	vendor_bcm_write_sco_time_slot();
	vendor_bcm_enable_wbs(val);
	bse_log_info(TAG, "vendor_bcm_wbs_config end");
}
