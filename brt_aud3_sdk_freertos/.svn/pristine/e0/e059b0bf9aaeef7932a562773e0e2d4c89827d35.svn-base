#include <sys/types.h>
//#include <sys/stat.h>
#include "vcf_parse.h"
#include "sysdep.h"
#include "bt_event.h"
#include "auto_pbap.h"
#include "brt_trace.h"
#include "photo_parser.h"

#define LOG(fmt, ...) bse_log_info("photo_parser", fmt, ## __VA_ARGS__)

#define MAX_PHOTO_ITEM_LEN (50*1024)

#define TERMINAL_CHAR1 '\0'
#define TERMINAL_CHAR2 '\r'
#define TERMINAL_CHAR3 '\n'

#define SBUSTITUE_BYTE  0xDC

static UINT8 s_filder_name[128];

void creat_filer_name(UINT8 *bd)
{
	memset(s_filder_name, 0, 128);
	sprintf(s_filder_name, "/data/misc/brlink/%02X%02X%02X%02X%02X%02X",
	        bd[5], bd[4], bd[3], bd[2], bd[1], bd[0]);
	rmdir(s_filder_name);
	LOG("creat %s", s_filder_name);
	mkdir(s_filder_name, 0777);
}

void clear_filer_name()
{
	memset(s_filder_name, 0, 128);
}

void app_photo_parser_stor(photo_parser_t *p_parser)
{
	unsigned short nItemNameLen = 0;
	unsigned short nItemPropLen = 0;
	unsigned short nItemValLen = 0;
	if (p_parser == NULL)
		return;

	if (p_parser->p_item_name_start) {

		nItemNameLen = p_parser->p_item_name_end - p_parser->p_item_name_start + 1;

		if (nItemNameLen >= MAX_PHOTO_ITEM_LEN) {
			LOG("Name Item(%d) too long(larger than %d), should be cut.",
			              nItemNameLen, MAX_PHOTO_ITEM_LEN);
			nItemNameLen = MAX_PHOTO_ITEM_LEN - 1;
		}

		if (nItemNameLen > 0) {
			memcpy(p_parser->p_pb_item->sLine, p_parser->p_item_name_start, nItemNameLen);

			if(*(p_parser->p_item_name_end + 1) == ';') {
				memcpy(p_parser->p_pb_item->sLine + nItemNameLen, ";", 1);
				nItemNameLen += 1;
			}
			p_parser->p_item_name_start = p_parser->p_pb_item->sLine;
			p_parser->p_item_name_end = p_parser->p_item_name_start + nItemNameLen - 1;
		}

	}

	if (p_parser->p_item_prop_start) {

		nItemPropLen = p_parser->p_item_prop_end - p_parser->p_item_prop_start + 1;

		if (nItemNameLen + nItemPropLen >= MAX_PHOTO_ITEM_LEN) {
			LOG("Name plus Property item(%d) too long(larger than %d), should be cut.",
			              (nItemNameLen + nItemPropLen), MAX_PHOTO_ITEM_LEN);
			nItemPropLen = MAX_PHOTO_ITEM_LEN - nItemNameLen - 1;
		}

		if (nItemPropLen > 0) {
			memcpy(p_parser->p_pb_item->sLine + nItemNameLen, p_parser->p_item_prop_start, nItemPropLen);
			if(*(p_parser->p_item_prop_end + 1) == ':') {
				memcpy(p_parser->p_pb_item->sLine + nItemNameLen + nItemPropLen, ":", 1);
				nItemNameLen += 1;
			}
			p_parser->p_item_prop_start = p_parser->p_pb_item->sLine + nItemNameLen;
			p_parser->p_item_prop_end = p_parser->p_item_prop_start + nItemPropLen - 1;
		}
	}

	if (p_parser->p_item_val_start) {

		nItemValLen = p_parser->p_item_val_end - p_parser->p_item_val_start + 1;

		if (nItemNameLen + nItemPropLen + nItemValLen >= MAX_PHOTO_ITEM_LEN) {
			LOG("Name plus Property plus Value item(%d) too long(larger than %d), should be cut.",
			              (nItemNameLen + nItemPropLen + nItemValLen), MAX_PHOTO_ITEM_LEN);
			nItemValLen = (MAX_PHOTO_ITEM_LEN - nItemPropLen - nItemNameLen - 1);
		}

		if (nItemValLen > 0) {
			memcpy(p_parser->p_pb_item->sLine + nItemNameLen + nItemPropLen, p_parser->p_item_val_start, nItemValLen);
			p_parser->p_item_val_start = p_parser->p_pb_item->sLine + nItemNameLen + nItemPropLen;
			p_parser->p_item_val_end = p_parser->p_item_val_start + nItemValLen - 1;
		}
	}

	p_parser->p_pb_item->nCount = nItemNameLen + nItemPropLen + nItemValLen;

	if (p_parser->p_pb_item->nCount > 0) {
		p_parser->b_continue = TRUE;
	}
	memset((p_parser->p_pb_item->sLine + p_parser->p_pb_item->nCount), 0, (MAX_PHOTO_ITEM_LEN - p_parser->p_pb_item->nCount));

}
static char *photo_strstr(const char *s_start, const char *s_end, const char *needle)
{
	int i;
	size_t needle_len;
	char *ptr;

	/* segfault here if needle is not NULL terminated */
	if (0 == (needle_len = strlen(needle)))
		return (char *)s_start;

	/* Limit the search if s_start is shorter than 'len' */
	i = 0;
	for(ptr=s_start; ptr<=s_end; ptr++) {
		i++;
		if (*ptr == '\0') {
			break;
		}
	}

	if (i < needle_len) {
		return NULL;
	}

	for(ptr=s_start; ptr<=s_end-needle_len+1; ptr++) {
		if ((ptr[0] == needle[0]) &&
		    (0 == strncmp(ptr, needle, needle_len)))
			return (char *) ptr;
	}

	return NULL;
}
static char hlp_pbapparser_check_encoding(photo_parser_t *p_parser, const UINT *sEncoding)
{
	return (p_parser->p_item_prop_start && photo_strstr(p_parser->p_item_prop_start, p_parser->p_item_prop_end, sEncoding));
}
void app_photo_parser_free(photo_parser_t *p_parser)
{
	if (p_parser != NULL) {
		if (p_parser->p_pb_item != NULL) {
			FREE(p_parser->p_pb_item);
		}

		FREE(p_parser);
	}
}

