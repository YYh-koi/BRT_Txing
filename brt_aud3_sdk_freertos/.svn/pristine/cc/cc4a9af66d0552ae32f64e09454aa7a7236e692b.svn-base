#include "porting.h"
#include "string.h"
#include "autoconf_bt.h"
#include "audio_sink_device.h"
#ifdef CONFIG_OS_ANDROID
#include <hardware/hardware.h>
#endif
#include <pthread.h>
#include <semaphore.h>
#include "bt_audio_hw.h"
#include "audio_sink_hw.h"
#include "bse_timer.h"
#include "bse_audio_queue.h"
#include "brt_trace.h"
#define TAG "audio_sink_device"

#define AUDIO_SINK_TICKS 2

#ifdef CONFIG_AUDIO_DEBUG
#define BTHW_VDBG(fmt,...) bse_log_info(TAG, fmt, ## __VA_ARGS__)
#else
#define BTHW_VDBG(fmt,...) ((void *)0)
#endif

/* The audio data is been written into AudioTrack in calling thread or the daemon audio-thread. */

#define debug_lock(A) do { \
    BTHW_VDBG("fifo_lock   called by %s -- [%d]", __func__, __LINE__); fifo_mutex_lock(A); \
} while (0);

#define debug_unlock(A) do { \
    BTHW_VDBG("fifo_unlock called by %s -- [%d]", __func__, __LINE__); fifo_mutex_unlock(A); \
} while (0);

enum {
    TASK_NULL = 0,
    TASK_OPEN_AUDIO,
    TASK_CLOSE_AUDIO,
    TASK_START_AUDIO,
    TASK_STOP_AUDIO,
    TASK_WRITE_AUDIO,
	TASK_READ_AUDIO,
};

typedef struct fifo_lock_str {
	pthread_mutex_t lock;
	uint32_t next_ticket;
	uint32_t cur_ticket;
	pthread_cond_t cond;
} fifo_mutex_t;

typedef struct audio_sink_device_tag {
	audio_sink_hw_device_t * hw_device;
	audio_a2dp_callbacks_t bt_sink_cbs;
	fifo_mutex_t lock;
	sem_t wait_obj;
	int running;
	int current_task;
	int sample_rate;
	int audio_started;
	int is_audio_on;
	int type;
#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	bse_timer_t *timer;
	bse_audio_queue_t *queue;
	unsigned char *write_buffer;
#endif
} audio_sink_device_cb_t;

static void fifo_mutex_init(fifo_mutex_t *fifo_lock);
static void fifo_mutex_destroy(fifo_mutex_t *fifo_lock);
static void fifo_mutex_lock(fifo_mutex_t *fifo_lock);
static void fifo_mutex_unlock(fifo_mutex_t *fifo_lock);

static void *audio_sink_process(void *param);
static void audio_sink_switch(int val);
static int open_audio_sink(int sample);
static void close_audio_sink();
static void start_audio_sink();
static void write_audio_sink(const void *buffer, unsigned int size);
static void read_audio_sink(const void *buffer, unsigned int size);
static void stop_audio_sink();
static int is_audio_sink_stopped();

static void queue_event_cbk(uint8_t event, int param) {
	//bse_log_info(TAG, "queue_event_cbk(%d, %d)", event, param);
	switch (event) {
		case BSE_AUDIO_QUEUE_EVENT_OVERRUN:
			break;
		case BSE_AUDIO_QUEUE_EVENT_UNDERRUN:
			break;
		case BSE_AUDIO_QUEUE_EVENT_THRESHOLD:
			break;
	}
}

static audio_sink_device_cb_t s_dev_cb;

#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
static void write_audio_proc(void *param);
#endif

void audio_sink_set_running()
{
	s_dev_cb.running = 0;//discable to 0
}

void audio_sink_device_open()
{
	int err;
	hw_module_t *module;
	const char *id = AUDIO_SINK_HARDWARE_MODULE_ID;

	memset(&s_dev_cb, 0, sizeof(audio_sink_device_cb_t));
	memset(&s_dev_cb.bt_sink_cbs, 0, sizeof(audio_a2dp_callbacks_t));
	s_dev_cb.sample_rate = 44100;
	s_dev_cb.running = 1;
	s_dev_cb.is_audio_on = 1;

	fifo_mutex_init(&s_dev_cb.lock);
	sem_init(&s_dev_cb.wait_obj, 0, 0);
#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	s_dev_cb.timer = bse_timer_create("audio_sink_hw", AUDIO_SINK_TICKS, write_audio_proc, &s_dev_cb);
#endif
	hw_device_t *audio_dev = NULL;
#ifdef AUDIO_SINK_DEVICE_BY_HAL
	err = hw_get_module(id, (hw_module_t const **) &module);

	if (err == 0) {
		err = module->methods->open(module, id, &audio_dev);

		if (err != 0) {
			BTHW_VDBG("%s", "Error while opening Bluetooth Audio Sink Library");
		}
	} else {
		BTHW_VDBG("%s", "No Audio Sink Library Found.");
	}
#else
	audio_dev = audio_sink_dev_open();
#endif
	
	if (audio_dev) {
		s_dev_cb.hw_device = (audio_sink_hw_device_t *) audio_dev;
	}

	s_dev_cb.bt_sink_cbs.audio_track_switch = audio_sink_switch;
	s_dev_cb.bt_sink_cbs.open_audio = open_audio_sink;
	s_dev_cb.bt_sink_cbs.close_audio = close_audio_sink;
	s_dev_cb.bt_sink_cbs.start_audio = start_audio_sink;
	s_dev_cb.bt_sink_cbs.write_audio = write_audio_sink;
	s_dev_cb.bt_sink_cbs.read_audio = read_audio_sink;
	s_dev_cb.bt_sink_cbs.stop_audio = stop_audio_sink;
	s_dev_cb.bt_sink_cbs.is_audio_stopped = is_audio_sink_stopped;
	app_register_audio_sink_callback(&s_dev_cb.bt_sink_cbs);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, audio_sink_process, NULL);
	pthread_detach(thread_id);
}

