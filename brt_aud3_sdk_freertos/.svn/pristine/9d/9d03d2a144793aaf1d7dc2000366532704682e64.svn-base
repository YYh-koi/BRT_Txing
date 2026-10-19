#include "porting.h"
#include "audio_sink_device.h"
#include <pthread.h>
#include <semaphore.h>
#include "bt_audio_hw.h"
#include "audio_sink_hw.h"
#include "brt_trace.h"

#define TAG "audio_sink_linux_device"

#define MAX_BUFFER_SIZE (512)

#ifdef CONFIG_AUDIO_DEBUG
#define BTHW_VDBG(fmt,...) bse_log_info(TAG, fmt, ## __VA_ARGS__)
#else
#define BTHW_VDBG(fmt,...) ((void *)0)
#endif

/* The audio data is been written into AudioTrack in calling thread or the daemon audio-thread. */
#define WRITE_AUDIO_IN_CALLING_THREAD

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
	int type;
	unsigned char *buffer;
	int size;
	int buffer_size;
	int is_audio_on;
} audio_sink_device_cb_t;

static void fifo_mutex_init(fifo_mutex_t *fifo_lock);
static void fifo_mutex_destroy(fifo_mutex_t *fifo_lock);
static void fifo_mutex_lock(fifo_mutex_t *fifo_lock);
static void fifo_mutex_unlock(fifo_mutex_t *fifo_lock);

static void *audio_sink_process(void *param);
static void audio_sink_switch(int val);
static int open_audio_sink(int sample);
static void close_audio_sink(int type);
static void start_audio_sink();
static void write_audio_sink(const void *buffer, unsigned int size);
static void read_audio_sink(const void *buffer, unsigned int size);
static void stop_audio_sink();
static int is_audio_sink_stopped();

static audio_sink_device_cb_t s_dev_cb;

void audio_sink_device_open()
{
	int err;

	memset(&s_dev_cb, 0, sizeof(audio_sink_device_cb_t));
	memset(&s_dev_cb.bt_sink_cbs, 0, sizeof(audio_a2dp_callbacks_t));
	s_dev_cb.running = 1;
	s_dev_cb.buffer_size = MAX_BUFFER_SIZE;
	s_dev_cb.is_audio_on = 1;
	s_dev_cb.buffer = (unsigned char *) malloc(s_dev_cb.buffer_size);

	fifo_mutex_init(&s_dev_cb.lock);
	sem_init(&s_dev_cb.wait_obj, 0, 0);

	audio_sink_hw_device_t *audio_dev = NULL;
	audio_dev = audio_sink_dev_open();

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
	//app_start_pthread();

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, audio_sink_process, NULL);
	pthread_detach(thread_id);
}

void audio_sink_device_close()
{
	s_dev_cb.running = 0;

	if (s_dev_cb.buffer) {
		free(s_dev_cb.buffer);
		s_dev_cb.buffer = NULL;
	}

	sem_post(&s_dev_cb.wait_obj);
	sem_destroy(&s_dev_cb.wait_obj);
	fifo_mutex_destroy(&s_dev_cb.lock);

	if (s_dev_cb.hw_device) {
		s_dev_cb.hw_device->common.close((struct hw_device_t *) s_dev_cb.hw_device);
		s_dev_cb.hw_device = NULL;
	}
}

static void *audio_sink_process(void *param)
{
	while (s_dev_cb.running) {
		fifo_mutex_lock(&s_dev_cb.lock);
		switch (s_dev_cb.current_task) {
		case TASK_OPEN_AUDIO:
			s_dev_cb.hw_device->open_audio_sink(s_dev_cb.sample_rate);
			break;
		case TASK_CLOSE_AUDIO:
			s_dev_cb.hw_device->close_audio_sink(s_dev_cb.type);
			break;
		case TASK_START_AUDIO:
			s_dev_cb.hw_device->start_audio_sink();
			break;
		case TASK_STOP_AUDIO:
			s_dev_cb.hw_device->stop_audio_sink();
			break;
		case TASK_WRITE_AUDIO:
			s_dev_cb.hw_device->write_audio_sink(s_dev_cb.buffer, s_dev_cb.size);
			break;
		}
		fifo_mutex_unlock(&s_dev_cb.lock);

		sem_wait(&s_dev_cb.wait_obj);
	}

	return NULL;
}

static void audio_sink_switch(int val)
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.is_audio_on = val;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
}

static int open_audio_sink(int sample)
{
	printf("open_audio_sink\n");
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.sample_rate = sample;
	s_dev_cb.current_task = TASK_OPEN_AUDIO;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);

	return 0;
}

static void close_audio_sink(int type)
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.type = type;
	s_dev_cb.current_task = TASK_CLOSE_AUDIO;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
}

static void start_audio_sink()
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.current_task = TASK_START_AUDIO;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
}

static void read_audio_sink(const void *buffer, unsigned int size)
{
	printf("[wl_debug]read_audio_sink\n");
	fifo_mutex_lock(&s_dev_cb.lock);
	if(s_dev_cb.is_audio_on == 0) {
		fifo_mutex_unlock(&s_dev_cb.lock);
		return ;
	}
	s_dev_cb.hw_device->read_audio_sink(buffer, size);

	fifo_mutex_unlock(&s_dev_cb.lock);
}

static void write_audio_sink(const void *buffer, unsigned int size)
{
	fifo_mutex_lock(&s_dev_cb.lock);

	if(s_dev_cb.is_audio_on == 0) {
		fifo_mutex_unlock(&s_dev_cb.lock);
#ifdef WRITE_AUDIO_IN_CALLING_THREAD

#else
		sem_post(&s_dev_cb.wait_obj);
#endif
		return ;
	}
#ifdef WRITE_AUDIO_IN_CALLING_THREAD
	s_dev_cb.hw_device->write_audio_sink(buffer, size);
#else
	if (size > s_dev_cb.buffer_size) {
		s_dev_cb.buffer_size = size;
		s_dev_cb.buffer = realloc(s_dev_cb.buffer, size);
	}

	memcpy(s_dev_cb.buffer, buffer, size);
	s_dev_cb.size = size;
	s_dev_cb.current_task = TASK_WRITE_AUDIO;
#endif
	fifo_mutex_unlock(&s_dev_cb.lock);
#ifdef WRITE_AUDIO_IN_CALLING_THREAD
#else
	sem_post(&s_dev_cb.wait_obj);
#endif
}

static void stop_audio_sink()
{
	fifo_mutex_lock(&s_dev_cb.lock);
	s_dev_cb.current_task = TASK_STOP_AUDIO;
	fifo_mutex_unlock(&s_dev_cb.lock);
	sem_post(&s_dev_cb.wait_obj);
}

static int is_audio_sink_stopped()
{
	// TODO
	return 1;
}

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
