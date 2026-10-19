#include "porting.h"
#include "bt_audio_hw.h"
#include "audio_player.h"
#include "audio_recorder.h"
#include "audio_utils/resampler.h"
#include "utils.h"
#ifdef CONFIG_OS_ANDROID
#include <cutils/log.h>
#endif
#include <pthread.h>
#ifdef SIMPLE_AEC_BY_VAD
#include "vad.h"
#endif
#ifdef CONFIG_AUDIO_STREAM_IPC
#include "audio_stream_ipc.h"
#endif
#include "brt_trace.h"

/* If define VARIABLE_MIN_TX_FRAME:
 * At the begin time, choose a smaller min buffer to obtain data from the recorder buffer.
 * Then if there is enough data in the recorder buffer(return value of recorder->read() is
 * much larger then the requested length), choose a bigger buffer to obtain data. */
//#define VARIABLE_MIN_TX_FRAME

// enable trace SCO data to file
//#define OUTPUT_SCO_DATA
//#define REDIRECT_LOG_INTO_FILE
//#define LOOPBACK_TEST

static long long record_time = 0;

#ifndef REDIRECT_LOG_INTO_FILE
#define LOG(fmt, ...) bse_log_info("audio_sco_hw", fmt, ## __VA_ARGS__)
#else
#include <stdarg.h>
FILE * log_fp = 0;
static void LOG(const char *fmt, ...)
{
	if (log_fp) {
		char buf[256];
		snprintf(buf, 256, "%ld: ", (long) (get_current_time() - record_time));
		fwrite(buf, 1, strlen(buf), log_fp);
		va_list ap;
		va_start(ap,fmt);
		vsnprintf(buf, 256, fmt, ap);
		fwrite(buf, 1, strlen(buf), log_fp);
		fwrite("\n", 1, 1, log_fp);
		va_end(ap);
	}
}
#endif

#ifdef LOOPBACK_TEST
static void loopback_open();
static int loopback_cb(int handle, const void *buffer, unsigned int size);
static void loopback_close();
#endif

// TODO move into struct
static int sco_sample_rate  = 8000;
#define DEFAULT_BT_CAPTURE_SAMPLE_RATE 22050
#define DEFAULT_CHANNEL_RECORD 2
#define DEFAULT_CHANNEL 2
#define DEFAULT_FORMAT_PCM 2 /* 16 bits */
#define DEFAULT_SAMPLE_INTERVAL 60  /* 60ms */
#define CACHE_AUDIO_DELAY_MS 15

enum {
    AUDIO_SCO_MUTE,
    AUDIO_SCO_UNMUTE
};

static unsigned char s_is_audio_sco_mute = AUDIO_SCO_UNMUTE;

//#define PLAYER_BUFFER_SIZE (4 * 1323) // 60ms
#define PLAYER_BUFFER_SIZE (DEFAULT_BT_CAPTURE_SAMPLE_RATE * DEFAULT_CHANNEL * DEFAULT_FORMAT_PCM * DEFAULT_SAMPLE_INTERVAL / 1000)
#define RECORD_BUFFER_SIZE (DEFAULT_BT_CAPTURE_SAMPLE_RATE * DEFAULT_CHANNEL_RECORD * DEFAULT_FORMAT_PCM * DEFAULT_SAMPLE_INTERVAL / 1000)
#define MIN_RECORD_BUFFER_SIZE (RECORD_BUFFER_SIZE / 2)//FIXME

#define CONFIG_SCO_TO_CONTROLLER_IOSYNC
#define CONFIG_SCO_TO_CONTROLLER_USING_MTU

#ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU
static int sco_to_controller_mtu = 0;
#endif

#ifndef RECORD_RING_BUFFER
#define AUDIO_RECORDER_CREATE audio_recorder_create
#define AUDIO_RECORDER_FREE   audio_recorder_free
#else
#define AUDIO_RECORDER_CREATE audio_recorder_wrapper_create
#define AUDIO_RECORDER_FREE   audio_recorder_wrapper_free
#endif

#ifndef CONFIG_OS_ANDROID
#ifdef CONFIG_OS_LINUX
#define create_resampler(...) ((void *) 0)
#define release_resampler(...) ((void *) 0)
#endif
#endif

#include "wav_file.h"
static WAV_FILE_HANDLE sco_fp = 0;