void audio_sink_device_close()
{
	s_dev_cb.running = 0;
#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	if(s_dev_cb.timer) {
		bse_timer_destroy(s_dev_cb.timer);
		s_dev_cb.timer = NULL;
	}
	if(s_dev_cb.queue) {
		bse_audio_queue_destroy(s_dev_cb.queue);
		s_dev_cb.queue = NULL;
	}
	if (s_dev_cb.write_buffer) {
		free(s_dev_cb.write_buffer);
		s_dev_cb.write_buffer = NULL;
	}
#endif
	sem_post(&s_dev_cb.wait_obj);
	sem_destroy(&s_dev_cb.wait_obj);
	fifo_mutex_destroy(&s_dev_cb.lock);

	if (s_dev_cb.hw_device) {
		s_dev_cb.hw_device->common.close((struct hw_device_t *) s_dev_cb.hw_device);
		s_dev_cb.hw_device = NULL;
	}
}

static void do_stop_audio_sink()
{
	s_dev_cb.audio_started = 0;
	s_dev_cb.hw_device->stop_audio_sink();
#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	bse_timer_cancel(s_dev_cb.timer);
	bse_audio_queue_reset(s_dev_cb.queue);
#endif
}

static void *audio_sink_process(void *param)
{
	while (s_dev_cb.running) {
                bse_log_info("audio_sink_process","current_task:%d\r\n",s_dev_cb.current_task);
		fifo_mutex_lock(&s_dev_cb.lock);
		switch (s_dev_cb.current_task) {
		case TASK_OPEN_AUDIO:
            bse_log_info("audio_sink_process","s_dev_cb.sample_rate:%d\r\n", s_dev_cb.sample_rate);
			s_dev_cb.hw_device->open_audio_sink(s_dev_cb.sample_rate);
			break;
		case TASK_CLOSE_AUDIO:
			/*if (s_dev_cb.audio_started) {
				do_stop_audio_sink();
			}*/
			bse_log_info("audio_sink_process","s_dev_cb.sample_rate:%d\r\n", s_dev_cb.type);
			s_dev_cb.hw_device->close_audio_sink(s_dev_cb.type);
			break;
		case TASK_START_AUDIO:
			s_dev_cb.hw_device->start_audio_sink();
			BTHW_VDBG("%s audio started", __FUNCTION__);
			s_dev_cb.audio_started = 1;
#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
			bse_timer_schedule(s_dev_cb.timer);
#endif
			break;
		case TASK_STOP_AUDIO:
			do_stop_audio_sink();
			break;
		case TASK_WRITE_AUDIO:
			break;
		case TASK_READ_AUDIO:
			//s_dev_cb.hw_device->read_audio_sink();
			break;
		}
		fifo_mutex_unlock(&s_dev_cb.lock);
                bse_log_info("audio_sink_process_lock","current_task:%d Exit\r\n",s_dev_cb.current_task);
		sem_wait(&s_dev_cb.wait_obj);
	}

	return NULL;
}

static void audio_sink_switch(int val)
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.is_audio_on = val;
	fifo_mutex_unlock(&s_dev_cb.lock);
	//sem_post(&s_dev_cb.wait_obj);
}

static int open_audio_sink(int sample)
{
    bse_log_info("open_audio_sink","sample=%d\r\n",sample);
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.sample_rate = sample;
    bse_log_info("open_audio_sink","s_dev_cb.sample_rate=%d\r\n",sample);
	s_dev_cb.current_task = TASK_OPEN_AUDIO;

#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	printf("audio_sink: create_audio_queue rate is %d\r\n", sample);
	int queue_buffer_size = sample * 4;
	int min_buffered_size = GKI_TICKS_TO_MS(AUDIO_SINK_TICKS) * sample * 2 * 2 / 1000;//16 bit,2chnl.
	s_dev_cb.queue = bse_audio_queue_create(queue_buffer_size, min_buffered_size, queue_event_cbk);
	s_dev_cb.write_buffer = (unsigned char *) malloc(sample);
#endif
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);

	return 0;
}

