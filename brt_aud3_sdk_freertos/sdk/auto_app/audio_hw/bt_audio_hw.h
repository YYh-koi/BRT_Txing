#ifndef A2DP_SINK_HW_H_
#define A2DP_SINK_HW_H_
#include "btdatatype.h"
#include "brtbt_interface.h"

enum {
    AUDIO_TYPE_SCO,
    AUDIO_TYPE_A2DP,
};

typedef struct tag_audio_a2dp_callback {
	int (*open_audio)(int sample);
	void (*close_audio)(int type);
	void (*start_audio)();
	void (*write_audio)(const void *buffer, unsigned int size);
	void (*read_audio)(const void *buffer, unsigned int size);
	void (*stop_audio)();
	int (*is_audio_stopped)();
	void (*audio_track_switch)(int val);
} audio_a2dp_callbacks_t;

typedef int (*sco_data_send_cb)(int handle, const void *buffer, unsigned int size);

typedef struct tag_audio_sco_param {
	int handle;
	unsigned char is_wbs;
} audio_sco_param_t;

typedef struct tag_audio_sco_callback {
	void (*init)(sco_data_send_cb cb);
	void (*done)();
	int (*open_audio)(int handle, unsigned char is_wbs);
	void (*close_audio)();
	void (*write_audio)(const void *buffer, unsigned int size);
	void (*read_audio)(const void *buffer, unsigned int size);
	void (*audio_recoder_switch)(int val);
} audio_sco_callbacks_t;


void	bt_porting_register_play_state_callback_internal(bt_play_state_callback cb );
void bt_porting_register_a2dp_data_callback_internal(bt_pcm_data_callback cb);
void bt_porting_register_hfp_spk_callback_internal(bt_pcm_data_callback cb);

void app_register_audio_a2dp_callback(audio_a2dp_callbacks_t *cb);
void app_register_audio_sco_callback(audio_sco_callbacks_t *cb);


int send_sco_data(uint16_t handle, const void *data, unsigned int len);
void bt_porting_audio_on(unsigned char type, int val);
void app_set_micmute_stat(int flag);
void bt_porting_audio_switch(unsigned char type, int val);
void bt_porting_audio_open(unsigned char type, void *param);
void bt_porting_audio_close(unsigned char type);
void bt_porting_audio_start(unsigned char type);
void bt_porting_audio_pause(unsigned char type);
void bt_porting_audio_cfg(unsigned char type, unsigned char *param);
void bt_porting_audio_data_ind(unsigned char type, unsigned char *data, unsigned int len);
void bt_porting_audio_set_gain(unsigned char type, float gain);
int bt_porting_audio_get_session_id(unsigned char type);

#endif

