#include "porting.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include <alsa/asoundlib.h>
//#include "alsa/asoundlib.h"
#include "bt_audio_hw.h"
#include "audio_buf.h"
#include "btdatatype.h"
#include "autoconf_bt.h"
#include "porting.h"
#include "sysdep.h"
#include "porting_utils.h"

#define TAG "sco_pcm_hw"
#undef OUTPUT_SCO_DATA
#define CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_SCO

#undef CONFIG_BRT_AEC_PROCESSING_REVERSE

#ifdef CONFIG_BRT_AEC_PROCESSING
#include "vcp_internal.h"
#endif


audio_buffer_t *echo_buf;
audio_buffer_t *mic_buf;
audio_buffer_t *filter_buf;
audio_buffer_t *echo_reverse_buf;
#define ECHO_MAX_SIZE (2048 * 8)

char *mic_pdevice = "default"; //"hw:0,0";
char *spk_pdevice = "default"; //"hw:0,1";

struct sndpcm_s{
    struct snd_pcm_t *handle;
    snd_pcm_format_t format;
    int channels;
    snd_pcm_uframes_t chunk_size;
    snd_pcm_uframes_t buffer_size;
    size_t chunk_bytes;
    size_t bits_per_sample;
    size_t bits_per_frame;
}; 
typedef struct sndpcm_s sndpcm_t;

sndpcm_t *ms_mic_pcm;
sndpcm_t *ms_spk_pcm;
static SYSTEM_LOCK s_sco_write_lock = NULL;//add by qm[20191111]保护音频节点的操作间的互斥，打开关闭互斥。

//char *pdevice = "default";

static pthread_mutex_t s_mutex;
static snd_pcm_t *phandle = NULL;
static snd_pcm_hw_params_t *p_hwparams = NULL;
static snd_pcm_sw_params_t *p_swparams = NULL;


static void *aec_run(void *param);
static void close_snd_pcm(sndpcm_t *sndpcm);
static ssize_t write_pcm(sndpcm_t *sndpcm,  int16_t *buf, size_t count);
static ssize_t read_pcm(sndpcm_t *sndpcm, u_char *buf, size_t rcount);


int is_audio_sco_open = 0;
//static pthread_t s_aec_run_thread;

static int sco_sample_rate  = 8000;

enum {
	AUDIO_SCO_MUTE,
	AUDIO_SCO_UNMUTE
};
static unsigned char s_is_audio_sco_mute = AUDIO_SCO_UNMUTE;
static int s_sco_device_state = 0;//add by qm[20191111]for yuante of tengyun, save state of PCM device ,0 = close ,1 is open,2 is openning,3 is writeing

#ifdef OUTPUT_SCO_DATA
#include "wav_file.h"
static WAV_FILE_HANDLE sco_fp_farend = 0;
static WAV_FILE_HANDLE sco_fp_nearend = 0;
static WAV_FILE_HANDLE sco_fp_aec = 0;
#define FAREND_RECORD_FILE_NAME  "/data/misc/brlink/farend.wav"
#define NEAREND_RECORD_FILE_NAME "/data/misc/brlink/nearend.wav"
#define AEC_RECORD_FILE_NAME  "/data/misc/brlink/aec.wav"

#define sco_file_open wav_file_open
#define sco_file_close wav_file_close
#define sco_file_write wav_file_write
#else
#define sco_file_open(...) ((void *) 0)
#define sco_file_close(...) ((void *) 0)
#define sco_file_write(...) ((void *) 0)
#define get_sco_file_path(...) ((void *) 0)
#endif

sco_data_send_cb sco_send_cb;;
static void init(sco_data_send_cb cb)
{
   sco_send_cb = cb; 
}

/*
static void msleep(unsigned long timeout)
{
	struct timespec delay;
	int err;

	if (timeout == 0)
		return;

	delay.tv_sec = timeout / 1000;
	delay.tv_nsec = 1000 * 1000 * (timeout % 1000);

	do {
		err = nanosleep(&delay, &delay);
	} while (err < 0 && errno == EINTR);
}
*/
static void done()
{
    //TODO
}
static s_device_status = 0;//add by qm[20191011]Need to close SCO audio and then open a2dp audio,0 is close.
int get_device_audio_status(void)
{
    return s_device_status;
}