static void close_audio_sink(int type_s)
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.type = type_s;
    bse_log_info("close_audio_sink","s_dev_cb.type=%d\r\n",s_dev_cb.type);
	s_dev_cb.current_task = TASK_CLOSE_AUDIO;

#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
	if(s_dev_cb.queue) {
		bse_audio_queue_destroy(s_dev_cb.queue);
		s_dev_cb.queue = NULL;
	}
	if (s_dev_cb.write_buffer) {
		free(s_dev_cb.write_buffer);
		s_dev_cb.write_buffer = NULL;
	}
#endif
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
}

static void start_audio_sink()
{
        bse_log_info(TAG, "%s +.", __FUNCTION__);
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.current_task = TASK_START_AUDIO;
	//s_dev_cb.audio_started = 1;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
        bse_log_info(TAG, "%s -.", __FUNCTION__);
}

static void read_audio_sink(const void *buffer, unsigned int size)
{
	//printf("[wl_debug]read_audio_sink2\n");
//	fifo_mutex_lock(&s_dev_cb.lock);
	if(s_dev_cb.is_audio_on == 0) {
//		fifo_mutex_unlock(&s_dev_cb.lock);
		return ;
	}
	s_dev_cb.hw_device->read_audio_sink(buffer, size);

//	fifo_mutex_unlock(&s_dev_cb.lock);
}

static void write_audio_sink(const void *buffer, unsigned int size)
{
//	fifo_mutex_lock(&s_dev_cb.lock);

	if(s_dev_cb.is_audio_on == 0) {
		UINT8 *buffer_null = (UINT8 *)malloc(size);
		if (buffer_null == NULL) {
			bse_log_info(TAG, "buffer_null malloc fail");
//			fifo_mutex_unlock(&s_dev_cb.lock);
			return ;
		}
		memset(buffer_null, 0, size);
#ifndef WRITE_AUDIO_IN_PERIODIC_PTHREAD
		s_dev_cb.hw_device->write_audio_sink(buffer_null, size);
#else
		bse_audio_queue_write(s_dev_cb.queue, buffer_null, 0, size);
#endif
		free(buffer_null);
	} else {
#ifndef WRITE_AUDIO_IN_PERIODIC_PTHREAD
		s_dev_cb.hw_device->write_audio_sink(buffer, size);
#else
		bse_audio_queue_write(s_dev_cb.queue, buffer, 0, size);
#endif
	}

//	fifo_mutex_unlock(&s_dev_cb.lock);
}

static void stop_audio_sink()
{
        bse_log_info(TAG, "%s +.", __FUNCTION__);
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.audio_started = 0;
	s_dev_cb.current_task = TASK_STOP_AUDIO;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
    bse_log_info(TAG, "%s -.", __FUNCTION__);
}

static int is_audio_sink_stopped()
{
	// TODO
	return 1;
}

#ifdef WRITE_AUDIO_IN_PERIODIC_PTHREAD
static void write_audio_proc(void *param)
{
	audio_sink_device_cb_t *pcb = (audio_sink_device_cb_t *) param;
	size_t size = GKI_TICKS_TO_MS(AUDIO_SINK_TICKS) * s_dev_cb.sample_rate * 4 / 1000;
	if (pcb == NULL || pcb->write_buffer == NULL) return;

	size = bse_audio_queue_read(pcb->queue, pcb->write_buffer, 0, size);
	BTHW_VDBG("bse_audio_queue_read return %d bytes: %lld", size, get_current_time());

	if (size > 0) {
		pcb->hw_device->write_audio_sink(pcb->write_buffer, size);
	}
	BTHW_VDBG("after write_audio_sink:%lld.", get_current_time());
}

#endif

static void fifo_mutex_init(fifo_mutex_t *fifo_lock)
{
	memset(fifo_lock, 0, sizeof(fifo_mutex_t));
	pthread_mutex_init(&fifo_lock->lock, NULL);
	fifo_lock->cur_ticket = 1;
	fifo_lock->next_ticket = 0;
}

static void fifo_mutex_destroy(fifo_mutex_t *fifo_lock)
{
	pthread_mutex_destroy(&fifo_lock->lock);
}

static void fifo_mutex_lock(fifo_mutex_t *fifo_lock)
{
	unsigned long my_ticket = __sync_add_and_fetch(&fifo_lock->next_ticket, 1);
	pthread_mutex_lock(&fifo_lock->lock);
	while (fifo_lock->cur_ticket != my_ticket) {
		pthread_cond_wait(&fifo_lock->cond, &fifo_lock->lock);
	}
}

static void fifo_mutex_unlock(fifo_mutex_t *fifo_lock)
{
	fifo_lock->cur_ticket++;
	pthread_cond_broadcast(&fifo_lock->cond);
	pthread_mutex_unlock(&fifo_lock->lock);
}

