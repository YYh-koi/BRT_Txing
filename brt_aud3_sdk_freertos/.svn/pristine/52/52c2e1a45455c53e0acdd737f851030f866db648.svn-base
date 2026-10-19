#include "btdatatype.h"
#include <pthread.h>
#include "bse_audio_queue.h"

#define BTHW_VDBG(fmt,...) bse_log_print("bse_audio_queue", "\r\n", fmt, ## __VA_ARGS__)
#ifdef CONFIG_AUDIO_DEBUG
#define BTHW_VVDBG(fmt,...) bse_log_print("bse_audio_queue", "\r\n", fmt, ## __VA_ARGS__)
#else
#define BTHW_VVDBG(fmt,...) ((void *) 0)
#endif

enum {
	EVENT_OVERRUN_MASK = 0x01,
	EVENT_UNDERRUN_MASK = 0x02,
	EVENT_THRESHOLD_MASK = 0xC0,
};

struct bse_audio_queue {
	pthread_mutex_t mutex;
	/*pthread_cond_t  cond;*/
	size_t min_buffered_size;
	size_t read_idx;
	size_t write_idx;
	size_t delta;
	size_t size;
	unsigned char *data;
	queue_event_callback cbk;
	uint8_t event_flag;
};

bse_audio_queue_t *bse_audio_queue_create(size_t queue_size, size_t min_buffered, queue_event_callback evt_cbk)
{
	bse_audio_queue_t *queue = (bse_audio_queue_t *) malloc(sizeof(bse_audio_queue_t));

	BTHW_VVDBG("%s(queue size:%d,min buffered:%d)", __FUNCTION__, queue_size, min_buffered);
	if (queue != NULL) {
		memset(queue, 0, sizeof(bse_audio_queue_t));
		pthread_mutex_init(&queue->mutex, NULL);
		queue->size = queue_size;
		queue->min_buffered_size = min_buffered;
		queue->data = malloc(queue_size);
		queue->cbk = evt_cbk;
	}

	return queue;
}

void bse_audio_queue_destroy(bse_audio_queue_t *queue)
{
	if (queue != NULL) {
		if (queue->data) {
			free(queue->data);
		}

		pthread_mutex_destroy(&queue->mutex);
		free(queue);
	}
}

void bse_audio_queue_reset(bse_audio_queue_t *queue)
{
	if (queue != NULL) {
		pthread_mutex_lock(&queue->mutex);
		queue->read_idx = 0;
		queue->write_idx = 0;
		queue->delta = 0;
		pthread_mutex_unlock(&queue->mutex);
	}
}

