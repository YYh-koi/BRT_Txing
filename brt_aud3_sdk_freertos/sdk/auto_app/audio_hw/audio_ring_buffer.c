#include "btdatatype.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "audio_ring_buffer.h"
#include "brt_trace.h"

//FIXME
//#define AUDIO_RECORD_READ_AGGRESSIVE
#define NAME_MAX_LEN 64
//#define CONFIG_RING_BUFFER_DEBUG
#define LOG_TAG "ring_buffer"

#ifdef CONFIG_RING_BUFFER_DEBUG
#ifdef CONFIG_OS_ANDROID
#include <cutils/log.h>
#undef LOG_TAG
#define LOG_TAG "ring_buffer"
#define DBG(...) do { \
        ALOGD(__VA_ARGS__); \
    } while (0);

#else
//#define DBG(...) do { \
//        printf(__VA_ARGS__); \
//        printf("\n"); \
//    } while (0);

#define DBG(fmt, ...) bse_log_info(LOG_TAG, fmt, ## __VA_ARGS__)
#endif
#else
#define DBG(...) ((void *)0) 
#endif

#define DBG_WARN(fmt, ...) bse_log_info(LOG_TAG, fmt, ## __VA_ARGS__)

#define QUEUE_BUFFER_SIZE (4 * 8000 * 2 * 2)
#define AUDIO_DATA_UNIT_SIZE 2048

int s_pull_min_len = 0;


typedef struct tag_ring_buffer {
	ring_buffer_t common;
	pthread_mutex_t mutex;
	pthread_cond_t  cond;

	size_t buf_size;
	unsigned char *audio_data_queue;
	size_t read_idx;
	size_t write_idx;
	size_t delta;

	/* start/stop the ring buffer which means to
	   leave from block when pull data from or push data from the ring-buffer. */
	unsigned char avail;
	unsigned char need_block;
    char name[NAME_MAX_LEN];
	size_t min_size_signal;
} ring_buffer_impl_t;

static int audio_ring_buffer_push(ring_buffer_t *rb, unsigned char *buffer, int length);
static int audio_ring_buffer_pull(ring_buffer_t *rb, unsigned char *buffer, int length);
static int audio_ring_buffer_avail(ring_buffer_t *rb);
static void audio_ring_buffer_start(ring_buffer_t *rbb);
static void audio_ring_buffer_stop(ring_buffer_t *rbb);

ring_buffer_t *audio_ring_buffer_create(int buf_size, int need_block, const char *name)
{
	ring_buffer_impl_t *obj = (ring_buffer_impl_t *) malloc(sizeof(ring_buffer_impl_t));

	if (!obj) {
		return NULL;
	}

	obj->common.push = audio_ring_buffer_push;
	obj->common.pull = audio_ring_buffer_pull;
	obj->common.avail = audio_ring_buffer_avail;
	obj->common.start = audio_ring_buffer_start;
	obj->common.stop = audio_ring_buffer_stop;
	obj->need_block = need_block;
	//obj->buf_size = buf_size > 0 ? buf_size : QUEUE_BUFFER_SIZE;
	obj->buf_size = QUEUE_BUFFER_SIZE;
	obj->audio_data_queue = (unsigned char *) malloc(obj->buf_size);

	if (!obj->audio_data_queue) {
		free(obj);
		return NULL;
	}

	memset(obj->audio_data_queue, 0, sizeof(obj->buf_size));
	obj->read_idx = 0;
	obj->write_idx = 0;
	obj->delta = 0;
	obj->avail = 1;
	obj->min_size_signal = 0;
	pthread_mutex_init(&obj->mutex, NULL);
	pthread_cond_init(&obj->cond, NULL);
	strncpy(obj->name, name, 64);
	return (ring_buffer_t *) obj;
}

