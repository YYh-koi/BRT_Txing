#include "vad.h"
#include "btdatatype.h"
#include <stdint.h>
#include <speex/speex_preprocess.h>
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("SIMPLE_AEC", fmt, ## __VA_ARGS__)

#define CONFIG_VAD_SIMPLE

#define DEFAULT_BT_CAPTURE_SAMPLE_RATE 8000
static int was_speech = 0;
static int silent2speech = 0;
static int speech2silent = 0;
static SpeexPreprocessState * speex_state = NULL;
#define SPEEX_BUF_SIZE (DEFAULT_BT_CAPTURE_SAMPLE_RATE / 100)
static spx_int16_t speex_buffer[SPEEX_BUF_SIZE]; // 10ms
static int sp_idx = 0;
/* 250 ms delay */
#define SPEECH_DETECT_DELAY (SPEEX_BUF_SIZE * 25)
static int speech_delay_frames = 0;
static int silent_delay_frames = 0;

static int speech_packets = 0;
static int silent_packets = 0;


void vad_init()
{
	spx_int32_t vad = 1;
	speex_state = speex_preprocess_state_init(SPEEX_BUF_SIZE, DEFAULT_BT_CAPTURE_SAMPLE_RATE);
	speex_preprocess_ctl(speex_state, SPEEX_PREPROCESS_SET_VAD, &vad);
	vad = 95; /* default value: 35 */
	speex_preprocess_ctl(speex_state, SPEEX_PREPROCESS_SET_PROB_START, &vad);
	vad = 50; /* default value: 20 */
	speex_preprocess_ctl(speex_state, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad);

	sp_idx = 0;
	was_speech = 0;
	silent2speech = 0;
	speech2silent = 0;
	speech_delay_frames = 0;
	silent_delay_frames = 0;
}

void vad_destroy()
{
	if (speex_state != NULL) {
		speex_preprocess_state_destroy(speex_state);
		speex_state = NULL;
	}
}

int vad_check_speech(const void *buffer, unsigned int size)
{
	int i, frames = size / 2;

#ifdef CONFIG_VAD_SIMPLE
	int count = 0;
	int16_t *p = (int16_t *) buffer;
	for(i=0; i<frames; i++) {
		if ((p[i] & 0xFF00) == 0xFF00)
			continue;

		if ((p[i] & 0xFF00) > 0x8000)
			count++;
	}

	if (count * 4 > frames) {
		speech_packets++;
		silent_packets = 0;
	} else {
		silent_packets++;
	}

	if (speech_packets > 0) {
		speech_packets = 0;
		was_speech = 1;
		LOG("silent_packets:%d. speech!", speech_packets);
	}

	if (silent_packets > 20) {
		speech_packets = 0;
		silent_packets = 0;
		was_speech = 0;
		LOG("silent_packets :%d. silent!", silent_packets);
	}
#else
	int remain = 0;
	int ws = 0;
	spx_int16_t *p = (spx_int16_t *) buffer;

	if (speex_state == NULL) {
		return was_speech;
	}

	if (sp_idx < SPEEX_BUF_SIZE) {
		int len = SPEEX_BUF_SIZE - sp_idx;
		len = len < frames ? len : frames;

		memcpy(speex_buffer + sp_idx, p, len * sizeof(spx_int16_t));
		sp_idx += len;
		p += len;
		remain = frames - len;
	}

	if (sp_idx >= SPEEX_BUF_SIZE) {
		ws = speex_preprocess_run(speex_state, speex_buffer);
		LOG("ws:%d, was_speech:%d", ws, was_speech);
		sp_idx = 0;

		if (ws) {
			speech_packets++;
			silent_packets = 0;
		} else {
			silent_packets++;
		}

		if ((!silent2speech)  && (ws == 1) && (was_speech == 0)) {
			silent2speech = 1;
			speech_delay_frames = 0;
		}

		if ((!speech2silent)  && (ws == 0) && (was_speech == 1)) {
			speech2silent = 1;
			silent_delay_frames = 0;
		}

		if (!silent2speech && !speech2silent) {
			was_speech = ws;
		}
	}

	if (remain > 0) {
		memcpy(speex_buffer + sp_idx, p, remain * sizeof(spx_int16_t));
		sp_idx += remain;
	}

	if (silent2speech) {
		speech_delay_frames += frames;

		if (speech_delay_frames >= SPEECH_DETECT_DELAY) {
			was_speech = 1;
			silent2speech = 0;
			speech_delay_frames = 0;
		}
	}
	if (speech2silent) {
		silent_delay_frames += frames;

		if (silent_delay_frames >= SPEECH_DETECT_DELAY) {
			was_speech = 0;
			speech2silent = 0;
			silent_delay_frames = 0;
		}
	}
#endif

	return was_speech;
}

int vad_is_speech()
{
	return was_speech;
}