void bse_audio_queue_write(bse_audio_queue_t *queue, const void *buffer, size_t offset, size_t size)
{
	int cp_len;
	unsigned char *dest, *src;
	uint8_t notify = 0;

	if (queue == NULL || queue->data == NULL) return;

	BTHW_VVDBG("->%s(offset:%d,size:%d):%lld", __FUNCTION__, offset, size, get_current_time());
	pthread_mutex_lock(&queue->mutex);

	// check whether overwrite
	if (queue->delta + size > queue->size) {
		BTHW_VDBG("overrun!!! %d + %d > %d.", queue->delta, size, queue->size);
		/* pthread_cond_wait(&queue->cond, &queue->mutex); */
		if (!(queue->event_flag & EVENT_OVERRUN_MASK)) {
			queue->event_flag |= EVENT_OVERRUN_MASK;
			if (queue->cbk != NULL) {
				queue->cbk(BSE_AUDIO_QUEUE_EVENT_OVERRUN, queue->size);
			}
		}
	} else {
		queue->event_flag &= ~EVENT_OVERRUN_MASK;
	}

	if (queue->delta * 2 > queue->size) {
		if (((queue->event_flag & EVENT_THRESHOLD_MASK) >> 2) != 1) {
			queue->event_flag &= ~EVENT_THRESHOLD_MASK;
			queue->event_flag |= (1 << 2);
			notify = 1;
		}
	} else if (queue->delta * 4 > queue->size) {
		if (((queue->event_flag & EVENT_THRESHOLD_MASK) >> 2) != 2) {
			queue->event_flag &= ~EVENT_THRESHOLD_MASK;
			queue->event_flag |= (2 << 2);
			notify = 1;
		}
	} else if (queue->delta * 8 > queue->size) {
		if (((queue->event_flag & EVENT_THRESHOLD_MASK) >> 2) != 3) {
			queue->event_flag &= ~EVENT_THRESHOLD_MASK;
			queue->event_flag |= (3 << 2);
			notify = 1;
		}
	} else {
		queue->event_flag &= ~EVENT_THRESHOLD_MASK;
	}

	if (queue->cbk != NULL && notify) {
		queue->cbk(BSE_AUDIO_QUEUE_EVENT_THRESHOLD, 2 ^ ((queue->event_flag & EVENT_THRESHOLD_MASK) >> 2));
	}

	queue->delta += size;
	dest = queue->data + queue->write_idx;
	src = buffer + offset;

	if (queue->write_idx + size >= queue->size) {
		cp_len = queue->size - queue->write_idx;
		memcpy(dest, src, cp_len);

		dest = queue->data;
		src += cp_len;
		cp_len = size - cp_len;
	} else {
		cp_len = size;
	}

	/* XXX cp_len may larger than queue->size ? */
	if (cp_len > 0) {
		memcpy(dest, src, cp_len);
	}

	queue->write_idx += size;
	queue->write_idx %= queue->size;
	/* pthread_cond_signal(&queue->cond); */
	pthread_mutex_unlock(&queue->mutex);
	BTHW_VVDBG("<-%s(offset:%d,size:%d):%lld", __FUNCTION__, offset, size, get_current_time());
}

int bse_audio_queue_read(bse_audio_queue_t *queue, const void *buffer, size_t offset, size_t size)
{
	size_t read_len = 0;
	size_t cp_len;
	unsigned char *src, *dest;

	if (queue == NULL || queue->data == NULL) return read_len;

	BTHW_VVDBG("->%s(offset:%d,size:%d):%lld", __FUNCTION__, offset, size, get_current_time());
	pthread_mutex_lock(&queue->mutex);

	if (queue->delta < queue->min_buffered_size) {
		BTHW_VDBG("underrun!!!");
		/* if no data available, block
		   pthread_cond_wait(&queue->cond, &queue->mutex); */
		pthread_mutex_unlock(&queue->mutex);
		if (queue->cbk != NULL) {
			if (!(queue->event_flag & EVENT_UNDERRUN_MASK)) {
				queue->event_flag |= EVENT_UNDERRUN_MASK;
				queue->cbk(BSE_AUDIO_QUEUE_EVENT_UNDERRUN, queue->min_buffered_size);
			}
		}
		return read_len;
	} else {
		queue->event_flag &= ~EVENT_UNDERRUN_MASK;
	}

	read_len = size > queue->delta ? queue->delta : size;

	dest = buffer + offset;
	src = queue->data + queue->read_idx;

	if (read_len + queue->read_idx >= queue->size) {
		cp_len = queue->size - queue->read_idx;
		memcpy(dest, src, cp_len);

		dest += cp_len;
		src = queue->data;
		cp_len = read_len - cp_len;
	} else {
		cp_len = read_len;
	}

	if (cp_len > 0) {
		memcpy(dest, src, cp_len);
	}

	queue->delta -= read_len;
	queue->read_idx += read_len;
	queue->read_idx %= queue->size;
	pthread_mutex_unlock(&queue->mutex);
	BTHW_VVDBG("<-%s(offset:%d,size:%d):%lld", __FUNCTION__, offset, size, get_current_time());

	return (int) read_len;
}