void audio_ring_buffer_free(ring_buffer_t *obj)
{
	ring_buffer_impl_t *o = (ring_buffer_impl_t *) obj;
	DBG_WARN("[wl_debug 3] audio_ring_buffer_free o = %p", o);
	if (o) {
		// FIXME need to singal?
		pthread_mutex_lock(&o->mutex);
		o->avail = 0;
		pthread_cond_signal(&o->cond);
		pthread_mutex_unlock(&o->mutex);
		DBG_WARN("[wl_debug 2] audio_ring_buffer_free o = %p", o);
		free(o->audio_data_queue);
		o->audio_data_queue = NULL;
		DBG_WARN("[wl_debug 1] audio_ring_buffer_free o = %p", o);
		pthread_mutex_destroy(&o->mutex);
		pthread_cond_destroy(&o->cond);

		free(o);
		o = NULL;
		//DBG("[wl_debug 0] audio_ring_buffer_free o = %p", o);
	}
}

int aduio_ring_buffer_count(ring_buffer_t *obj)
{
	ring_buffer_impl_t *r = (ring_buffer_impl_t *) obj;
	return r->delta;
}

static int audio_ring_buffer_push(ring_buffer_t *rbb, unsigned char *buffer, int length)
{
	int  cp_len, avail;
	unsigned char *dest, *src;
	ring_buffer_impl_t *rb = (ring_buffer_impl_t *) rbb;
	pthread_mutex_lock(&rb->mutex);

	// check whether overun
	while (rb->avail && (rb->delta + length > rb->buf_size)) {
		DBG_WARN("%s[%s] Overrun! push %d byte, remain %d bytes not be consumed. buffer size:%d.\n", __func__, ((ring_buffer_impl_t *)rb)->name, length, rb->delta, rb->buf_size);

		if (rb->need_block) {
			pthread_cond_wait(&rb->cond, &rb->mutex);
		} else {
			pthread_mutex_unlock(&rb->mutex);
			// FIXME just copy small data?
			return 0;
		}
	}

	if (!rb->avail) {
		pthread_mutex_unlock(&rb->mutex);
		return 0;
	}

	rb->delta += length;

	if (rb->audio_data_queue) {
		dest = rb->audio_data_queue + rb->write_idx;
		src = buffer;

		if (rb->write_idx + length >= rb->buf_size) {
			cp_len = rb->buf_size - rb->write_idx;
			memcpy(dest, src, cp_len);
			dest = rb->audio_data_queue;
			src += cp_len;
			cp_len = length - cp_len;
		} else {
			cp_len = length;
		}

		if (cp_len > 0) {
			memcpy(dest, src, cp_len);
		}

		rb->write_idx += length;
		rb->write_idx %= rb->buf_size;
//zjgtest
        if(rb->delta >= rb->min_size_signal) {
		    pthread_cond_signal(&rb->cond);
        }
	}
	pthread_mutex_unlock(&rb->mutex);

    //DBG("%s[%s] push:%d bytes, rb->delta:%d bytes", __func__,((ring_buffer_impl_t *)rb)->name, length, rb->delta);
	return length;
}

static int audio_ring_buffer_avail(ring_buffer_t *rbb)
{
	int avail = 0;
	ring_buffer_impl_t *rb = (ring_buffer_impl_t *) rbb;

	pthread_mutex_lock(&rb->mutex);
	if (rb->avail) {
		avail = rb->delta;
	}
	pthread_mutex_unlock(&rb->mutex);

	return avail;
}

static void audio_ring_buffer_start(ring_buffer_t *rbb)
{
    ring_buffer_impl_t *rb = (ring_buffer_impl_t *) rbb;
    pthread_mutex_lock(&rb->mutex);
	pthread_cond_signal(&rb->cond);
    rb->read_idx = 0;
    rb->write_idx = 0;
    rb->delta = 0;
    rb->avail = 1;
    pthread_mutex_unlock(&rb->mutex);
}

