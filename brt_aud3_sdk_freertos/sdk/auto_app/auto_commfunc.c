#include "auto_commfunc.h"
#include "auto_gap.h"
#include "gap_ui.h"
#include "hci_ui.h"
#include <stdlib.h>
#include <unistd.h>
#include "bt_functions.h"
#include "bt_event.h"

#define TAG "bse_com"

#define TERMINAL_CHAR1 '\0'
#define TERMINAL_CHAR2 '\r'
#define TERMINAL_CHAR3 '\n'
#define SBUSTITUE_BYTE  220 //0xDC
#define SBUSTITUE_BYTE1 221 //0xDD
#define SBUSTITUE_BYTE2 222 //0xDE
#define SBUSTITUE_BYTE3 223 //0xDF

unsigned char ctoi(char c)
{
	if((c >= 'A') && ((c <= 'F')))
		return (c-'A'+0x0A);
	if((c >= 'a') && ((c <= 'f')))
		return (c-'a'+0x0A);
	if((c >= '0') && ((c <= '9')))
		return (c-'0'+0x00);
	return 0;
}

unsigned int hlp_strnlen(const char *str, unsigned int maxsize)
{
	unsigned int n;

	/* Note that we do not check if s == NULL, because we do not
	* return errno_t...
	*/

	for (n = 0; n < maxsize && *str; n++, str++)
		;

	return n;
}

unsigned char * utf8_strncpy(unsigned char * dest, const unsigned char * utf8str,  int length)
{
	int copied = 0;
	int len;
	int i = 0;
	int n = 0;
	const unsigned char * pt;
	const unsigned char * end;
	unsigned char * uc;

	if (!dest || !utf8str) {
		return 0;
	}

	len = hlp_strnlen(utf8str, length);
	pt = utf8str;
	end = pt + len;
	uc = dest;

	memset(uc, 0, length);
	while (pt < end && *pt != '\0') {
		n = 0;
		if (*pt <= 0x7f) {
			n = 1;
		} else if (*pt <= 0xdf) {
			n = 2;
		} else if (*pt <= 0xef) {
			n = 3;
		} else {
			n = 4; // FIXME: may be 5,6 bytes in UTF-8
		}

		if (copied + n > len) break;

		for (i=0; i<n && copied < len; i++) {
			*uc++ = *pt++;
			copied++;
		}

	}
	return dest;
}

unsigned char * raw_bytes_to_str_bytes(const unsigned char *raw_bytes, unsigned short raw_len,
                                       unsigned char *result_bytes, unsigned short *result_len)
{
	int terminal_bytes = 0;
	int i;
	int j = 0;

	if (raw_bytes == NULL) {
		if (result_len != NULL) {
			*result_len = 0;
		}
		return NULL;
	}

	for (i=0; i<raw_len; i++) {
		if ((raw_bytes[i] == TERMINAL_CHAR1) ||
		    (raw_bytes[i] == TERMINAL_CHAR2) ||
		    (raw_bytes[i] == TERMINAL_CHAR3))  {
			terminal_bytes++;
		}
		if (raw_bytes[i] == SBUSTITUE_BYTE) {
			terminal_bytes++;
		}
	}

	if (result_len != NULL) {
		*result_len = raw_len + terminal_bytes;
	}

	if (result_bytes != NULL) {
		for (i=0; i<raw_len; i++, j++) {
			if (raw_bytes[i] == TERMINAL_CHAR1) {
				result_bytes[j] = SBUSTITUE_BYTE;
				j++;
				result_bytes[j] = SBUSTITUE_BYTE1;
			} else if (raw_bytes[i] == TERMINAL_CHAR2) {
				result_bytes[j] = SBUSTITUE_BYTE;
				j++;
				result_bytes[j] = SBUSTITUE_BYTE2;
			} else if (raw_bytes[i] == TERMINAL_CHAR3) {
				result_bytes[j] = SBUSTITUE_BYTE;
				j++;
				result_bytes[j] = SBUSTITUE_BYTE3;
			} else if (raw_bytes[i] == SBUSTITUE_BYTE) {
				result_bytes[j] = SBUSTITUE_BYTE;
				j++;
				result_bytes[j] = SBUSTITUE_BYTE;
			} else {
				result_bytes[j] = raw_bytes[i];
			}
		}
	}

	return result_bytes;
}

unsigned char * str_bytes_to_raw_bytes(const unsigned char *raw_bytes, unsigned short raw_len,
                                       unsigned char *result_bytes, unsigned short *result_len)
{
	int terminal_bytes = 0;
	int i, j = 0;

	if (raw_bytes == NULL) {
		if (result_len != NULL) {
			*result_len = 0;
		}
		return NULL;
	}


	for (i = 0; i < raw_len; i++) {
		if (raw_bytes[i] == SBUSTITUE_BYTE) {
			terminal_bytes++;
			i++;
		}
	}

	if (terminal_bytes >= raw_len) {
		if (result_len != NULL) {
			*result_len = 0;
		}
		return NULL;
	}

	if (result_len != NULL) {
		*result_len = raw_len - terminal_bytes;
	}

	for (i = 0; i < raw_len; i++, j++) {
		if (raw_bytes[i] == SBUSTITUE_BYTE && i < raw_len - 1) {
			if (raw_bytes[i+1] == SBUSTITUE_BYTE) {
				result_bytes[j] = SBUSTITUE_BYTE;
			} else if (raw_bytes[i+1] == SBUSTITUE_BYTE1) {
				result_bytes[j] = TERMINAL_CHAR1;
			} else if (raw_bytes[i+1] == SBUSTITUE_BYTE2) {
				result_bytes[j] = TERMINAL_CHAR2;
			} else if (raw_bytes[i+1] == SBUSTITUE_BYTE3) {
				result_bytes[j] = TERMINAL_CHAR3;
			} else {
				// invalid byte
			}
			i++;
		} else {
			result_bytes[j] = raw_bytes[i];
		}
	}
	return result_bytes;
}

#if defined(CONFIG_OS_WIN32) || defined(CONFIG_OS_WINCE)
int snprintf(char *str, size_t size, const char *format, ...)
{
	int remain = 0;
	va_list ap;

	memset(str, 0, size);
	va_start(ap, format);
	remain = _vsnprintf(str, size-1, format, ap);
	va_end(ap);

	return remain;
}
#endif

