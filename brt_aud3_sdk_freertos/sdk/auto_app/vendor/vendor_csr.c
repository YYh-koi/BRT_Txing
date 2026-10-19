#include "vendor_interface.h"
#include "internal_csr_dsp_8350.h"
#include "internal_stream.h"
#ifdef CONFIG_BT_CHIP_CSR8350
#include "auto_stream.h"
#endif

#include "brt_trace.h"

#define TAG "vendor_csr"

#ifdef TAG
#define LOG(fmt, ...) bse_log_print(TAG, "\r\n", fmt, ##__VA_ARGS__)
#else
#define LOG(...) ((void*)0)
#endif

static void vendor_csr_init(vendor_interface_callback event_cbk);
static void vendor_csr_done();
static void vendor_csr_pre_reset();
static void vendor_csr_post_reset();
static void vendor_csr_cmd_cbk(void *context, UINT8 result, void *param);
static void vendor_csr_event_ind(void *param, UINT32 size);
static void vendor_csr_on_gap_started();
static char* vendor_csr_get_firmware_version();
static UINT8* vendor_csr_get_device_info(); /* get device information(Local config, Remote device list) from EEPROM.*/

static void vendor_csr_ringtone_start(UINT8 count);
static void vendor_csr_ringtone_stop();
static void vendor_csr_ringtone_close();
static void vendor_csr_on_sco_established(UINT16 handle);
static void vendor_csr_on_sco_released();
static void vendor_csr_on_codec_configured(UINT8 select_codec);

static void vendor_csr_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle);
static void vendor_csr_on_stream_start();
static void vendor_csr_on_stream_suspend();
static void vendor_csr_on_stream_done();
static void vendor_csr_on_stream_configured(UINT8 codec, UINT16 sample);

static void vendor_csr_set_mic_gain(UINT8 gain);
static void vendor_csr_get_mic_gain();
static void vendor_csr_mic_control(UINT8 flag); /* mute/unmute */
static void vendor_csr_set_local_volume(UINT8 volume);
static int vendor_csr_get_local_volume();
static int vendor_csr_update_audio_param();

static vendor_interface_t s_vendor_itf;

#define MAX_LOCAL_FIRMWARE_VERSION_LEN 8
#define PROJECT_VERSION_LEN 4
//usr for 8350
#define CSR_8350_FIRMWARE_VERSION_LEN 8
#define CSR_8350_FIRMWARE_VERSION_ADDR 0x02AE

static char s_firmware_version[MAX_LOCAL_FIRMWARE_VERSION_LEN];

vendor_interface_t *vendor_interface_create()
{
	memset(&s_vendor_itf, 0, sizeof(vendor_interface_t));

	s_vendor_itf.vendor_init = vendor_csr_init;
	s_vendor_itf.vendor_done = vendor_csr_done;
	s_vendor_itf.vendor_pre_reset = vendor_csr_pre_reset;
	s_vendor_itf.vendor_post_reset = vendor_csr_post_reset;
	s_vendor_itf.vendor_cmd_cbk = vendor_csr_cmd_cbk;
	s_vendor_itf.vendor_event_ind = vendor_csr_event_ind;
	s_vendor_itf.vendor_on_gap_started = vendor_csr_on_gap_started;
	s_vendor_itf.vendor_get_firmware_version = vendor_csr_get_firmware_version;
	s_vendor_itf.vendor_get_device_info = vendor_csr_get_device_info;

	s_vendor_itf.vendor_ringtone_start = vendor_csr_ringtone_start;
	s_vendor_itf.vendor_ringtone_stop = vendor_csr_ringtone_stop;
	s_vendor_itf.vendor_ringtone_close = vendor_csr_ringtone_close;
	s_vendor_itf.vendor_on_sco_established = vendor_csr_on_sco_established;
	s_vendor_itf.vendor_on_sco_released = vendor_csr_on_sco_released;
	s_vendor_itf.vendor_on_codec_configured = vendor_csr_on_codec_configured;

	s_vendor_itf.vendor_on_stream_open = vendor_csr_on_stream_open;
	s_vendor_itf.vendor_on_stream_start = vendor_csr_on_stream_start;
	s_vendor_itf.vendor_on_stream_suspend = vendor_csr_on_stream_suspend;
	s_vendor_itf.vendor_on_stream_done = vendor_csr_on_stream_done;
	s_vendor_itf.vendor_on_stream_configured = vendor_csr_on_stream_configured;

	s_vendor_itf.vendor_set_mic_gain = vendor_csr_set_mic_gain;
	s_vendor_itf.vendor_get_mic_gain = vendor_csr_get_mic_gain;
	s_vendor_itf.vendor_mic_control = vendor_csr_mic_control;
	s_vendor_itf.vendor_set_local_volume = vendor_csr_set_local_volume;
	s_vendor_itf.vendor_get_local_volume = vendor_csr_get_local_volume;
	s_vendor_itf.vendor_update_audio_param = vendor_csr_update_audio_param;

	return &s_vendor_itf;
}

static int process_get_chip_version_event(void *param)
{
	VENDOR_RESP *evt = NULL;
	UINT8 * e = (UINT8 *) param;

	if (!e) return 0;

	if (e[0] == 0xff) {
		evt = (VENDOR_RESP *)((UINT8*) param + 1);
	} else {
		evt = (VENDOR_RESP *)((UINT8* ) param);
	}

	if (evt->Varid == DSP_GET_CHIP_VERSION) {
		if (evt->Status == 0) {
			sprintf(s_firmware_version + PROJECT_VERSION_LEN, "%02x", evt->payload[0]);
		}

		return 1;
	} else {
		return 0;
	}
}