static void audio_ring_buffer_stop(ring_buffer_t *rbb)
{
    ring_buffer_impl_t *rb = (ring_buffer_impl_t *) rbb;
    pthread_mutex_lock(&rb->mutex);
    rb->avail = 0;
    pthread_cond_signal(&rb->cond);
    pthread_mutex_unlock(&rb->mutex);
}

static int audio_ring_buffer_pull(ring_buffer_t *rbb, unsigned char *buffer, int length)
{
	int read_len, cp_len, avail;
	unsigned char *dest, *src;
	ring_buffer_impl_t *rb = (ring_buffer_impl_t *) rbb;
    rb->min_size_signal = length;
	pthread_mutex_lock(&rb->mutex);

//    DBG("%s[%s] to pull:%d bytes, rb->delta:%d\n", __func__, ((ring_buffer_impl_t *)rb)->name, length, rb->delta);
	// if no data available, block
#ifndef AUDIO_RECORD_READ_AGGRESSIVE
	while (rb->avail && (rb->delta < (size_t) length))
#else
	while (rb->avail && (rb->delta <= (size_t) 0))
#endif
	{
        DBG_WARN("%s[%s] Underrun!! request %d bytes, but only %d bytes available.\n", __func__, ((ring_buffer_impl_t *)rb)->name, length, rb->delta);
		//bse_log_info(LOG_TAG, "%s[%s] Underrun!! request %d bytes, but only %d bytes available.\n", __func__, ((ring_buffer_impl_t *)rb)->name, length, rb->delta);

		if (rb->need_block) {
		    //DBG("%s[%s] Pull : Start wait", __func__, ((ring_buffer_impl_t *)rb)->name);
			pthread_cond_wait(&rb->cond, &rb->mutex);
		    //DBG("%s[%s] Pull : wait End", __func__, ((ring_buffer_impl_t *)rb)->name);
		} else {
			// FIXME just fetch small data?
			pthread_mutex_unlock(&rb->mutex);
			return 0;
		}
	}
	//DBG("[wl_debug 1] rb = %p rb->avail = %d", rb, rb->avail);
	if(rb == NULL)
	{
		return 0;
	}

	if (!rb->avail) {
		// FIXME pthread_cond_signal(&rb->cond);
		pthread_mutex_unlock(&rb->mutex);
		return 0;
	}

	if (length >= (int) rb->delta) {
		read_len = (int) rb->delta;
	} else {
		read_len = length;
//		while ((read_len + length <= (int) rb->delta) && read_len + length <= 4 * length) {
//			read_len += length;
//		}
	}
	//DBG("[wl_debug 2] rb = %p rb->avail = %d", rb, rb->avail);

	dest = buffer;
	src = rb->audio_data_queue + rb->read_idx;

	if (read_len + rb->read_idx >= rb->buf_size) {
		cp_len = rb->buf_size - rb->read_idx;
		memcpy(dest, src, cp_len);
		dest += cp_len;
		src = rb->audio_data_queue;
		cp_len = read_len - cp_len;
	} else {
		cp_len = read_len;
	}

	if (cp_len > 0) {
		memcpy(dest, src, cp_len);
	}
	//DBG("[wl_debug 3] rb = %p rb->avail = %d", rb, rb->avail);
	rb->delta -= read_len;
	rb->read_idx += read_len;
	rb->read_idx %= rb->buf_size;
	//DBG("[wl_debug 4] rb = %p rb->avail = %d", rb, rb->avail);
	pthread_cond_signal(&rb->cond);
	pthread_mutex_unlock(&rb->mutex);


    //DBG("%s[%s] pull:%d bytes, rb->delta:%d bytes", __func__,((ring_buffer_impl_t *)rb)->name, length, rb->delta);
	//bse_log_info(LOG_TAG, "%s[%s] pull:%d bytes, rb->delta:%d bytes", __func__,((ring_buffer_impl_t *)rb)->name, length, rb->delta);
	return read_len;
}