photo_parser_t *app_photo_parser_alloc(UINT8 store_type, UINT8 *p_parser_name)
{
	photo_parser_t *p_parser = (photo_parser_t *)NEW(sizeof(photo_parser_t));
	LOG("app_photo_parser_alloc, parser name: %s", p_parser_name);

	if (p_parser != NULL) {
		memset(p_parser, 0, sizeof(photo_parser_t));
#ifdef VCF_PARSER_DEBUG_ENABLE
		memcpy(p_parser->parser_name, p_parser_name, strlen(p_parser_name) + 1);
#endif
		p_parser->p_pb_item = (PBITEMP *)NEW(sizeof(PBITEMP));

		if (p_parser->p_pb_item == NULL) {
			FREE(p_parser);
			return NULL;
		}

		memset(p_parser->p_pb_item, 0, sizeof(PBITEMP));
		p_parser->item_type = VCF_PARSE_TYPE_NONE;
		p_parser->store_type = store_type;
		p_parser->status = PARSE_IDLE;
	}

	return p_parser;
}
static UINT8 hlp_photo_parser_check_item_name(photo_parser_t *p_parser, const UINT8 *sItemName)
{
	int nlen = 0;
	int len = 0;

	if (!p_parser->p_item_name_start || !p_parser->p_item_name_end) {
		return 0;
	}

	nlen = p_parser->p_item_name_end - p_parser->p_item_name_start + 1;
	len = strlen(sItemName);

	return ((nlen == len) && (memcmp(p_parser->p_item_name_start, sItemName, len) == 0));

}
enum {
    PHOTO_NONE,
    PHOTO_START,
    PHOTO_STARTING,
    PHOTO_END,
};
static int s_photo_offset = 0;
static int s_photo_state = PHOTO_NONE;
static UINT8 s_photo_buf[MAX_PHOTO_ITEM_LEN];
static int s_index = 0;
static void base_decode(const photo_parser_t *p_parser)
{

	//FILE *photo_fp = NULL;
	unsigned char photo_name[128];
	int decode_len = 0;
	int buf_len = 0;
	unsigned char *photo_buf = NULL;
	unsigned char *s = NULL;


	memcpy(s_photo_buf + strlen(s_photo_buf), "\r\n", 2);
	s = strstr(s_photo_buf, "JPEG");
	if(s == NULL) {
		return;
	}
	s += 4;
	if(*s == ':') {
		s++;
	}

//    s_photo_x = fopen("/data/misc/brlink/photo_x", "a+");
//    fwrite(s, strlen(s), 1, s_photo_x);
	LOG("index %d, len is %d\r\n", p_parser->p_pb_item->pb_index, strlen(s));

	decode_len = Base64decode_len(s);
	buf_len = decode_len + 4 + 4 + 1;   // pb_index(4) decode_len(4)
	photo_buf = (unsigned char *)NEW(buf_len);

	memset(photo_buf, 0, buf_len);
	sprintf(photo_buf, "%04X%04X", p_parser->p_pb_item->pb_index, decode_len);
	Base64decode((photo_buf + 8), s);

	LOG("strlen photo_buf is %d", strlen(photo_buf));

#if  0
	unsigned char * t_photo = photo_buf + 8;
	if(access(s_filder_name, 0) == 0) {
		LOG("%s", "file exist");
		sprintf(photo_name, "%s/%d", s_filder_name, p_parser->p_pb_item->pb_index);
		photo_fp = fopen(photo_name, "w+");
		chmod(photo_name, 0777);
		if(photo_fp) {
			fwrite(t_photo, decode_len, 1, photo_fp);
		}
		fclose(photo_fp);
	} else {
		LOG("%s", "file not exist");
	}
#else

	int photo_event_len = 0;
	unsigned char *photo_event_buf = NULL;
	int count = check_terminal_char(photo_buf, buf_len);

	photo_event_buf = (unsigned char *)NEW(buf_len + count + 1);
	raw_bytes_to_str_bytes(photo_buf, buf_len, photo_event_buf, &photo_event_len);

	UINT8 photo_len_str[5];
	memset(photo_len_str, 0, 5);
	sprintf(photo_len_str, "%04x", photo_event_len);
	memcpy((photo_event_buf + 4), photo_len_str, 4);

	p_parser->item_callback(EV_PBAP_PHONEBOOK_PHOTODATA_IND, photo_event_buf);
	FREE(photo_event_buf);

#endif
	FREE(photo_buf);

//    fclose(s_photo_x);

	LOG("leave base_decode, photo len is %d\r\n", decode_len);
}

