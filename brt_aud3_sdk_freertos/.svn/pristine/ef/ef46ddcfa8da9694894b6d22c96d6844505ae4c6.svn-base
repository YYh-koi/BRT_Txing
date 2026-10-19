#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "brt_trace.h"
#include "sysdep.h"
#include "bt_pcm.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define LOG_TAG "pcm_player"

struct riff_wave_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t wave_id;
};

struct chunk_header {
	uint32_t id;
	uint32_t sz;
};

struct chunk_fmt {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

static struct pcm *s_pcm_player = NULL;

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
				char *param_name, char *param_unit)
{
	unsigned int min;
	unsigned int max;
	int is_within_bounds = 1;

	min = pcm_params_get_min(params, param);
	if (value < min) {
		bse_log_info(LOG_TAG, "%s is %u%s, device only supports >= %u%s\n", param_name, value,
			param_unit, min, param_unit);
		is_within_bounds = 0;
	}

	max = pcm_params_get_max(params, param);
	if (value > max) {
		bse_log_info(LOG_TAG, "%s is %u%s, device only supports <= %u%s\n", param_name, value,
			param_unit, max, param_unit);
		is_within_bounds = 0;
	}

	return is_within_bounds;
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
					   unsigned int rate, unsigned int bits, unsigned int period_size,
					   unsigned int period_count)
{
	struct pcm_params *params;
	int can_play;

	params = pcm_params_get(card, device, PCM_OUT);
	if (params == NULL) {
		fprintf(stderr, "Unable to open PCM device %u.\n", device);
		return 0;
	}

	can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
	can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
	can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
	can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", "Hz");
	can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", "Hz");

	pcm_params_free(params);

	return can_play;
}

void write_pcm_data(unsigned char *record_buf, int count)
{
	int ret = 0;
	if (s_pcm_player && count && pcm_is_ready(s_pcm_player)) {
		ret = pcm_write(s_pcm_player, record_buf,count);
		if (ret) {
			bse_log_info(LOG_TAG,"Error playing sample c %d ret %d(%s)\n", count, ret, pcm_get_error(s_pcm_player));
		}
	}
}

void open_pcm_write(unsigned int card, unsigned int device, unsigned int channels,
					unsigned int rate, unsigned int bits, unsigned int period_size,
					unsigned int period_count)
{
	struct pcm_config config = {0};
	config.channels = channels;
	config.rate = rate;
	config.period_size = period_size;
	config.period_count = period_count;
	if (bits == 32)
		config.format = PCM_FORMAT_S32_LE;
	else if (bits == 16)
		config.format = PCM_FORMAT_S16_LE;

	config.start_threshold = 0;
	config.stop_threshold = 0;
	config.silence_threshold = 0;
	if (!sample_is_playable(card, device, channels, rate, bits, period_size, period_count)) {
		bse_log_info(LOG_TAG, "sample_is_playable Failed\n");
		return;
	}

	s_pcm_player = pcm_open(card, device, PCM_OUT, &config);
	if (!s_pcm_player || !pcm_is_ready(s_pcm_player)) {
		bse_log_info(LOG_TAG, "Unable to open PCM device %d (%s)\n",
			device, pcm_get_error(s_pcm_player));
		return;
	}
	pcm_start(s_pcm_player);
	bse_log_info(LOG_TAG,"open_pcm_write, pcm = 0x%x\n", s_pcm_player);

}

void close_pcm_write()
{
	if (s_pcm_player) {
		pcm_stop(s_pcm_player);
		pcm_close(s_pcm_player);
	}
	s_pcm_player = NULL;
	bse_log_info(LOG_TAG,"close_pcm_write\n");
}
