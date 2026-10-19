#ifndef _BSE_AUDIO_QUEUE_H__
#define _BSE_AUDIO_QUEUE_H__
#include <stdlib.h>
#include "bluedroid/bluetooth.h"

struct bse_audio_queue;
typedef struct bse_audio_queue bse_audio_queue_t;

enum {
	BSE_AUDIO_QUEUE_EVENT_OVERRUN,
	BSE_AUDIO_QUEUE_EVENT_UNDERRUN,
	BSE_AUDIO_QUEUE_EVENT_THRESHOLD,
};

typedef void (*queue_event_callback)(uint8_t event, int param);
bse_audio_queue_t *bse_audio_queue_create(size_t queue_size, size_t min_buffered, queue_event_callback evt_cbk);
void bse_audio_queue_destroy(bse_audio_queue_t *queue);
void bse_audio_queue_reset(bse_audio_queue_t *queue);
void bse_audio_queue_write(bse_audio_queue_t *queue, const void *buffer, size_t offset, size_t size);
int bse_audio_queue_read(bse_audio_queue_t *queue, const void *buffer, size_t offset, size_t size);

#endif  /*_BSE_AUDIO_QUEUE_H__*/

