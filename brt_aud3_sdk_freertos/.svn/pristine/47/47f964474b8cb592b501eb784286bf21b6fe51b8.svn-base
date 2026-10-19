#ifndef _APP_COMM_FUNC_H_
#define _APP_COMM_FUNC_H_

#include "btdatatype.h"
#include "brt_trace.h"

#define CMD_NAME_LEN	2

#define LOBYTE(w)           ((BYTE)((unsigned long)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((unsigned long)(w) >> 8))
#define MAKEWORD(low,high)  ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))

unsigned char ctoi(char c);
unsigned char * utf8_strncpy(unsigned char * dest, const unsigned char * utf8str,  int length);

/**
  * Convert byte stream which may include NULL-terminated character(\0) to a new byte stream.
  * The new byte stream will exclude any NULL-terminated character.
  * NOTE: the new byte stream may be larger than the original byte stream.
  *       You can call the "str_bytes_to_raw_bytes" to convert it back to the original byte stream.
  *
  * Parameters:
  * raw_bytes:   The original byte stream which want to be converted.
  * raw_length:  The length of the raw bytes.
  * result_byte: The buffer which keep the result byte stream. If is NULL, no conversion will be done,
  *              but just return the length the result byte stream through the output parameter result_len,
  *              else, the caller should keep the space is enough to keep the result.
  * result_len:  [output parameter]. The length of the result byte stream.
  *
  * Return:
        the pointer of the result byte stream.
  */
unsigned char * raw_bytes_to_str_bytes(const unsigned char *raw_bytes, unsigned short raw_len,
                                       unsigned char *result_bytes, unsigned short *result_len);

/**
  * Do the reverse conversion as "raw_bytes_to_str_bytes".
  *
  * Parameters:
  * raw_bytes:  The original byte stream which want to be converted.
  * raw_length: the length of the raw bytes.
  * result_byte: the buffer which keep the result byte stream. If is NULL, no conversion will be done,
  *              but just return the length the result byte stream through the output parameter result_len.
  * result_len:  [output parameter]. The length of the result byte stream.
  *
  * Return:
        the pointer of the result byte stream.
  */
unsigned char * str_bytes_to_raw_bytes(const unsigned char *raw_bytes, unsigned short raw_len,
                                       unsigned char *result_bytes, unsigned short *result_len);

#endif