#ifdef OUTPUT_SCO_DATA
#include "porting.h"
static char farend_record_fpath[128];
static char nearend_record_fpath[128];
#define get_sco_file_path() do { \
	const char *config_dir = get_configure_path(CONFIGURATION_TYPE_READWRITE); \
	if (config_dir) { \
		snprintf(farend_record_fpath, 128, "%s/farend.wav", config_dir); \
		snprintf(nearend_record_fpath, 128, "%s/nearend.wav", config_dir); \
	} else {\
		strcpy(farend_record_fpath, "farend.wav"); \
		strcpy(nearend_record_fpath, "nearend.wav"); \
	} \
} while (0);

#define sco_file_open wav_file_open
#define sco_file_close wav_file_close
#define sco_file_write wav_file_write
#else
#define sco_file_open(...) ((void *) 0)
#define sco_file_close(...) ((void *) 0)
#define sco_file_write(...) ((void *) 0)
#define get_sco_file_path(...) ((void *) 0)
#endif

typedef struct tag_audio_sco_block {
	sco_data_send_cb            sco_send_cb;
	int                         sco_on;
	unsigned short              sco_handle;

	pthread_t                   sco_read_thread;
	pthread_mutex_t             mutex;
	pthread_cond_t              cond;

	pthread_mutex_t             player_mutex;
	audio_player_t *            player;
	unsigned char               buffer[2 * PLAYER_BUFFER_SIZE];
	unsigned char               rsmp_buf[RECORD_BUFFER_SIZE];
	struct resampler_itfe *     resampler;
#ifdef VARIABLE_MIN_TX_FRAME
	unsigned char               tx_stream_synced;
#endif
} audio_sco_block_t;

static audio_sco_block_t s_inst;

static inline int calc_aggressive_ratio(int b, int s)
{
	if (b > s * 16) {
		return 4;
	} else if (b > s * 8) {
		return 3;
	} else if (b > s * 4) {
		return 2;
	} else if (b > s * 2) {
		return 1;
	}

	return 0;
}