static int set_params(sndpcm_t *pcm_info, void * *buff, unsigned char is_wbs, int is_capture)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
    unsigned int period_time = 0;
    unsigned int buffer_time = 0;
    snd_pcm_uframes_t period_frames = 0;
    snd_pcm_uframes_t buffer_frames = 0;
    int monotonic = 0;
    int can_pause = 0;
    int start_delay = 0;
    int stop_delay = 0;
	int err;
    uint32_t exact_rate; 
	size_t n;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(pcm_info->handle, params);
	if (err < 0) {
        bse_log_print(TAG, "\r\n", "%s:[error]Broken configuration for this PCM: no configurations available", __func__);
		return -1;
	}

    err = snd_pcm_hw_params_set_access(pcm_info->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]Access type not available", __func__);
        return -1;
	}
	err = snd_pcm_hw_params_set_format(pcm_info->handle, params, pcm_info->format);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]Sample format non available", __func__);
        return -1;
	}
    bse_log_print(TAG, "\r\n", "%s:hwparams.channels=[%d]", __func__, pcm_info->channels);
	err = snd_pcm_hw_params_set_channels(pcm_info->handle, params, 1);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]Channels count non available", __func__);
        return -1;
	}

    if(is_wbs){
        exact_rate = 16000;
    }else{
        exact_rate = 8000;
    }
	rate = exact_rate;
	err = snd_pcm_hw_params_set_rate_near(pcm_info->handle, params, &exact_rate, 0);
    bse_log_print(TAG, "\r\n", "%s:hwparams.rate[%d]rate[%d]", __func__, exact_rate, rate);

    printf("rate = %d format = %d channels = %d\n", exact_rate, pcm_info->format, 1);

	rate = exact_rate;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
        printf("[wl_debug] buffer_time = %d\n", buffer_time);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0){
        //[wl_debug]err = snd_pcm_hw_params_set_period_time_near(pcm_info->handle, params,&period_time, 0);
    }else{
        //[wl_debug]err = snd_pcm_hw_params_set_period_size_near(pcm_info->handle, params,&period_frames, 0);
    }
    //[wl_debug]
    period_time = 21333;//21333
    printf("[wl_debug] period_time = %d\n", period_time);
	int period_time_dir = 0;
    //err = snd_pcm_hw_params_set_period_time_near(pcm_info->handle, params,&period_time, &period_time_dir);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_hw_params_set_period_time_near", __func__);
        return -1;
	}
    
    unsigned int periods = 960;
	int periods_dir = 0;
    err = snd_pcm_hw_params_set_period_size_near(pcm_info->handle, params,&periods, &periods_dir);
    err = snd_pcm_hw_params_set_periods_near(pcm_info->handle, params, &periods, &periods_dir);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_hw_params_set_period_size_near", __func__);
        return -1;
	}

	if (buffer_time > 0) {
		//[wl_debug]err = snd_pcm_hw_params_set_buffer_time_near(pcm_info->handle, params, &buffer_time, 0);
	} else {
		//[wl_debug]err = snd_pcm_hw_params_set_buffer_size_near(pcm_info->handle, params, &buffer_frames);
	}
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_hw_params_set_buffer_size_near", __func__);
        return -1;
	}
	monotonic = snd_pcm_hw_params_is_monotonic(params);
	can_pause = snd_pcm_hw_params_can_pause(params);
	err = snd_pcm_hw_params(pcm_info->handle, params);
	if (err < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]Unable to install hw params:", __func__);
        return -1;
	}
	snd_pcm_hw_params_get_period_size(params, &(pcm_info->chunk_size), 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (pcm_info->chunk_size == buffer_size) {
		bse_log_print(TAG, "\r\n", "%s:[error]Can't use period equal to buffer size", __func__);
        return -1;
	}
	snd_pcm_sw_params_current(pcm_info->handle, swparams);

	n = pcm_info->chunk_size;

    //n = 1000;

    printf("[wl_debug] chunk_size = %ld\n", n);

	if (snd_pcm_sw_params_set_avail_min(pcm_info->handle, swparams, n) < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_sw_params_set_avail_min", __func__);
        return -1;
	}

	/* round up to closest transfer boundary */
	n = buffer_size;
    start_delay = 1;
	if (start_delay <= 0) {
		start_threshold = n + (double) rate * start_delay / 1000000;
	} else{
        start_threshold = (double) rate * start_delay / 1000000;
    }
		
	if (start_threshold < 1){
		start_threshold = 1;
    }
	if (start_threshold > n){
		start_threshold = n;
    }

    //start_threshold = 2;

    printf("[wl_debug] start_threshold = %ld\n", start_threshold);

	if (snd_pcm_sw_params_set_start_threshold(pcm_info->handle, swparams, start_threshold) < 0) {
        printf("snd_pcm_sw_params_set_start_threshold_ERROR\n");
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_sw_params_set_start_threshold", __func__);
        return -1;
	}
	if (stop_delay <= 0){ 
		stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
    }else{
        stop_threshold = (double) rate * stop_delay / 1000000;
    }

	/*if (snd_pcm_sw_params_set_stop_threshold(pcm_info->handle, swparams, stop_threshold) < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_sw_params_set_stop_threshold", __func__);
        return -1;
	}[wl_debug]*/

	if (snd_pcm_sw_params(pcm_info->handle, swparams) < 0) {
		bse_log_print(TAG, "\r\n", "%s:[error]unable to install sw params", __func__);
        return -1;
	}

//	if (setup_chmap(pcm_info)){//@@@#????
//		bse_log_print(TAG, "\r\n", "%s:[error]setup_chmap", __func__);
//        return -1;
//    }
    pcm_info->bits_per_sample = snd_pcm_format_physical_width(pcm_info->format);
    pcm_info->bits_per_frame = pcm_info->bits_per_sample * pcm_info->channels;
    pcm_info->chunk_bytes = pcm_info->chunk_size * pcm_info->bits_per_frame / 8;

    if(is_capture && buff){
	    *buff = realloc(*buff, pcm_info->chunk_bytes);
    }

	buffer_frames = buffer_size;	/* for position test */
    bse_log_print(TAG, "\r\n", "%s:buffer_size[%d]pcm_info->chunk_bytes[%d]pcm_info->bits_per_sample[%d]pcm_info->bits_per_frame[%d]", __func__, 
        buffer_size, pcm_info->chunk_bytes, pcm_info->bits_per_sample,pcm_info->bits_per_frame);
}

