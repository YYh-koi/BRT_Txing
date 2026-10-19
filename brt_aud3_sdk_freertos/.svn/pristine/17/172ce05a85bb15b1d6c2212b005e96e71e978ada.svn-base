/**
  * This file defines the abstract interfaces which each vendor chip may implement.
  *
  * Created 2016-03-16 09:21:22 by zmj
  *
  */

#ifndef _VENDOR_INTERFACE_H__
#define _VENDOR_INTERFACE_H__

#include "btdatatype.h"

#define MAX_LOCAL_FIRMWARE_VERSION_LEN  8

enum {
    VENDOR_EV_POST_RESET_HARDWARE_FINISHED,     /* param: BOOL(result) */
    VENDOR_EV_MIC_GAIN,                         /* param: UINT8(mic gain value) */
    VENDOR_EV_AUDIO_PARAMETER_UPDATED,          /* param: UINT8(result) */
};

typedef void (*vendor_interface_callback)(UINT8 event, void *param);

typedef struct tag_vendor_interface {
	/* interfaces used in GAP layer.*/
	void (*vendor_init)(vendor_interface_callback event_cbk);
	void (*vendor_done)();
	void (*vendor_pre_reset)();
	void (*vendor_post_reset)();
	void (*vendor_cmd_cbk)(void *context, UINT8 result, void *param);
	void (*vendor_event_ind)(void *param, UINT32 size);
	void (*vendor_on_gap_started)();
	char* (*vendor_get_firmware_version)();
	UINT8* (*vendor_get_device_info)(); /* get device information(Local config, Remote device list) from EEPROM.*/

	/* interfaces used in HFP layer.*/
	void (*vendor_ringtone_start)(UINT8 count);
	void (*vendor_ringtone_stop)();
	void (*vendor_ringtone_close)();
	void (*vendor_on_sco_established)(UINT16 handle);
	void (*vendor_on_sco_released)();
	void (*vendor_on_codec_configured)(UINT8 select_codec);

	/* interfaces used in A2DP layer.*/
	void (*vendor_on_stream_open)(UINT16 l2cap_cid, UINT16 acl_handle);
	void (*vendor_on_stream_start)();
	void (*vendor_on_stream_suspend)();
	void (*vendor_on_stream_done)();
	void (*vendor_on_stream_configured)(UINT8 codec, UINT16 sample);

	/* Audio interfaces. */
	void (*vendor_set_mic_gain)(UINT8 gain);
	void (*vendor_get_mic_gain)();
	void (*vendor_mic_control)(UINT8 flag); /* mute/unmute */
	void (*vendor_set_local_volume)(UINT8 volume);
	int (*vendor_get_local_volume)();
	int (*vendor_update_audio_param)();
	void (*vendor_wbs_config)(UINT8 enabled);

} vendor_interface_t;

vendor_interface_t *vendor_interface_create();
void vendor_interface_destroy();

#endif  /*_VENDOR_INTERFACE_H__*/