static void *sco_read_proc(void *param)
{
	int default_buffer_size = RECORD_BUFFER_SIZE;
	int buffer_size = MIN_RECORD_BUFFER_SIZE;
	int read_size;
	int aggressive = 0;
	unsigned char buffer[RECORD_BUFFER_SIZE];
	int buf_len;
	long long start_time = 0;
	record_time = 0;
	int capture_frame_size = DEFAULT_CHANNEL_RECORD * DEFAULT_FORMAT_PCM;
	int sco_frame_size = 1 * DEFAULT_FORMAT_PCM;
	/* xxx_unit: milliseconds needed to transmit ONE byte. */
	double capture_unit = ((double) 1000000) / DEFAULT_BT_CAPTURE_SAMPLE_RATE / capture_frame_size;
	double sco_unit = ((double) 1000000) / sco_sample_rate / sco_frame_size;
#ifdef CONFIG_SCO_TO_CONTROLLER_IOSYNC
	long expected_time, elapsed = 0;
#endif
	size_t rsmp_in_frames;
	size_t record_frames;
	unsigned char *record_buf;

	WAV_FILE_HANDLE fp = sco_file_open(nearend_record_fpath, sco_sample_rate, 1, 16);
#ifndef CONFIG_AUDIO_STREAM_IPC
	audio_recorder_t *recorder_default = AUDIO_RECORDER_CREATE(AUDIO_RECORDER_TYPE_DEFAULT,
	                                     DEFAULT_BT_CAPTURE_SAMPLE_RATE, DEFAULT_CHANNEL_RECORD, &default_buffer_size);

	default_buffer_size *= capture_frame_size;
	if (!recorder_default) {
		LOG("ERROR:  recorder_default is wrong");
		return NULL;
	}

	recorder_default->start(recorder_default);
	LOG("Enter %s thread. min_frame_count:%d. avail:%d", __FUNCTION__, default_buffer_size, recorder_default->avail(recorder_default));
#endif
	int delay_cnt = 0;
	do {
		usleep(1000);
	} while (delay_cnt++ < CACHE_AUDIO_DELAY_MS && s_inst.sco_on);

	do {

#ifdef CONFIG_SCO_TO_CONTROLLER_IOSYNC
		if (record_time > 0) {
			expected_time = (long) (buffer_size * capture_unit);
			elapsed = get_current_time() - record_time;

			if (expected_time > elapsed && !aggressive) {
				usleep(expected_time - elapsed);
			}
		}

		record_time = get_current_time();
#endif

#ifdef VARIABLE_MIN_TX_FRAME
		if (aggressive) {
			if (!s_inst.tx_stream_synced) {
				buffer_size += MIN_RECORD_BUFFER_SIZE;
				LOG("increase buffer_size to %d.", buffer_size);
			}
			if (buffer_size >= RECORD_BUFFER_SIZE) {
				s_inst.tx_stream_synced = 1;
			}
		}
#endif

#ifndef CONFIG_AUDIO_STREAM_IPC
		read_size = recorder_default->avail(recorder_default);
		buffer_size = recorder_default->read(recorder_default, buffer, MIN_RECORD_BUFFER_SIZE);
#else
		buffer_size = audio_stream_ipc_recv_data(buffer, MIN_RECORD_BUFFER_SIZE);
#endif
		elapsed = (long)(get_current_time() - record_time);
		if (elapsed > 100) {
//			LOG("read_size: %d, buffer_size: %d. used %ld.", read_size, buffer_size, elapsed);
		}
		aggressive = calc_aggressive_ratio(read_size, default_buffer_size);
		if (aggressive) {
			LOG("aggressive :%d.", aggressive);
		}
		rsmp_in_frames = buffer_size / capture_frame_size;
		record_frames  = rsmp_in_frames;

		/* 4-->2 */
		if (DEFAULT_CHANNEL_RECORD / 4) {
			convert_quad_to_stereo(buffer, record_frames);
		}

		pthread_mutex_lock(&s_inst.mutex);
		if (s_inst.resampler != NULL) {
			s_inst.resampler->resample_from_input(s_inst.resampler,
			                                      (int16_t *) buffer,
			                                      &rsmp_in_frames,
			                                      (int16_t *) s_inst.rsmp_buf,
			                                      &record_frames);
			record_buf = s_inst.rsmp_buf;
		} else {
			record_buf = buffer;
			record_frames = rsmp_in_frames;
		}
		pthread_mutex_unlock(&s_inst.mutex);

		buf_len = record_frames * capture_frame_size;
		if (DEFAULT_CHANNEL_RECORD / 2) {
			convert_stereo_to_mono(record_buf, record_frames);
			buf_len = record_frames * 2;
		}

		if ((fp != WAV_FILE_INVALID_HANDLE) && (fp != 0)) {
			sco_file_write(fp, record_buf, buf_len);
		}

		if (s_inst.sco_send_cb && s_inst.sco_handle > 0) {
			int l, wlen = buf_len;
			int offset = 0;

			if (sco_to_controller_mtu == 0) {
				continue;
			}

#ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU
			do {
				l = wlen > sco_to_controller_mtu ? sco_to_controller_mtu : wlen;
#ifdef SIMPLE_AEC_BY_VAD
				if (!vad_is_speech())
#endif
				{
					if (s_is_audio_sco_mute == AUDIO_SCO_UNMUTE) { /* use for mic mute */
						s_inst.sco_send_cb(s_inst.sco_handle, record_buf + offset, l);
					}
				}
				wlen -= l;
				offset += l;

#ifdef CONFIG_SCO_TO_CONTROLLER_IOSYNC
				elapsed = (long) (get_current_time() - record_time);
				expected_time = (long) (offset * sco_unit) * (aggressive ? (1.0f - 0.05f * aggressive) : 1.0f);
				//expected_time = (long) (offset * sco_unit) - 100; /* Execute code exist delay */

				if (wlen > 0 && elapsed < expected_time) {
					/*LOG("usleep:%ld after send sco:offset:%d, len:%d.", (expected_time - elapsed), offset, l);*/
					usleep(expected_time - elapsed);
				}
#endif
			} while (s_inst.sco_on && wlen > 0);

#else /* #ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU */
#ifdef SIMPLE_AEC_BY_VAD
			if (!vad_is_speech())
#endif
			{
				if (s_is_audio_sco_mute == AUDIO_SCO_UNMUTE) { /* use for mic mute */
					s_inst.sco_send_cb(s_inst.sco_handle, record_buf, buf_len);
				}
			}
#endif /* #ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU*/
		}

	} while (s_inst.sco_on);

	LOG("Leave thread:%s(%ld)", __FUNCTION__, pthread_self());

	if ((fp != WAV_FILE_INVALID_HANDLE) && (fp != 0)) {
		sco_file_close(fp);
	}

#ifndef CONFIG_AUDIO_STREAM_IPC
	recorder_default->stop(recorder_default);
	AUDIO_RECORDER_FREE(recorder_default);
#endif
	return NULL;
}