int check_terminal_char(const unsigned char *raw_bytes, int raw_len)
{
	int i = 0;
	int terminal_bytes = 0;

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
	return terminal_bytes;
}


static void cpy_photo_data(const photo_parser_t *p_parser)
{
	int i = 0;
	int j = 0;
	int len = 0;
	int buf_len = 0;
	BOOL is_end = FALSE;
	UINT8* src = NULL;
	src = p_parser->p_item_name_start;
	buf_len = strlen(p_parser->p_item_name_start);
	UINT8* s = NULL;
	if(s_photo_state == PHOTO_START) {
		s_photo_state = PHOTO_STARTING;
		if(s = strstr(p_parser->p_item_name_start, "\r\n")) {
			buf_len = s - p_parser->p_item_name_start;
			s_photo_state = PHOTO_STARTING;
		}
	} else if (s_photo_state == PHOTO_STARTING) {

		if(s = strstr(p_parser->p_item_name_start, "\r\n\r\n")) {
			buf_len = s - p_parser->p_item_name_start;
			s_photo_state = PHOTO_END;
		}
	}



	for(i = 0; i < buf_len; i++) {
		if(*(src + i) == '\r'|| *(src + i) == '\n' || *(src + i) > 0x7A || *(src + i) == ' ') {
			continue;
		}
		s_photo_buf[s_photo_offset + j] = *(src + i);
		j++;
	}
	s_photo_offset += j ;
	if(s_photo_state == PHOTO_END) {
		base_decode(p_parser);
		s_photo_state = PHOTO_NONE;
	}

}
static void hlp_photo_parser_process(photo_parser_t *p_parser)
{
	if (p_parser == NULL)
		return;
	if(s_photo_state >= PHOTO_START) {
		cpy_photo_data(p_parser);
	}
	if (hlp_photo_parser_check_item_name(p_parser, "BEGIN")) {
		s_photo_state = PHOTO_NONE;
		p_parser->p_pb_item->pb_index++;
		LOG("[DBG_LOG] BEGIN item index is %d\r\n", p_parser->p_pb_item->pb_index);
		//TODO
	}
	if(hlp_photo_parser_check_item_name(p_parser, "PHOTO")) {
		//TODO

		s_photo_offset = 0;
		s_photo_state = PHOTO_START;
		memset(s_photo_buf, 0, MAX_PHOTO_ITEM_LEN);
		LOG("[DBG_LOG] PHOTO item index is %d\r\n", p_parser->p_pb_item->pb_index);
		cpy_photo_data(p_parser);
	} else if (hlp_photo_parser_check_item_name(p_parser, "END")) {
		//s_photo_state = PHOTO_END;
		//TODO
	}
}