void vendor_save_project_version(char * project_ver)
{
	if (project_ver != NULL) {
		memcpy(s_firmware_version, project_ver, PROJECT_VERSION_LEN);
	}
}

void vendor_save_frimware_version(char * firmware_ver)
{
#ifdef CONFIG_BT_CHIP_CSR8350 
	if (firmware_ver != NULL)
	{
		memcpy(s_firmware_version, firmware_ver, strlen(firmware_ver));	
	}
#endif
}

void vendor_interface_destroy()
{
	vendor_csr_done();
}

static vendor_interface_callback s_event_cbk = NULL;
static void vendor_csr_event_indcate(UINT32 event, void *param);

static void vendor_csr_init(vendor_interface_callback event_cbk)
{
	s_event_cbk = event_cbk;
	memset(s_firmware_version, 0, sizeof(s_firmware_version));

#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_init();
#else
	//TODO
#endif
}

static void vendor_csr_done()
{
}

static void vendor_csr_pre_reset()
{
}

static void vendor_csr_post_reset()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_hardware_reset();
#else
	//TODO
#endif
	vendor_csr_event_indcate(VENDOR_EV_POST_RESET_HARDWARE_FINISHED, (void *) TRUE);
#ifdef CONFIG_BT_CHIP_CSR8350
    //get_chip_version();
	hlp_csr_get_pskey(CSR_8350_FIRMWARE_VERSION_ADDR, CSR_8350_FIRMWARE_VERSION_LEN);
#else
	//TODO
#endif
}

static void vendor_csr_cmd_cbk(void *context, UINT8 result, void *param)
{
#if 0 // add by cuifaxing
    if (process_get_chip_version_event(param))
        return;
#endif

#ifdef CONFIG_BT_CHIP_CSR8350
	hlp_audio_vendorCmd_resp(param);
#else
	//TODO
#endif
}

static void vendor_csr_event_ind(void *param, UINT32 size)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_vendor_event_cbk(param, size);
#else
	//TODO
#endif
}

static void vendor_csr_on_gap_started()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_gap_started();
#else
	//TODO
#endif
}

static char* vendor_csr_get_firmware_version()
{
	if (s_firmware_version[0] == 0) {
		return NULL;
	}
	return s_firmware_version;
}

static UINT8* vendor_csr_get_device_info()
{
	return NULL;
}

static void vendor_csr_ringtone_start(UINT8 count)
{
	// if (count == 0)
	{
#ifdef CONFIG_BT_CHIP_CSR8350
		csr_dsp_8350_start_ringtone();
#else
		//TODO
#endif
	}
}

static void vendor_csr_ringtone_stop()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_stop_ringtone();
#else
	//TODO
#endif
}

static void vendor_csr_ringtone_close()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_close_ringtone();
#else
	//TODO
#endif
}

static void vendor_csr_on_sco_established(UINT16 handle)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_sco_established(handle);
#else
	//TODO
#endif
}

static void vendor_csr_on_sco_released()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_sco_released();
#else
	//TODO
#endif
}

static void vendor_csr_on_codec_configured(UINT8 select_codec)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_set_codec_type(select_codec);
#else
	//TODO
#endif
}

static void vendor_csr_on_stream_open(UINT16 l2cap_cid, UINT16 acl_handle)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_stream_open(l2cap_cid, acl_handle);
#else
	//TODO
#endif
}

static void vendor_csr_on_stream_start()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_stream_start();
#else
	//TODO
#endif
}

static void vendor_csr_on_stream_suspend()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_stream_suspend();
#else
	//TODO
#endif
}

static void vendor_csr_on_stream_done()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_stream_done();
#else
	//TODO
#endif
}

static void vendor_csr_on_stream_configured(UINT8 codec, UINT16 sample)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	csr_dsp_8350_on_stream_set_config(codec, sample);
#else
	//TODO
#endif
}

static void vendor_csr_event_indcate(UINT32 event, void *param)
{
	if (s_event_cbk != NULL) {
		s_event_cbk(event, param);
	}
}

// TODO used in outside: hlp_stream ?!
UINT16 s_nFactoryMicGain = 0;

static void vendor_csr_set_mic_gain(UINT8 gain)
{
	/*
	   8350 mic setting
	   if set micvalue  == 0xff
	   so set Factory Default;
	   if set micvalue  >= 0x80
	   so set pre-amp bit;
	   mic gain value range : 0~23;
	 */
	UINT16 micGain = gain;
	if( micGain == 0xff) {
		micGain = s_nFactoryMicGain;
	} else if ( micGain >= 0x80) {
		micGain -= 0x80;
		micGain |= 0x8000;
	} else if(micGain >= 23) {
		return;
	}
#ifdef CONFIG_BT_CHIP_CSR8350
	hlp_SetLocalMicGain(micGain);
#else
	//TODO
#endif
}

static void vendor_csr_get_mic_gain()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	int gain = hlp_GetLocalMicGain();
	vendor_csr_event_indcate(VENDOR_EV_MIC_GAIN, (void *)(intptr_t) gain);
#else
	//TODO
#endif
}

static void vendor_csr_mic_control(UINT8 flag)
{
#ifdef CONFIG_BT_CHIP_CSR8350
	app_stream_mic_mute(flag);
#else
	//TODO
#endif
}

static void vendor_csr_set_local_volume(UINT8 volume)
{
	// 0~22 10";
#ifdef CONFIG_BT_CHIP_CSR8350
	hlp_SetLocalSpkGain(volume);
#else
	//TODO
#endif
}

static int vendor_csr_get_local_volume()
{
#ifdef CONFIG_BT_CHIP_CSR8350
	return hlp_GetLocalSpkGain();
#else
	//TODO
	return 0;
#endif
}

static int vendor_csr_update_audio_param()
{
	return 0;
}

