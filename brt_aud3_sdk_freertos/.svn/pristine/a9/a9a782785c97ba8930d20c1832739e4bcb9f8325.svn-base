#ifndef RING_BUFFER_H__
#define RING_BUFFER_H__

typedef struct tag_ring_buffer_base {
	int (*push)(struct tag_ring_buffer_base *obj, unsigned char *buffer, int length);
	int (*pull)(struct tag_ring_buffer_base *obj, unsigned char *buffer, int length);
	int (*avail)(struct tag_ring_buffer_base *obj);
	void (*start)(struct tag_ring_buffer_base *obj);
	void (*stop)(struct tag_ring_buffer_base *obj);
} ring_buffer_t;

ring_buffer_t *audio_ring_buffer_create(int buf_size, int need_block, const char *name);
void audio_ring_buffer_free(ring_buffer_t *obj);
int aduio_ring_buffer_count(ring_buffer_t *obj);

#endif /*RING_BUFFER_H__*/