static int open_audio(int handle, unsigned char is_wbs)
{
#if 1
    pthread_t tid;
	int ret = 0;
   char audio_hw_sco_mic[128] = {0};
   char audio_hw_sco_spk[128] = {0};
   snd_pcm_info_t *info = NULL;
    bse_log_print(TAG, "\r\n", "%s: in", __func__);

    if (is_wbs) {
        sco_sample_rate = 16000;
    } else {
        sco_sample_rate = 8000;
    }

   const char *config_fpath = compose_file_path(porting_get_configure_path(CONFIGURATION_TYPE_READONLY), TL_CONFIG_FILE);
   bse_log_print(TAG, "\r\n", "%s: config_fpath = %s", __func__, config_fpath);

//   ret = get_private_profile_string("local", "audio_hw_sco_mic", mic_pdevice, audio_hw_sco_mic, 128, config_fpath);
//   if (ret > 0) {
//        mic_pdevice = audio_hw_sco_mic;
//   }

   ret = get_private_profile_string("local", "audio_hw_sco_spk", spk_pdevice, audio_hw_sco_spk, 128, config_fpath);
   if (ret > 0) {
        spk_pdevice = audio_hw_sco_spk;
    }
    bse_log_print(TAG, "\r\n", "%s: mic_pdevice=%s, spk_pdevice=%s.\n", __func__, mic_pdevice, spk_pdevice);

    if(is_audio_sco_open == 1) {
        bse_log_print(TAG, "\r\n", "%s:audio_state%d", __func__, is_audio_sco_open);
        return;
    }

    if(NULL == s_sco_write_lock){
        s_sco_write_lock = CREATE_CRITICAL("sco_lock");
    }
    if(NULL == s_sco_write_lock){
        bse_log_print(TAG, "\r\n", "%s:luck is null", __func__);
        return ;
    }
    ENTER_CRITICAL(s_sco_write_lock);
#ifdef CONFIG_BRT_AEC_PROCESSING
    //brt_internal_vcp_create(sco_sample_rate, 1);
    vcp8_create(sco_sample_rate, 1);
#endif
    echo_buf = (audio_buffer_t *)malloc(sizeof(audio_buffer_t));
    mic_buf = (audio_buffer_t *)malloc(sizeof(audio_buffer_t));
    filter_buf = (audio_buffer_t *)malloc(sizeof(audio_buffer_t));
    echo_reverse_buf = (audio_buffer_t *)malloc(sizeof(audio_buffer_t));
    echo_buf->s16 = malloc(ECHO_MAX_SIZE);
    mic_buf->s16 = malloc(ECHO_MAX_SIZE);
    filter_buf->s16 = malloc(ECHO_MAX_SIZE);
    echo_reverse_buf->s16 = malloc(ECHO_MAX_SIZE);

    ms_mic_pcm = (sndpcm_t *)malloc(sizeof(sndpcm_t));
    ms_spk_pcm = (sndpcm_t *)malloc(sizeof(sndpcm_t));

    bse_log_print(TAG, "\r\n", "%s: %d", __func__, sco_sample_rate);
#ifdef OUTPUT_SCO_DATA
     sco_fp_nearend = sco_file_open(NEAREND_RECORD_FILE_NAME, sco_sample_rate, 1, 16);
     sco_fp_farend = sco_file_open(FAREND_RECORD_FILE_NAME, sco_sample_rate, 1, 16);
     sco_fp_aec = sco_file_open(AEC_RECORD_FILE_NAME, sco_sample_rate, 1, 16);
#endif

#ifdef CONFIG_AUDIO_DATA_TREATMENT_PLATFORM_SCO

    ms_mic_pcm->format = SND_PCM_FORMAT_S16_LE;
    ms_mic_pcm->channels = 1;
    ms_mic_pcm->chunk_size = 1024;

    ms_spk_pcm->format = SND_PCM_FORMAT_S16_LE;
    ms_spk_pcm->channels = 1;
    ms_spk_pcm->chunk_size = 1024;


//mic
//     if((ret = snd_pcm_open(&ms_mic_pcm->handle, mic_pdevice, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
//         bse_log_print(TAG, "\r\n", "%s:[error]snd_pcm_open local mic(%s) cap faild,%s", __func__, mic_pdevice, snd_strerror(ret));
//         LEAVE_CRITICAL(s_sco_write_lock);
//		 return -1;
//      }
//
//     if(set_params(ms_mic_pcm, &(mic_buf->s16), is_wbs, 1/*CAPTURE*/) < 0) {
//         bse_log_print(TAG, "\r\n", "%s:[error]setparams local mic(%s) cap faild", __func__, mic_pdevice);
//         LEAVE_CRITICAL(s_sco_write_lock);
//         return -1;
//     }
//     bse_log_print(TAG, "\r\n", "%s:%s local mic cap handle = %p", __func__, mic_pdevice, ms_mic_pcm->handle);

//local-SPk
    if((ret = snd_pcm_open(&ms_spk_pcm->handle,  spk_pdevice, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    //if((ret = snd_pcm_open(&ms_spk_pcm->handle,  spk_pdevice, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
       bse_log_print(TAG, "\r\n", "%s:snd_pcm_open %s cap faild,%s", __func__, spk_pdevice, snd_strerror(ret));
       LEAVE_CRITICAL(s_sco_write_lock);
       return -1;
    }

    if(set_params(ms_spk_pcm, NULL, 0/*is_wbs*/, 0) < 0) {
       bse_log_print(TAG, "\r\n", "%s:setparams local spk%s play faild", __func__, spk_pdevice);
       LEAVE_CRITICAL(s_sco_write_lock);
       return -1;
    }
    bse_log_print(TAG, "\r\n", "%s:%s local spk cap handle = %p", __func__, spk_pdevice, ms_spk_pcm->handle);
#endif
    is_audio_sco_open = 1;

    //pthread_create(&s_aec_run_thread, NULL, aec_run, NULL);
    //pthread_detach(tid);
    s_device_status = 1;
    LEAVE_CRITICAL(s_sco_write_lock);
    bse_log_print(TAG, "\r\n", "%s: is_audio_sco_open= %d",__func__, is_audio_sco_open);
	return 0;
    
#elif

    int ret = 0;
	char sink_hw_device[128] = {0};
	
	pthread_mutex_lock(&s_mutex);
	//memset(a2dp_buf, 0, sizeof(a2dp_buf));

	sco_sample_rate = sampleRate;

   const char *config_fpath = compose_file_path(porting_get_configure_path(CONFIGURATION_TYPE_READONLY), TL_CONFIG_FILE);
   bse_log_print(TAG, "\r\n", "%s: config_fpath = %s", __func__, config_fpath);

   ret = get_private_profile_string("local", "sink_hw", pdevice, sink_hw_device, 128, config_fpath);
   if (ret > 0) {
        pdevice = sink_hw_device;
    }
    bse_log_print(TAG, "\r\n", "%s: sink_hw pdevice=%s.\n", __func__, pdevice);

	printf("try to open sound card:%s.\n", pdevice);
	ret = snd_pcm_open(&phandle, pdevice, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	//ret = snd_pcm_open(&phandle, pdevice, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	bse_log_info(TAG, "enter snd_pcm_open sampleRate: %d", sampleRate);
	if (ret < 0)
	{
		bse_log_info(TAG, "enter snd_pcm_open ret: %d(%s)", ret, snd_strerror(ret));
		goto quit;
	}

	if ((ret = snd_pcm_hw_params_malloc(&p_hwparams)) < 0)
	{
		bse_log_info(TAG, "snd_pcm_hw_params_alloca failed");
		goto quit;
	}
        
	if ((ret = sethwparams(phandle, p_hwparams, sampleRate)) < 0)//sampleRate
	//if (sethwparams(phandle, p_hwparams) < 0)
	{
		bse_log_info(TAG, "set play hw params failed!");
		goto quit;
	}

	snd_pcm_sw_params_alloca(&p_swparams);

	if ((ret = setswparams(phandle, p_hwparams, p_swparams)) < 0)
	{
		bse_log_info(TAG, "set play sw params failed!");
		goto quit;
	}
   
	
	ret = snd_pcm_prepare(phandle);
	if (ret < 0)
	{
		bse_log_info(TAG, "snd_pcm_prepare error!");
		goto quit;
	}

	write_count = snd_pcm_poll_descriptors_count(phandle);
	if (write_count <= 0) {
		bse_log_info(TAG, "Invalid poll descriptors write_count");
		ret = -1;
		goto quit;
	}

	if (ufds != NULL)
	{
		bse_log_info(TAG, "enter open_audio_track ufds != NULL");
		free(ufds);
		ufds = NULL;
	}

	ufds = malloc(sizeof(struct pollfd) * write_count);
	
	if (ufds == NULL) {
		bse_log_info(TAG, "No enough memory");
		ret = -ENOMEM;
		goto quit;
	} 

	if ((ret = snd_pcm_poll_descriptors(phandle, ufds, write_count)) < 0)
	{
		bse_log_info(TAG, "snd_pcm_poll_descriptors error!");
		free(ufds);
		ufds = NULL;
		goto quit;
	}

quit:
	if (ret >= 0) {
		s_state = AUDIO_SINK_STATE_OPENED;
	}

	pthread_mutex_unlock(&s_mutex);
	return ret;
#endif
}


static void close_audio()
{
    bse_log_print(TAG, "\r\n", "%s++", __func__);
    if(is_audio_sco_open == 0) {
        bse_log_print(TAG, "\r\n", "%s:audio_state%d", __func__, is_audio_sco_open);
        return;
    }
    ENTER_CRITICAL(s_sco_write_lock);
    
    is_audio_sco_open = 0;

    //bse_log_print(TAG, "\r\n", "%s:wait aec_run exit", __func__);
    //pthread_join(s_aec_run_thread, NULL);
    bse_log_print(TAG, "\r\n", "%s:do snd_pcm_close audio", __func__, is_audio_sco_open);
    close_snd_pcm(ms_mic_pcm);
    close_snd_pcm(ms_spk_pcm);


#ifdef CONFIG_BRT_AEC_PROCESSING
    //bse_log_print(TAG, "\r\n", "%s: vcp8_destroy", __func__);
    //brt_internal_vcp_destroy();
    bse_log_print(TAG, "\r\n", "%s: vcp8_destroy", __func__);
    vcp8_destroy();
#endif
    if(echo_buf) {
        if(echo_buf->s16) {
            free(echo_buf->s16); 
        }
        free(echo_buf);
        echo_buf = NULL;
    }
    if(mic_buf) {
        if(mic_buf->s16) {
            free(mic_buf->s16); 
        }
        free(mic_buf);
        mic_buf = NULL;
    }
    if(filter_buf) {
        if(filter_buf->s16) {
            free(filter_buf->s16); 
        }
        free(filter_buf);
        filter_buf = NULL;
    }
    if(echo_reverse_buf) {
        if(echo_reverse_buf->s16) {
            free(echo_reverse_buf->s16); 
        }
        free(echo_reverse_buf);
        echo_reverse_buf = NULL;
    }


#ifdef OUTPUT_SCO_DATA
    if(sco_fp_nearend) {
        sco_file_close(sco_fp_nearend);
    }
    if(sco_fp_farend) {
        sco_file_close(sco_fp_farend);
    }
    if(sco_fp_aec) {
        sco_file_close(sco_fp_aec);
    }

#endif
    bse_log_print(TAG, "\r\n", "%s exit", __func__);
    s_device_status = 0;
    LEAVE_CRITICAL(s_sco_write_lock);
}

#define SCO_DATA_MAX_NUMBER 8
#define SCO_PACKET_SIZE 120

static int16_t debug_sco_buff[SCO_DATA_MAX_NUMBER * SCO_PACKET_SIZE + 1];
static int curr_sco_number = 0;

static void write_audio(const void *buffer, unsigned int size)
{
#ifdef OUTPUT_SCO_DATA
    //bse_log_print("sco write_audio------");
    //printf("sco write_audio------");
    static int debug_fg = 0;
    printf("1sco write_audio------ %d %d size = %d curr_sco_number=%d\n", debug_fg++, is_audio_sco_open, size, curr_sco_number);
    if ((sco_fp_farend != WAV_FILE_INVALID_HANDLE) && (sco_fp_farend != 0)) {
        bse_log_print("sco sco_file_write------ 1234");
        sco_file_write(sco_fp_farend, buffer, size);
    }
#endif
    bse_log_print(TAG, "\r\n", "is_audio_sco_open %d", is_audio_sco_open);
    if(is_audio_sco_open)
    {
        //bse_log_print("sco write_pcm------");
        //printf("sco write_pcm------");
#if 0
        if(curr_sco_number < SCO_DATA_MAX_NUMBER)
        {
            memcpy(&debug_sco_buff[curr_sco_number * SCO_PACKET_SIZE], buffer, size);
            curr_sco_number++;
        }
        if(curr_sco_number >= SCO_DATA_MAX_NUMBER)
        {
            //printf("packet_number = %d", curr_sco_number * SCO_PACKET_SIZE);
            write_pcm(ms_spk_pcm, debug_sco_buff, curr_sco_number * SCO_PACKET_SIZE);
            curr_sco_number = 0;
            memset(debug_sco_buff, 0, SCO_DATA_MAX_NUMBER * SCO_PACKET_SIZE + 1);
        }
#endif  

        write_pcm(ms_spk_pcm, buffer, size);

        //snd_pcm_writei(ms_spk_pcm, buffer, size);
    }
        
}

static void write_mic_audio(const void *buffer, unsigned int size)
{
    //TODO

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
//	.write_mic_audio = write_mic_audio,
	.audio_recoder_switch = audio_recoder_switch
};

static void suspend(struct snd_pcm_t *handle)
{
	int res;
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		if ((res = snd_pcm_prepare(handle)) < 0) {
			bse_log_print(TAG, "\r\n", "%s: prepare error", __func__);
		}
	}

}

static ssize_t write_pcm(sndpcm_t *sndpcm,  int16_t *buf, size_t count)
{
	ssize_t r;
	ssize_t result = 0;
    ssize_t test_count = count;
    uint8_t *data = buf;
    if(!is_audio_sco_open) {
        bse_log_print(TAG, "\r\n", "%s:audio is close is_audio_sco_open = %d", __func__, is_audio_sco_open);
        return 0; 
    }
    printf("count = %d vs sndpcm->chunk_size = %d\n", count, sndpcm->chunk_size);
	if (count < sndpcm->chunk_size) {
    //if (0) {
		snd_pcm_format_set_silence(sndpcm->format, data + count * sndpcm->bits_per_frame / 8, (sndpcm->chunk_size - count) * sndpcm->channels);
		count = sndpcm->chunk_size;
	}
	bse_log_print(TAG, "\r\n", "%s:count = %d, sndpcm->chunk_size:%d", __func__, count, sndpcm->chunk_size);
	while (count > 0 && is_audio_sco_open) {
        printf("count123 = %d\n", count);
        //usleep(50000);
        usleep(40000);
		r = snd_pcm_writei(sndpcm->handle, data, count);
        //printf("r = %d  80ms\n", r);
        usleep(40000);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            bse_log_print(TAG, "\r\n", "%s:EAGAIN\n", __func__);
		    //snd_pcm_wait(sndpcm->handle, 50000);
            printf("-11 %s:%d\n", __func__, r);
            //usleep(40000);
		} else if (r == -EPIPE) {
            printf("r == -EPIPE %s:%d\n", __func__, r);
            snd_pcm_prepare(sndpcm->handle);
            bse_log_print(TAG, "\r\n", "%s: write pcm buffer underrun,[%x]", __func__, sndpcm->handle);
		} else if (r == -ESTRPIPE) {
            printf("r == -ESTRPIPE %s:%d\n", __func__, r);
            bse_log_print(TAG, "\r\n", "%s: write pcm need suppend", __func__);
            suspend(sndpcm->handle);
		} else if (r < 0) {
            printf("r < 0\n");
			bse_log_print(TAG, "\r\n", "%s:write error: %s", __func__, snd_strerror(r));
            break;
		}
		if (r > 0) {
			result += r;
			count -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return test_count;
}
static ssize_t read_pcm(sndpcm_t *sndpcm, u_char *buf, size_t rcount)
{
	ssize_t r;
    size_t count = rcount;
	size_t result = 0;
    u_char *data = buf;
    
    if(!is_audio_sco_open) {
        bse_log_print(TAG, "\r\n", "%s:audio is close is_audio_sco_open = %d", __func__, is_audio_sco_open);
        return 0; 
    }

	if (count != sndpcm->chunk_size) {
		count = sndpcm->chunk_size;
	}

	while (data && count > 0 && is_audio_sco_open) {
		r = snd_pcm_readi(sndpcm->handle, data, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(sndpcm->handle, 10);
		} else if (r == -EPIPE) {
            snd_pcm_prepare(sndpcm->handle);
            bse_log_print(TAG, "\r\n", "%s: read pcm buffer underrun[%x]", __func__, sndpcm->handle);
		} else if (r == -ESTRPIPE) {
            bse_log_print(TAG, "\r\n", "%s: read pcm need suppend[%x]", __func__, sndpcm->handle);
            suspend(sndpcm->handle);
		} else if (r < 0) {
			bse_log_print(TAG, "\r\n", "%s:read error: %s[%x]", __func__, snd_strerror(r), sndpcm->handle);
            break;
		}
		if (r > 0) {
			result += r;
			count -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return rcount;
}
#if 0
static void *aec_run(void *param)
{
	int frame_size = 0;

	frame_size = ms_mic_pcm->chunk_bytes * 8 / ms_mic_pcm->bits_per_frame;
	//frame_size = sco_down_pcm->chunk_bytes * 8 / sco_down_pcm->bits_per_frame;

	bse_log_print(TAG, "\r\n", "%s:frame_size = %d, ms_mic_pcm->chunk_bytes = %d, ms_mic_pcm->bits_per_frame = %d", __func__, frame_size, ms_mic_pcm->chunk_bytes, ms_mic_pcm->bits_per_frame);
	while (is_audio_sco_open) {
		if(read_pcm(sco_down_pcm, echo_buf->s16, frame_size) !=  frame_size) {//read sco down
			bse_log_print(TAG, "\r\n", "%s:read_pcm sco_down_pcm error", __func__);
			break;
		}
#ifdef CONFIG_BRT_AEC_PROCESSING
		echo_buf->frameCount = frame_size;
		bse_log_print(TAG, "\r\n", "%s:set echo size:%d", __func__,  echo_buf->frameCount);
		brt_internal_vcp_set_echo_reference(echo_buf);
#endif
		if(read_pcm(ms_mic_pcm, mic_buf->s16, frame_size) != frame_size) {//read local mic
			bse_log_print(TAG, "\r\n", "%s:read_pcm ms_mic_pcm error", __func__); 
			break;
		} else {
			bse_log_print(TAG, "\r\n", "%s: read %d frames from MIC.", __func__,  frame_size);
		}
#ifdef OUTPUT_SCO_DATA
        if ((sco_fp_farend != WAV_FILE_INVALID_HANDLE) && (sco_fp_farend != 0)) {
            sco_file_write(sco_fp_farend, mic_buf->s16, frame_size * 2);
        }   
        if ((sco_fp_nearend != WAV_FILE_INVALID_HANDLE) && (sco_fp_nearend != 0)) {
            sco_file_write(sco_fp_nearend, echo_buf->s16, frame_size * 2);
        }   
#endif
#ifdef CONFIG_BRT_AEC_PROCESSING
		mic_buf->frameCount = frame_size;
		filter_buf->frameCount = frame_size;
		brt_internal_vcp_process(mic_buf, filter_buf);
#endif

		if(write_pcm(ms_spk_pcm, echo_buf->s16, frame_size) != frame_size) {//write local spk
			bse_log_print(TAG, "\r\n", "%s:write_pcm ms_spk_pcm error", __func__);
			break;
		}

#ifdef CONFIG_BRT_AEC_PROCESSING
//		if(AUDIO_SCO_UNMUTE == s_is_audio_sco_mute){
            bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm AUDIO_SCO_UNMUTE20210114", __func__);
			if(write_pcm(sco_up_pcm,filter_buf->s16/*mic_buf->s16*/, frame_size) != frame_size) {//write sco up
				bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm error", __func__);
				break;
			}
//		}else{
//			memset(filter_buf->s16, 0, frame_size*2);
//			if(write_pcm(sco_up_pcm,filter_buf->s16/*mic_buf->s16*/, frame_size) != frame_size) {//write sco up
//				bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm error", __func__);
//				break;
//			}
//		}
#ifdef OUTPUT_SCO_DATA
		if ((sco_fp_aec != WAV_FILE_INVALID_HANDLE) && (sco_fp_aec != 0)) {
			sco_file_write(sco_fp_aec, filter_buf->s16, frame_size * 2);
		}    
#endif
#else
		if(AUDIO_SCO_UNMUTE == s_is_audio_sco_mute){
			bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm AUDIO_SCO_UNMUTE", __func__);
			if(write_pcm(sco_up_pcm,mic_buf->s16, frame_size) != frame_size) {//write sco up
				bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm error", __func__);
				break;
			}
		}else{
			bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm ", __func__);
			memset(filter_buf->s16, 0, frame_size*2);
			if(write_pcm(sco_up_pcm,mic_buf->s16, frame_size) != frame_size) {//write sco up
				bse_log_print(TAG, "\r\n", "%s:write_pcm sco_up_pcm error", __func__);
				break;
			}
		}
#ifdef OUTPUT_SCO_DATA
		if ((sco_fp_aec != WAV_FILE_INVALID_HANDLE) && (sco_fp_aec != 0)) {
			sco_file_write(sco_fp_aec, mic_buf->s16, frame_size * 2);
		}    
#endif
#endif    //end CONFIG_BRT_AEC_PROCESSING
    }
    bse_log_print(TAG, "\r\n", "%s: thread aec_run exit", __func__);
    return 0;
}
#endif

static void close_snd_pcm(sndpcm_t *sndpcm)
{
    if (sndpcm->handle != NULL) {
        bse_log_print(TAG, "\r\n", "%s:sndpcm->handle:%p", __func__, sndpcm->handle);
        snd_pcm_nonblock(sndpcm->handle, 0);
//        snd_pcm_drain(sndpcm->handle); //close play pcm need 10s
        snd_pcm_close(sndpcm->handle);
        sndpcm->handle = NULL;
    }
    if(sndpcm != NULL) {
        sndpcm = NULL;
    }
    bse_log_print(TAG, "\r\n", "%s:exit", __func__);
}