static void sco_on_state_changed(int connected)
{
	pthread_mutex_lock(&s_inst.mutex);
	s_inst.sco_on = connected;

#ifdef SIMPLE_AEC_BY_VAD
	if (s_inst.sco_on) {
		vad_init();
	} else {
		vad_destroy();
	}
#endif

	pthread_cond_signal(&s_inst.cond);
	pthread_mutex_unlock(&s_inst.mutex);
}

static void init(sco_data_send_cb cb)
{
#ifdef LOOPBACK_TEST
	s_inst.sco_send_cb = loopback_cb;
#else
	s_inst.sco_send_cb = cb;
#endif
	s_is_audio_sco_mute = AUDIO_SCO_UNMUTE;
	pthread_mutex_init(&s_inst.mutex, NULL);
	pthread_mutex_init(&s_inst.player_mutex, NULL);
	pthread_cond_init(&s_inst.cond, NULL);
}

static void done()
{
	s_inst.sco_send_cb = 0;
	s_is_audio_sco_mute = AUDIO_SCO_MUTE;
	pthread_mutex_destroy(&s_inst.mutex);
	pthread_mutex_destroy(&s_inst.player_mutex);
	pthread_cond_destroy(&s_inst.cond);
}

static int open_audio(int handle, unsigned char is_wbs)
{
	if(s_inst.sco_on == 1){
		LOG("audio has opened, return");
		return -1;
	}
	LOG("open_audio(handle:%d, sample_rate:%d)", handle, sco_sample_rate);
	s_inst.sco_handle = handle;
	sco_on_state_changed(1);
#ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU
	sco_to_controller_mtu = 0;
#endif
#ifdef LOOPBACK_TEST
	loopback_open();
#endif
#ifdef REDIRECT_LOG_INTO_FILE
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	const char *config_dir = get_configure_path(CONFIGURATION_TYPE_READWRITE);

	if (config_dir) {
		char log_fn[128];
		snprintf(log_fn, 128, "%s/audio_log_%d-%d-%d_%d_%d_%d.txt", config_dir, 
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		log_fp = fopen(log_fn, "w");
	}
#endif
	get_sco_file_path();
	pthread_create(&s_inst.sco_read_thread, NULL, sco_read_proc, NULL);

	if (is_wbs) {
		sco_sample_rate = 16000;
	} else {
		sco_sample_rate = 8000;
	}

#ifdef VARIABLE_MIN_TX_FRAME
	s_inst.tx_stream_synced = 0;
#endif

	sco_fp = sco_file_open(farend_record_fpath, sco_sample_rate, 1, 16);

	if (DEFAULT_BT_CAPTURE_SAMPLE_RATE != sco_sample_rate) {
		create_resampler(DEFAULT_BT_CAPTURE_SAMPLE_RATE,
		                 sco_sample_rate,
		                 DEFAULT_CHANNEL,
		                 RESAMPLER_QUALITY_DEFAULT,
		                 NULL, //struct resampler_buffer_provider *provider,
		                 &s_inst.resampler);

		if (s_inst.resampler != NULL) {
			s_inst.resampler->reset(s_inst.resampler);
		}
	}
#ifndef CONFIG_AUDIO_STREAM_IPC
	pthread_mutex_lock(&s_inst.player_mutex);
	s_inst.player = audio_player_create(AUDIO_PLAYER_TYPE_DEFAULT, sco_sample_rate, DEFAULT_CHANNEL, PLAYER_BUFFER_SIZE);
	if (s_inst.player) {
		s_inst.player->start(s_inst.player);
	}
	pthread_mutex_unlock(&s_inst.player_mutex);
#endif
	return 0;
}

static void close_audio()
{
	sco_on_state_changed(0);

	if ((sco_fp != WAV_FILE_INVALID_HANDLE) && (sco_fp != 0)) {
		sco_file_close(sco_fp);
	}

	sco_fp = 0;

#ifndef CONFIG_AUDIO_STREAM_IPC
	pthread_mutex_lock(&s_inst.player_mutex);
	if (s_inst.player) {
		s_inst.player->stop(s_inst.player);
		audio_player_free(s_inst.player);
		s_inst.player = 0;
	}
	pthread_mutex_unlock(&s_inst.player_mutex);
#endif
#ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU
	sco_to_controller_mtu = 0;
#endif

	pthread_mutex_lock(&s_inst.mutex);
	if (s_inst.resampler) {
		release_resampler(s_inst.resampler);
		s_inst.resampler = NULL;
	}
	pthread_mutex_unlock(&s_inst.mutex);
#ifdef REDIRECT_LOG_INTO_FILE
	/* FIXME if NOT call pthread_join, calling "LOG(...)" in the leaving position of sco_read_proc may crash
	 * as the "log_fp" may have be destroyed. So we should always call pthread_join(move it outside the macro)? */
	pthread_join(s_inst.sco_read_thread, NULL);
	if (log_fp) {
		fclose(log_fp);
		log_fp = 0;
	}
#endif
#ifdef LOOPBACK_TEST
	loopback_close();
#endif
}

static void write_audio(const void *buffer, unsigned int size)
{
#ifdef LOOPBACK_TEST
	return;
#endif
	unsigned char *pbuf = (unsigned char *) buffer;
	int wlen = size;
	int len = 0;

#ifdef CONFIG_SCO_TO_CONTROLLER_USING_MTU
	if (sco_to_controller_mtu != size) {
		sco_to_controller_mtu = size;
	}
#endif


#ifdef SIMPLE_AEC_BY_VAD
	vad_check_speech(buffer, size);
#endif

#ifndef CONFIG_AUDIO_STREAM_IPC
	pthread_mutex_lock(&s_inst.player_mutex);
	if (!s_inst.player) {
		pthread_mutex_unlock(&s_inst.player_mutex);
		return;
	}
#endif

	do {
		len = wlen > PLAYER_BUFFER_SIZE ? PLAYER_BUFFER_SIZE : wlen;
		memset(s_inst.buffer, 0, PLAYER_BUFFER_SIZE);
		memcpy(s_inst.buffer, pbuf, len);

#ifndef CONFIG_AUDIO_STREAM_IPC
		if (DEFAULT_CHANNEL == 2) {
			convert_mono_to_stereo(s_inst.buffer, len / 2);
			s_inst.player->write(s_inst.player, s_inst.buffer, len * 2);
		} else {
			s_inst.player->write(s_inst.player, s_inst.buffer, len);
		}

#else
		if ((sco_fp != WAV_FILE_INVALID_HANDLE) && (sco_fp != 0)) {
			sco_file_write(sco_fp, s_inst.buffer, len);
		}
		audio_stream_ipc_send_data(AUDIO_STREAM_TYPE_SCO, 0, s_inst.buffer, len);
#endif
		pbuf += len;
		wlen -= len;
	} while (wlen > 0);
#ifndef CONFIG_AUDIO_STREAM_IPC
	pthread_mutex_unlock(&s_inst.player_mutex);
#endif
}

static void audio_recoder_switch(int is_on)
{
	s_is_audio_sco_mute = is_on;
}

audio_sco_callbacks_t g_sco_callbacks = {
	.init = init,
	.done = done,
	.open_audio = open_audio,
	.close_audio = close_audio,
	.write_audio = write_audio,
	.audio_recoder_switch = audio_recoder_switch,
};


#ifdef LOOPBACK_TEST
static void loopback_open()
{
	sco_to_controller_mtu = 60;
}

static int loopback_cb(int handle, const void *buffer, unsigned int size)
{
	unsigned char *pbuf = (unsigned char *) buffer;
	int wlen = size;
	int len = 0;

	pthread_mutex_lock(&s_inst.player_mutex);
	if (!s_inst.player) {
		pthread_mutex_unlock(&s_inst.player_mutex);
		return 0;
	}

	do {
		len = wlen > PLAYER_BUFFER_SIZE ? PLAYER_BUFFER_SIZE : wlen;
		memset(s_inst.buffer, 0, PLAYER_BUFFER_SIZE);
		memcpy(s_inst.buffer, pbuf, len);

		if (DEFAULT_CHANNEL == 2) {
			convert_mono_to_stereo(s_inst.buffer, len / 2);
			s_inst.player->write(s_inst.player, s_inst.buffer, len * 2);
		} else {
			s_inst.player->write(s_inst.player, s_inst.buffer, len);
		}

		pbuf += len;
		wlen -= len;
	} while (wlen > 0);
	pthread_mutex_unlock(&s_inst.player_mutex);

	return size;
}

static void loopback_close()
{
}
#endif

