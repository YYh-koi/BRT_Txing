#include "bt_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Auto_commfunc.h"

unsigned char hex_char_to_byte(UINT8 ch)
{
	UINT8 num = 0xFF;

	if ((ch >= '0') && (ch <= '9')) {
		num = ch - '0';
	} else if ((ch >= 'a') && (ch <= 'z')) {
		num = ch - 'a' + 10;
	} else if ((ch >= 'A') && (ch <= 'Z')) {
		num = ch - 'A' + 10;
	}

	return num;
}

UINT8 hex_chars_to_byte(UINT8 data[2])
{
	UINT8 m = hex_char_to_byte(data[0]);
	UINT8 l = hex_char_to_byte(data[1]);

	return ((m << 8) & 0xF0) | (l & 0x0F);
}


unsigned char hex_string_to_byte_array(UINT32 str_len, char *str, UINT8 *num_arr)
{
	UINT32 n = 0;
	UINT8 num = 0;

	if (str_len < 2) {
		return FALSE;
	}
	while (n < str_len) {
		if ((num = hex_char_to_byte(str[n++])) == 0xFF) {
			return FALSE;
		}
		*num_arr = num << 4;
		if ((num = hex_char_to_byte(str[n++])) == 0xFF) {
			return FALSE;
		}
		*num_arr |= num;
		num_arr++;
	}

	return TRUE;
}

unsigned char byte_array_to_hex_string(UINT8 *str, UINT8 *arr, UINT16 len)
{
	UINT16 off = 0;

	if (!str || !arr || !len) {
		return FALSE;
	}
	while (off < len) {
		sprintf((char *)(str + (off * 2)), "%02X", arr[off]);
		off++;
	}

	return TRUE;
}

unsigned char bd_colon_str_to_bytes(UINT8 *str, UINT8 *bd)
{
	UINT32 n = 0;
	UINT8 num = 0;
	UINT8 l = 5;

	if (strlen((char *)str) < 17) {
		return FALSE;
	}
	while (n < 18) {
		if ((num = hex_char_to_byte(str[n++])) == 0xFF) {
			return FALSE;
		}
		bd[l] = num << 4;
		if ((num = hex_char_to_byte(str[n++])) == 0xFF) {
			return FALSE;
		}
		bd[l--] |= num;
		n++;
	}

	return TRUE;
}

unsigned char parse_byte(char *p)
{
	char buf[3] = {0};
	memcpy(buf, p, 2);
	return strtol(buf, NULL, 16) ;
}

unsigned short parse_uuid16(char *p)
{
	char buf[5] = {0};
	memcpy(buf, p, 4);
	return strtol(buf, NULL, 16) ;
}

unsigned short parse_short(char *p)
{
	char buf[5] = {0};
	memcpy(buf, p, 4);
	return strtol(buf, NULL, 16) ;
}

unsigned int parse_int(char *p)
{
	char buf[9] = {0};
	memcpy(buf, p, 8);
	return strtol(buf, NULL, 16) ;
}

long parse_long(char *p)
{
	char buf[17] = {0};
	memcpy(buf, p, 17);
	return strtol(buf, NULL, 16) ;
}

static bt_bdaddr_t s_addr;

bt_bdaddr_t parse_address(char *p)
{
	int i = 0;
	char str[13];

	memset(str, 0, 13);
	memcpy(str, p, 12);
	char* end = &(str[12]);

	for(i = 0; i < 6; i++) {
		s_addr.address[i] = strtol(end - (i + 1) * 2 , NULL, 16) ;
		*(end - (i + 1) * 2) = 0;
	}

	return s_addr;
}

#define MAX_AT_CMD_PARAM_RAW_DATA_LEN 256
static unsigned char bytes[MAX_AT_CMD_PARAM_RAW_DATA_LEN];

unsigned char *parse_bytes(char *p, int *len)
{
	unsigned short result_len = 0;

	if (*len > MAX_AT_CMD_PARAM_RAW_DATA_LEN) {
		*len = 0;
		return 0;
	}

	str_bytes_to_raw_bytes(p, (unsigned short) *len, bytes, &result_len);
	*len = (int) (result_len & 0xFFFF);

	return bytes;
}

/*
   "123412341234" ---- {0x34,0x12}
*/
void parse_bd(char *p, unsigned char bd[6])
{
	int i = 0;
	char bd_buf[13];
	memset(bd_buf, 0, 13);
	memcpy(bd_buf, p, 12);
	char* p_bd_buf_end = &(bd_buf[12]);

	for(i = 0; i < 6; i++) {
		bd[i] = strtol(p_bd_buf_end -(i+1)*2 , NULL, 16) ;
		*(p_bd_buf_end -(i+1)*2) = 0;
	}
}
#ifdef CONFIG_GATT
void parse_srvc_id(char *p, btgatt_srvc_id_t* srvc_id)
{
	UINT8 base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	                     0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
	                    };

	srvc_id->is_primary = parse_byte(p);
	srvc_id->id.inst_id = parse_short(p + 2);
	short srvc_uuid = parse_short(p + 6);
	base_uuid[12] = srvc_uuid & 0xff;
	base_uuid[13] = (srvc_uuid & 0xff00) >> 8;
	memcpy(srvc_id->id.uuid.uu, base_uuid, 16);
}

void parse_char_id(char *p, btgatt_gatt_id_t* char_id)
{
	UINT8 base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	                     0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
	                    };

	char_id->inst_id = parse_short(p);
	short char_uuid = parse_short(p + 4);
	base_uuid[12] = char_uuid & 0xff;
	base_uuid[13] = (char_uuid & 0xff00) >> 8;

	memcpy(char_id->uuid.uu, base_uuid, 16);
}

void parse_descr_id(char *p, bt_uuid_t* descr_id)
{
	UINT8 base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	                     0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
	                    };
	short descr_uuid = parse_short(p);
	base_uuid[12] = descr_uuid & 0xff;
	base_uuid[13] = (descr_uuid & 0xff00) >> 8;

	memcpy(descr_id->uu, base_uuid, 16);
}
#endif

//#ifdef CONFIG_SPP
unsigned char is_uuid_valid(UINT8 uuid[16])
{
	int i = 0;
	for(i = 0; i < 16; i++) {
		if(uuid[i] != 0) {
			return 1;
		}
	}
	return 0;
}

void parse_uuid128(char *p, bt_uuid_t *uuid_128)
{
	int i  = 0;
	char buf[3] = {0};
	for(i = 0; i < 16; i++) {
		memcpy(buf, p, 2);
		uuid_128->uu[i] = (unsigned char)strtol(buf, NULL, 16) ;
		p += 2;
	}
}

//#endif

void parse_bd_address(const char *str, UINT8 *bd)
{
	int i;
	char hex[5];

	strcpy(hex, "0xFF");

	for(i=0; i<6; i++) {
		strncpy(&hex[2], str, 2);
		str += 2;
		bd[5-i] = (UINT8) strtol(hex, NULL, 16);
	}
}

