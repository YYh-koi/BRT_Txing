#ifndef WAV_FILE_H__
#define WAV_FILE_H__

#include <stddef.h>

typedef void *WAV_FILE_HANDLE;
#define WAV_FILE_INVALID_HANDLE ((void *)(-1))

WAV_FILE_HANDLE wav_file_open(const char *name, int rate, int channels, int bits_per_sample);
int wav_file_write(WAV_FILE_HANDLE handle, void *buffer, size_t size);
void wav_file_close(WAV_FILE_HANDLE handle);

#endif //WAV_FILE_H__
