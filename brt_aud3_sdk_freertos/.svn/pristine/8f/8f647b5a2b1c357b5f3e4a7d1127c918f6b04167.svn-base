#include "tinyalsa/asoundlib.h"
#include "pcm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <cutils/sched_policy.h>
#include "brt_trace.h"
#include "sysdep.h"
#include "bt_audio_hw.h"
#include "bt_pcm.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1
#define LOG_TAG "pcm_record"
#define OUTPUT_PCM_DATA 0

struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

typedef struct {
	unsigned int card;
	unsigned int device;
	unsigned int channels;
	unsigned int rate;
	unsigned int bits;
	unsigned int period_size;
	unsigned int period_count;
}pcm_config_stru;

static int is_capturing = 0;
pcm_config_stru pcm_configs;
struct pcm *s_pcm_record = NULL;

unsigned int capture_sample(FILE *file,unsigned int card, unsigned int device,
							unsigned int channels, unsigned int rate,
							enum pcm_format format, unsigned int period_size,
							unsigned int period_count);

void close_pcm_capture()
{
	if (s_pcm_record)
	{
		bse_log_info(LOG_TAG, "close_pcm_capture +!");
		is_capturing = 0;
		pcm_stop(s_pcm_record);
		pcm_close(s_pcm_record);
		s_pcm_record = NULL;
		bse_log_info(LOG_TAG, "close_pcm_capture -");
	}
}

static bool s_bThreadExist = FALSE;
void cap_pcm()
{
	FILE *file = NULL;
	struct wav_header header;
	unsigned int frames;
	enum pcm_format format;
	format = PCM_FORMAT_S16_LE;

	while (s_bThreadExist || !get_sco_state()) {
		usleep(50*1000);
		bse_log_info(LOG_TAG, "[warning]thread exist");
	}
	
	is_capturing = 1;
	s_bThreadExist = TRUE;
	
	bse_log_info(LOG_TAG, "cap_pcm+");
#ifdef OUTPUT_PCM_DATA
	file = fopen("/data/misc/brlink/pcm_in.wav","wb+");
	//file = fopen("/data/misc/brlink/pcm_in.wav","ab+");
	if (!file) {
		bse_log_info(LOG_TAG, "Unable to create file ");
	}
#endif
	header.riff_id=ID_RIFF;
	header.riff_sz=0;
	header.riff_fmt=ID_WAVE;
	header.fmt_id=ID_FMT;
	header.fmt_sz=16;
	header.audio_format= FORMAT_PCM;
	header.num_channels = pcm_configs.channels;
	header.sample_rate = pcm_configs.rate;
	header.bits_per_sample = pcm_format_to_bits(format);
	header.byte_rate = (header.bits_per_sample/8) * pcm_configs.channels * pcm_configs.rate;
	header.block_align = pcm_configs.channels *(header.bits_per_sample/8);
	header.data_id = ID_DATA;
	if (file) {
		fseek(file,sizeof(struct wav_header),SEEK_SET);
	}

#if 1
	int tid = gettid();
	int rc = 0;
	set_sched_policy(tid, SP_AUDIO_SYS);
	if (rc) {
		bse_log_info(LOG_TAG, "failed to change sched policy, tid %d, err: %d", tid,
			errno);
	}
	{
		struct sched_param rt_params;
		rt_params.sched_priority = 1;
		rc = sched_setscheduler(tid, SCHED_FIFO, &rt_params);
		if (rc != 0) {
			bse_log_info(LOG_TAG,
				"%s unable to set SCHED_FIFO priority %d for tid %d, error %s",
				__func__, 1, tid, strerror(errno));
		}
	}
#endif

	frames=capture_sample(file,pcm_configs.card,pcm_configs.device, pcm_configs.channels, pcm_configs.rate, format, pcm_configs.period_size, pcm_configs.period_count);
#ifdef OUTPUT_PCM_DATA
	header.data_sz = frames*header.block_align;
	header.riff_sz = header.data_sz +sizeof(header)-8;
	if (file) {
		fseek(file,0,SEEK_SET);
		fwrite(&header,sizeof(struct wav_header),1,file);
		fclose(file);
	}
#endif
	s_bThreadExist = FALSE;
	is_capturing = 0;
	bse_log_info(LOG_TAG, "cap_pcm-");
}

extern void reset_audio_player();
extern void get_sco_state();
unsigned int capture_sample(FILE *file,unsigned int card, unsigned int device,
							unsigned int channels, unsigned int rate,
							enum pcm_format format, unsigned int period_size,
							unsigned int period_count)
{
	struct pcm_config config;
	struct pcm *pcm;
	char *buffer;
	unsigned int size;
	unsigned int bytes_read = 0;
	int res = 0;
    UINT32 s_start_time = 0;

	config.channels = channels;
	config.rate = rate;
	config.period_size = period_size;
	config.period_count = period_count;
	config.format = format;
	config.start_threshold = 0;
	config.stop_threshold = 0;
	config.silence_threshold = 0;
	pcm = pcm_open(card, device, PCM_IN, &config);
	if (!pcm || !pcm_is_ready(pcm)) {
		bse_log_info(LOG_TAG, "###Unable to open PCM device!(%s)\n###",pcm_get_error(pcm));
		return 0;
	}
	bse_log_info(LOG_TAG, "pcm record open! pcm=0x%08x\n", pcm);

	size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

	buffer = malloc(size);
	if (!buffer) {
		bse_log_info(LOG_TAG, "Unabele to allocate %d bytes\n", size);
		free(buffer);
		pcm_close(pcm);
		return 0;
	}

	s_pcm_record = pcm;
	pcm_start(s_pcm_record);
    s_start_time = GetCurrTime();
	while (is_capturing && s_pcm_record) {
		UINT32 time = GetCurrTime();
        memset(buffer, 0, size);
		res = pcm_read(s_pcm_record, buffer, size);
        if (GetCurrTime() - time <= 5) {
		    bse_log_info(LOG_TAG, "read fail, cost %d.try reset audio player", GetCurrTime() - time);
            reset_audio_player();
        }
		if (!res) {
#if 0//for test
            if(GetCurrTime() - s_start_time >= 0 && GetCurrTime() - s_start_time <= 300)
                continue;
#endif      
            if(!is_capturing || !s_pcm_record)
                memset(buffer, 0, size);
			bt_porting_audio_data_ind(AUDIO_TYPE_SCO,buffer,size);
			bytes_read +=size;
#ifdef OUTPUT_PCM_DATA
			if(file && fwrite(buffer,1,size,file)!=size){
				bse_log_info(LOG_TAG, "Error capturing sample \n");
			}
#endif
		} else if (s_pcm_record){
			bse_log_info(LOG_TAG, "[waring][%d], %s ", is_capturing, pcm_get_error(s_pcm_record));
			usleep(10000);
		}
	}

	free(buffer);
	if (s_pcm_record)
		return pcm_bytes_to_frames(s_pcm_record, bytes_read);
	else 
		return 0;
}

void open_pcm_capture(unsigned int card, unsigned int device, unsigned int channels,
					  unsigned int rate, unsigned int bits, unsigned int period_size,
					  unsigned int period_count)
{
	pthread_t tid;

	memset(&pcm_configs, 0, sizeof(pcm_config_stru));
	pcm_configs.card = card;
	pcm_configs.device = device;
	pcm_configs.channels = channels;
	pcm_configs.rate = rate;
	pcm_configs.bits = bits;
	pcm_configs.period_size = period_size;
	pcm_configs.period_count = period_count;
	pthread_create(&tid, NULL, cap_pcm, NULL);
	pthread_detach(tid);
}