static void hlp_photo_parser_clean_cache(photo_parser_t *p_parser)
{
	if (p_parser == NULL)
		return;

	p_parser->p_item_name_start = NULL;
	p_parser->p_item_name_end = NULL;
	p_parser->p_item_prop_start = NULL;
	p_parser->p_item_prop_end = NULL;
	p_parser->p_item_val_start = NULL;
	p_parser->p_item_val_end = NULL;
	p_parser->b_continue = FALSE;
	p_parser->is_encoding_QP = 0;
	p_parser->status = PARSE_IDLE;
}

void app_photoparser_data_ind(photo_parser_t *p_parser, UINT8 *p_input)
{
	UINT8 *p = 0;
	if(!p_parser) {
		return ;
	}

	if (p_parser->b_continue == TRUE) {
		// So, I am small, not feed me too much
		if (p_parser->p_pb_item->nCount >= MAX_PHOTO_ITEM_LEN) {
			LOG("%s", "So, I am small, not feed me too much");
			//TODO
		}

		p_parser->p_pb_item->sLine[p_parser->p_pb_item->nCount] = (*p_input);
		p = p_parser->p_pb_item->sLine + p_parser->p_pb_item->nCount;
		p_parser->p_pb_item->nCount++;
	} else {
		p = p_input;
	}

	switch (p_parser->status) {
	case PARSE_IDLE: {
		if ((*p) == ';') {
			p_parser->status = PARSE_ITEM_NAME_GOT;
		} else if ((*p) == ':') {
			p_parser->status = PARSE_ITEM_PROP_GOT;
		} else {
			if (p_parser->p_item_name_start == NULL) {
				p_parser->p_item_name_start = p;
			}

			p_parser->p_item_name_end = p;
		}
	}
	break;

	case PARSE_ITEM_NAME_GOT: {
		if ((*p) == ':') {
			p_parser->status = PARSE_ITEM_PROP_GOT;
		} else {
			if (p_parser->p_item_prop_start == NULL) {
				p_parser->p_item_prop_start = p;
			}

			p_parser->p_item_prop_end = p;
		}
	}
	break;

	case PARSE_ITEM_PROP_GOT: {
		if (((*p) == '\n') && p_parser->p_item_val_end && ((*p_parser->p_item_val_end) == '\r')) {
#ifdef CONFIG_UNLIMIT_RESOURCE
			if (hlp_pbapparser_check_encoding(p_parser, "QUOTED-PRINTABLE")) {
				p_parser->is_encoding_QP = 1;
			}
			if (((*(p_parser->p_item_val_end-1) != '=') && p_parser->is_encoding_QP) || (!p_parser->is_encoding_QP))
#endif
			{
				p_parser->status = PARSE_ITEM_VAL_GOT;

				if (p_parser->p_item_val_start == p_parser->p_item_val_end) {
					p_parser->p_item_val_start = NULL;
					p_parser->p_item_val_end = NULL;
				} else {
					p_parser->p_item_val_end--;
				}

				hlp_photo_parser_process(p_parser);
				hlp_photo_parser_clean_cache(p_parser);
				p_parser->status = PARSE_IDLE;
			}
		} else {
			if (p_parser->p_item_val_start == 0) {
				p_parser->p_item_val_start = p;
			}

			p_parser->p_item_val_end = p;
		}
	}
	break;

	default:
		break;
	}
}

