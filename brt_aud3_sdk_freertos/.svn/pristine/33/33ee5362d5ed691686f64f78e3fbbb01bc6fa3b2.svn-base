#include "wav_file.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

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

static struct wav_header header = {
	.riff_id = ID_RIFF,
	.riff_sz = 0,
	.riff_fmt = ID_WAVE,
	.fmt_id = ID_FMT,
	.fmt_sz = 16,
	.audio_format = FORMAT_PCM,
	.sample_rate = 8000,
	.data_id = ID_DATA
};

#define list_entry(LIST_ELEM, STRUCT, MEMBER) \
    ((STRUCT *) ((uint8_t *) LIST_ELEM - offsetof (STRUCT, MEMBER)))

typedef struct tag_elem {
	struct tag_elem *prev;
	struct tag_elem *next;
} list_elem_t;

typedef struct tag_list {
	list_elem_t *head;
	list_elem_t *tail;
} list_t;

typedef struct tag_wav_file {
	list_elem_t list_elem;
	int rate;
	int channels;
	int bits;
	int frames;
	WAV_FILE_HANDLE file;
} wav_file_t;

static void list_insert(list_elem_t *before, list_elem_t *elem)
{
	if (!before || !elem) return;

	elem->prev = before->prev;
	elem->next = before;

	if (before->prev)
		before->prev->next = elem;

	before->prev = elem;
}

static list_t *list = 0;

WAV_FILE_HANDLE wav_file_open(const char *name, int rate, int channels, int bits)
{
	wav_file_t *file;
	FILE *fp = 0;

	if (!name)
		return WAV_FILE_INVALID_HANDLE;

	fp = fopen(name, "wb+");

	if (!fp)
		return WAV_FILE_INVALID_HANDLE;

	file = (wav_file_t *) malloc(sizeof(wav_file_t));
	memset(file, 0, sizeof(wav_file_t));
	file->file = fp;
	file->rate = rate;
	file->channels = channels;
	file->bits = bits;
	/*
	header.num_channels = s_channels;
	header.bits_per_sample = bits;
	header.byte_rate = (header.bits_per_sample / 8) *  header.num_channels * 8000;
	header.block_align = header.num_channels * (header.bits_per_sample / 8);
	*/
	/* leave enough room for header */
	fseek(fp, sizeof(struct wav_header), SEEK_SET);
	list_elem_t *e = &file->list_elem;

	if (list == 0) {
		list = (list_t *) malloc(sizeof(list_t));
		list->head = e;
		list->tail = 0;
	} else {
		list_insert(list->head, e);
		list->head = e;
	}

	return fp;
}

wav_file_t *find_wav_file_by_handle(WAV_FILE_HANDLE handle)
{
	wav_file_t *file = 0;
	list_elem_t *e;

	for (e = list->head; e; e = e->next) {
		file = list_entry(e, wav_file_t, list_elem);

		if (file && file->file == handle) {
			return file;
		}
	}

	return 0;
}

int wav_file_write(WAV_FILE_HANDLE handle, void *buffer, size_t size)
{
	wav_file_t *file = find_wav_file_by_handle(handle);

	if (file) {
		file->frames += (size / (file->channels * (file->bits >> 3)));
		return fwrite(buffer, 1, size, (FILE *) handle);
	}

	return 0;
}

void wav_file_close(WAV_FILE_HANDLE handle)
{
	FILE *fp = (FILE *) handle;
	list_elem_t *e;
	wav_file_t *target = find_wav_file_by_handle(handle);

	if (target) {
		header.sample_rate = target->rate;
		header.num_channels = target->channels;
		header.bits_per_sample = target->bits;
		header.byte_rate = (header.bits_per_sample / 8) *  header.num_channels * header.sample_rate;
		header.block_align = header.num_channels * (header.bits_per_sample / 8);
		header.data_sz = target->frames * header.block_align;
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(&header, sizeof(struct wav_header), 1, fp);
	fclose(fp);
}

