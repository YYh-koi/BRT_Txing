#include "vcf_parse.h"
#include "sysdep.h"
#include "bt_event.h"
// FIXME EV_PBAP_XXX should NOT be visible in here
#include "auto_pbap.h"
// FIXME EV_MAP_BMSG_DATA_IND should NOT be visible in here
#include "auto_map.h"
#include "brt_trace.h"
#include "internal_pbap.h"
#include "auto_commfunc.h"

#define LOG(fmt, ...) bse_log_info("vcf_parser", fmt, ## __VA_ARGS__)
#define MAX_BUF_LEN 1024

#ifdef CONFIG_PBAP_RAWDATA
static BOOL s_parser_start = FALSE;
static int s_number_count = 0;
static int s_buf_offset = 0;
static char s_raw_data_buf[MAX_BUF_LEN];
#endif

#define VCF_PARSE_DEBUG 1
#define WL_DEBUG

static char *vcf_strstr(const char *s_start, const char *s_end, const char *needle)
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
		if (*ptr != '\0') {
			i++;
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

static UINT8 *hlp_vcf_get_item_value(UINT8 *p_src, UINT8 *p_dest, UINT32 max_len)
{
	UINT8 *p_src_t = p_src;
	UINT8 *p_dest_t = p_dest;
	max_len--;			//'\0'

	while (p_src_t && *p_src_t != '\0' && max_len > 0) {
//		if ((*(p_src_t) == '\r') && (*(p_src_t + 1) == '\n'))
//			break;

		*p_dest_t++ = *p_src_t++;
		max_len--;
	}

	*p_dest_t = '\0';
	return p_dest;
}

#define VCF_PARSE_BEGIN_TYPE(_dest, _type)	(_dest |= (0x01<<_type))
#define VCF_PARSE_CHECK_TYPE(_dest, _type)	((_dest>>_type) & 0x01)
#define VCF_PARSE_END_TYPE(_dest, _type)	(_dest &= ~(0x01<<_type))

/*hly add for name split*/
static void dev_name(PBNAMESTRU *pb_name_stru, const UINT8 *pb_name, UINT type)
{
	if (pb_name == NULL) {
		return ;
	}

	switch (type) {
	case 0: {
		utf8_strncpy(pb_name_stru->lastName, pb_name, strlen(pb_name));
	}
	break;

	case 1: {
		utf8_strncpy(pb_name_stru->firstName, pb_name, strlen(pb_name));
	}
	break;

	case 2: {
		utf8_strncpy(pb_name_stru->middleName, pb_name, strlen(pb_name));
	}
	break;

	case 3: {
		utf8_strncpy(pb_name_stru->prefix, pb_name, strlen(pb_name));
	}
	break;

	case 4: {
		utf8_strncpy(pb_name_stru->suffix, pb_name, strlen(pb_name));
	}
	break;

	default:
		break;
	}
}

#ifdef _WIN32
int isblank(int c)
{
	return ((c == 0x20) || (c == '\t'));
}
#endif

static void fill_sRawName(unsigned char *sRawName, const PBNAMESTRU *pb_name_stru)
{
	int len = MAX_VCF_NAME_LEN + 4;/*4 means ";"*/
	char *pb_raw_name = (char *)NEW(len);

	char *first = pb_name_stru->firstName;
	char *middle = pb_name_stru->middleName;
	char *last = pb_name_stru->lastName;
	char *prefix = pb_name_stru->prefix;
	char *suffix = pb_name_stru->suffix;

	memset(sRawName, 0, sizeof(sRawName));

	snprintf(pb_raw_name, len, "%s;%s;%s;%s;%s", last, first, middle, prefix, suffix);

	utf8_strncpy(sRawName, pb_raw_name, len);
	FREE(pb_raw_name);

}

static void merge_name(unsigned char *sName, const PBNAMESTRU *pb_name_stru)
{
	UINT8 *pb_name = 0;
	int len = MAX_VCF_NAME_LEN * 3 + 1; //lastName + firstName + middleName
	int i = 0;

	if (sName && pb_name_stru) {
		pb_name = (UINT8 *) NEW(len);

		if (pb_name) {
			UINT8 *first = pb_name_stru->firstName;
			UINT8 *middle = pb_name_stru->middleName;
			UINT8 *last = pb_name_stru->lastName;
			memset(pb_name, 0, len);
			for (i=0; i<MAX_VCF_NAME_LEN; i++) {
				if (((isalpha(*(first+i))>0)||(isblank(*(first+i))>0)||(*(first+i)=='\0')) &&
				    ((isalpha(*(middle+i))>0)||(isblank(*(middle+i))>0)||(*(middle+i)=='\0')) &&
				    ((isalpha(*(last+i))>0)||(isblank(*(last+i))>0)||(*(last+i)=='\0'))) {
				} else {
					snprintf(pb_name, len, "%s%s%s", last, middle, first);
					goto out;
				}
			}
			snprintf(pb_name, len, "%s%s%s", first, middle, last);
out:
			len = strlen(pb_name);
			len = len > (MAX_VCF_NAME_LEN - 1) ? (MAX_VCF_NAME_LEN - 1) : len;
			utf8_strncpy(sName, pb_name, len);
			FREE(pb_name);
			pb_name = NULL;
		}
	}
}

static UINT8 hlp_vcfparser_check_item_name(vcf_parser_t *p_parser, const UINT8 *sItemName)
{
	unsigned int nlen = 0;
	unsigned int len = 0;

	if ( !p_parser->p_item_name_end) {
		return 0;
	}

	nlen = p_parser->p_item_name_end ;
	len = strlen(sItemName);

	if (strcmp(sItemName, "TEL") == 0) {
		return ((nlen >= len) && (memcmp(&(p_parser->content.name[p_parser->p_item_name_end- len]), sItemName, len) == 0));
	} else {
		return ((nlen == len) && (memcmp(p_parser->content.name, sItemName, len) == 0));
	}
}

static char hlp_vcfparser_check_encoding(vcf_parser_t *p_parser, const UINT *sEncoding)
{
	return (p_parser->p_item_prop_end && vcf_strstr(p_parser->content.prop, &(p_parser->content.prop[p_parser->p_item_prop_end -1]), sEncoding));
}

#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
static UINT8 hlp_vcfparser_check_number_type(UINT8 *type)
{
	UINT8 numberType = 0;

	if (type == NULL)
		return NUMBER_TYPE_VOICE;
	if(strcmp(type, "HOME;FAX") == 0 ){
		numberType = NUMBER_TYPE_HOME_FAX;
	} else if (strcmp(type, "HOME") == 0) {
		numberType = NUMBER_TYPE_HOME;
	} else if (strcmp(type, "MSG") == 0) {
		numberType = NUMBER_TYPE_MSG;
	} else if (strcmp(type, "WORK;FAX") == 0) {
		numberType = NUMBER_TYPE_WORK_FAX;
	} else if (strcmp(type, "WORK") == 0) {
		numberType = NUMBER_TYPE_WORK;
	} else if (strcmp(type, "VOICE") == 0) {
		numberType = NUMBER_TYPE_VOICE;
	} else if (strcmp(type, "FAX") == 0) {
		numberType = NUMBER_TYPE_FAX;
	} else if (strcmp(type, "CELL") == 0) {
		numberType = NUMBER_TYPE_CELL;
	} else if (strcmp(type, "VIDEO") == 0) {
		numberType = NUMBER_TYPE_VIDEO;
	} else if (strcmp(type, "PAGER") == 0) {
		numberType = NUMBER_TYPE_PAGER;
	} else if (strcmp(type, "BBS") == 0) {
		numberType = NUMBER_TYPE_BBS;
	} else if (strcmp(type, "MODEM") == 0) {
		numberType = NUMBER_TYPE_MODEM;
	} else if (strcmp(type, "CAR") == 0) {
		numberType = NUMBER_TYPE_CAR;
	} else if (strcmp(type, "ISDN") == 0) {
		numberType = NUMBER_TYPE_ISDN;
	} else if (strcmp(type, "PCS") == 0) {
		numberType = NUMBER_TYPE_PCS;
	} else if (strcmp(type, "PREF") == 0) {
		numberType = NUMBER_TYPE_PREF;
	} else {
		numberType = NUMBER_TYPE_VOICE;
	}

	//LOG("NumberType = [%s]", type);
	return numberType;
}
#endif

static void hlp_vcfparser_clean_cache(vcf_parser_t *p_parser)
{
	if (p_parser == NULL)
		return;

	//p_parser->p_item_name_start = 0;
	p_parser->p_item_name_end = 0;
	//p_parser->p_item_prop_start = 0;
	p_parser->p_item_prop_end = 0;
	//p_parser->p_item_val_start = 0;
	p_parser->p_item_val_end = 0;
	p_parser->b_continue = FALSE;
	p_parser->is_encoding_QP = 0;
	p_parser->status = PARSE_IDLE;
	memset(&p_parser->content,0,sizeof(vcf_content_t));
}

#ifdef CONFIG_PBAP_RAWDATA
static void cp_raw_data(const vcf_parser_t *p_parser)
{
	int i = 0;
	int len = 0;
	int buf_len = 0;
	UINT8* s = NULL;
	s = p_parser->p_item_name_start;
	buf_len = strlen(p_parser->p_item_name_start);
	if(hlp_vcfparser_check_item_name(p_parser, "PHOTO")) {
		LOG("photo data , ignore!!!");
		len = (MAX_BUF_LEN - s_buf_offset) > 9 ? 9 : MAX_BUF_LEN - s_buf_offset;
		memcpy(s_raw_data_buf + s_buf_offset, "END:VCARD", len);
		s_buf_offset += len;

		UINT8 count_str[5];
		sprintf(count_str, "%04x", s_buf_offset - 9);
		memcpy(s_raw_data_buf + 5, count_str, 4);

		s_number_count = 0;
		s_buf_offset = 0;
		s_parser_start = FALSE;
		LOG(" photo data rawdata cbk index is %d", p_parser->p_pb_item->pb_index);
		p_parser->item_callback(EV_PBAP_PHONEBOOK_RAWDATA_IND, s_raw_data_buf);
		return ;
	}
	for(i = 0; i < buf_len; i++) {
		if(*(s + i) == 0x3E) {
			//TODO The data is insufficient
			return ;
		}
		if(*(s + i) == '\n' && *(s + i - 1) == '\r' ) { //&& *(s + i - 2) != )
			len = i + 1 > 1023 - s_buf_offset? 1023 - s_buf_offset : i + 1;
			//FIXME:This is not reasonable
			if((strncmp(s, "TEL", 3) == 0)) {
				if(s_number_count > 10) {
					break;
				}
				s_number_count++;
			}
			//END
			memcpy(s_raw_data_buf + s_buf_offset, s, len);
			s_buf_offset += len;

			if(strstr(s_raw_data_buf, "END")) {
				UINT8 count_str[5];
				sprintf(count_str, "%04x", s_buf_offset - 9);
				memcpy(s_raw_data_buf + 5, count_str, 4);
				s_number_count = 0;
				s_buf_offset = 0;
				s_parser_start = FALSE;
				p_parser->item_callback(EV_PBAP_PHONEBOOK_RAWDATA_IND, s_raw_data_buf);
			}
			return;
		}

	}


}
#endif

//static char dpb_buf[MAX_VCF_ITEM_LEN] = {0};
static char number_buff[256] = {0};
static vcf_content_buff st_pb_buff;
					//static char pb_buf[MAX_VCF_ITEM_LEN] = {0};
static int dpb_number = 0;
static int p_index = 0;
static int pb_flag = 0;

void hlp_pb_buff_init()
{
	for(int i = 0; i < PB_BUFF_NUMBER_MAX; i++)
	{
		st_pb_buff.numberType[i] = 0;
		memset(st_pb_buff.sNumber[i], 0, MAX_VCF_NUMBER_LEN);
	}
	memset(st_pb_buff.sName, 0, MAX_VCF_NAME_LEN);
	st_pb_buff.pb_index = 0; 
}

int hlp_event_last_day_pbap_packet(int pbap_c_flag)
{
	int ret = 0;
	//if(dpb_number > 0 && pb_flag)
		//event_pbap_indication(EV_PBAP_PHONEBOOK_DATA_IND, (void *)dpb_buf);
	LOG("dpb_number = %d pb_flag = %d pbap_c_flag = %d\n", dpb_number, pb_flag, pbap_c_flag);
	char dpb_buf_ex[MAX_VCF_ITEM_LEN] = {0};
	/*if(dpb_number > 0 && pb_flag)
	{
		snprintf(dpb_buf_ex, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d%c%d",1, 0xff, st_pb_buff.sName, 0xff, st_pb_buff.sNumber, 0xff, st_pb_buff.numberType[0], 0xff, st_pb_buff.pb_index);
		event_pbap_indication(EV_PBAP_PHONEBOOK_DATA_IND, (void *)dpb_buf_ex);
	}*/

	if(dpb_number > 0 && pb_flag)
	{
		pb_flag = 0;
		if(pbap_c_flag){
			char number_buff[256] = {0};
			//int number_index = 0;
            int pd_i = 0;
			for( pd_i = 0; pd_i < dpb_number; pd_i++)
			{
				//strcat(number_buff, st_pb_buff.sNumber[pd_i]);
				//strcat(number_buff, ',');
				sprintf(number_buff, "%s%s,", number_buff, st_pb_buff.sNumber[pd_i]);
			}
			number_buff[strlen(number_buff)-1] = 0xff;
			
			for(pd_i = 0; pd_i<dpb_number; pd_i++)
			{
				//strcat(number_buff, st_pb_buff.numberType[pd_i]+32, 1);
				//strcat(number_buff, ',' , 1);
				sprintf(number_buff, "%s%d,", number_buff, st_pb_buff.numberType[pd_i]);
			}
			number_buff[strlen(number_buff)-1] = 0;

			snprintf(dpb_buf_ex, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d",1, 0xff, st_pb_buff.sName, 0xff, number_buff, 0xff, st_pb_buff.pb_index);
			event_pbap_indication(EV_PBAP_PHONEBOOK_DATA_IND, (void *)dpb_buf_ex);
			void hlp_pb_buff_init();
			dpb_number = 0;
		}
	}

	return ret;
}

static PBNAMESTRU *PB_name_stru = NULL;
void app_vcfparser_create()
{
	PB_name_stru = (PBNAMESTRU *)NEW(sizeof(PBNAMESTRU));
}

static void hlp_vcfparser_process(vcf_parser_t *p_parser)
{
	static int tel_flag = 1;
	if (p_parser == NULL)
		return;
#ifdef CONFIG_PBAP_RAWDATA
	if(s_parser_start) {
		cp_raw_data(p_parser);
	}
#endif //CONFIG_PBAP_RAWDATA
	if (hlp_vcfparser_check_item_name(p_parser, "BEGIN")) {
		if (p_parser->p_item_val_end != 0) {
			UINT8 item_value[16] = {0};
			hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));
#ifdef CONFIG_PBAP_RAWDATA
			s_buf_offset = 0;
			s_number_count = 0;
			memset(s_raw_data_buf, 0, MAX_BUF_LEN);
			sprintf(s_raw_data_buf, "%d%04x%04x", p_parser->store_type, p_parser->p_pb_item->pb_index+1, 0);
			s_buf_offset += 9;
			s_parser_start = TRUE;

			cp_raw_data(p_parser);
#endif //CONFIG_PBAP_RAWDATA
			if (strncmp(item_value, "VCARD", strlen("VCARD")) == 0) {
				VCF_PARSE_BEGIN_TYPE(p_parser->item_type, VCF_PARSE_TYPE_VCARD);
				p_parser->p_pb_item->pb_index++;
                //XXX added by fcx
		        memset(p_parser->p_pb_item->sRawName, 0, MAX_VCF_NAME_LEN + 4);//4 is for ';'
			} else if (strncmp(item_value, "BMSG", strlen("BMSG")) == 0) {
				VCF_PARSE_BEGIN_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG);
			} else if (strncmp(item_value, "BENV", strlen("BENV")) == 0) {
				VCF_PARSE_BEGIN_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BENV);
			} else if (strncmp(item_value, "BBODY", strlen("BBODY")) == 0) {
				VCF_PARSE_BEGIN_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BBODY);
			} else if (strncmp(item_value, "MSG", strlen("MSG")) == 0) {
				VCF_PARSE_BEGIN_TYPE(p_parser->item_type, VCF_PARSE_TYPE_MSG);

				if (p_parser->p_pb_item->sMsg != NULL) {
					// FIXME: 'p_msg + sMsg_length' may be out of the actual bound of the Message content provided by stack.
					UINT8 *p_msg = p_parser->content.val+ strlen("MSG\r\n");
					snprintf(p_parser->p_pb_item->sMsg, p_parser->p_pb_item->sMsg_length, "%s", p_msg);
					//hlp_vcf_get_item_value(p_msg, p_parser->p_pb_item->sMsg, p_parser->p_pb_item->sMsg_length);
					if (VCF_PARSE_DEBUG)
						LOG("msg len = %d, %s", strlen(p_parser->p_pb_item->sMsg), p_parser->p_pb_item->sMsg);
				}
			}
		}
	}

	if (hlp_vcfparser_check_item_name(p_parser, "FN") && !(VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BENV))) {
		UINT8 pb_name[MAX_VCF_NAME_LEN];
		UINT count = 0;
		UINT pos = 0;
		//PBNAMESTRU *pb_name_stru = 0;
		//pb_name_stru = (PBNAMESTRU *)NEW(sizeof(PBNAMESTRU));
		if(PB_name_stru != NULL)
		{
			memset(PB_name_stru, 0, sizeof(PBNAMESTRU));
			memset(pb_name, 0, MAX_VCF_NAME_LEN);
			memset(p_parser->p_pb_item->sName, 0, MAX_VCF_NAME_LEN);
			memset(p_parser->p_pb_item->sRawName, 0, MAX_VCF_NAME_LEN + 4);//4 is for ';'

			if (hlp_vcfparser_check_encoding(p_parser, "QUOTED-PRINTABLE")) {
				if (p_parser->p_item_name_end) {
					unsigned short  i = 0, j = 0;

					while (i < p_parser->p_item_val_end) {
						if (p_parser->content.val[i] == '=') {
							if (i + 2 < p_parser->p_item_val_end) {
								if ((p_parser->content.val[i + 1]=='\r') && (p_parser->content.val[i + 2]=='\n')) {
									i += 3;
								} else {
									char sChar[3] = {0};
									char *p = 0;
									sChar[0] = p_parser->content.val[i + 1];
									sChar[1] = p_parser->content.val[i + 2];
									i += 3;

									pb_name[j++] = strtol(sChar, &p, 16);//strtohex(sChar);
									if (j == MAX_VCF_NAME_LEN - 1) {
										break;
									}
								}
							} else {
								break;
							}
						} else if (p_parser->content.val[i] == ';') {
							dev_name(PB_name_stru, pb_name + pos, count);
							i++;
							count++;
							pos = j;
						} else {
							pb_name[j++] = p_parser->content.val[i++];

							if (j == MAX_VCF_NAME_LEN - 1) {
								break;
							}
						}
					}

					dev_name(PB_name_stru, pb_name + pos, count);
					merge_name(p_parser->p_pb_item->sName, PB_name_stru);
					fill_sRawName(p_parser->p_pb_item->sRawName, PB_name_stru);
					LOG("s_pPBItem->sName is %s, pb_name is %s.", p_parser->p_pb_item->sName, pb_name);
				}
			} else {
				if (p_parser->p_item_val_end) {
					unsigned short i = 0, j = 0;

					while (i < p_parser->p_item_val_end) {
						if (j == MAX_VCF_NAME_LEN - 1) {
							break;
						}

						if (p_parser->content.val[i] != ';') {
							pb_name[j++] = p_parser->content.val[i];
						} else {

							if(pb_name[j-1] == '\\') {
								pb_name[j-1] = ';';
								LOG("The current character is escaped");
							}
							
							dev_name(PB_name_stru, pb_name + pos, count);
							count++;
							pos = j;
						}

						i++;
					}

					dev_name(PB_name_stru, pb_name + pos, count);
					merge_name(p_parser->p_pb_item->sName, PB_name_stru);
					fill_sRawName(p_parser->p_pb_item->sRawName, PB_name_stru);
					LOG("s_pPBItem->sName is %s, pb_name is %s.", p_parser->p_pb_item->sName, pb_name);
				}
			}
		}
	} else if (hlp_vcfparser_check_item_name(p_parser, "TEL") && !(VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BENV))) {
		tel_flag = 0;
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
		p_parser->p_pb_item->numberType = NUMBER_TYPE_VOICE;//NUMBER_TYPE_HOME;
		LOG("_parser->p_item_prop_end 1 = %d\n", p_parser->p_item_prop_end);
		if (p_parser->p_item_prop_end) {
			int i = 0;
			char buff[16] = {0};
			char *p = vcf_strstr(p_parser->content.prop, &(p_parser->content.prop[p_parser->p_item_prop_end-1]), "TYPE=");
			if (p != NULL) {
				p += strlen("TYPE=");
			} else {
				p = p_parser->content.prop;
				if (!(*p >= 'A' && *p <= 'Z')) {
					p = NULL;
				}
			}

			if (p != NULL) {
				while (1) {
					if ((*(p + i) != ';') &&( (*(p + i) < 'A') || (*(p + i) > 'Z') ))
						break;

					buff[i] = *(p + i);
					i++;
				}

				p_parser->p_pb_item->numberType = hlp_vcfparser_check_number_type(buff);
			}
		}

#endif
		LOG("_parser->p_item_val_end 2 = %d\n", p_parser->p_item_val_end);
		if (p_parser->p_item_val_end || strlen(p_parser->p_pb_item->sName) > 0) {
			char sNumber[MAX_VCF_NUMBER_LEN] = {0};
			unsigned char i = 0, j = 0;
			memset(p_parser->p_pb_item->sNumber, 0, MAX_VCF_NUMBER_LEN);

			if (p_parser->p_item_val_end && p_parser->p_item_val_end  >= MAX_VCF_NUMBER_LEN) {
				memcpy(p_parser->p_pb_item->sNumber, p_parser->content.val, MAX_VCF_NUMBER_LEN-1);
			} else {
				memcpy(p_parser->p_pb_item->sNumber, p_parser->content.val, p_parser->p_item_val_end);
			}
			if (VCF_PARSE_DEBUG)
				LOG("number = %s", p_parser->p_pb_item->sNumber);
			//Do not care about no number
			for (i = 0; i < strlen(p_parser->p_pb_item->sNumber) ; i++) {
				if (((p_parser->p_pb_item->sNumber[i] >= '0') && (p_parser->p_pb_item->sNumber[i] <= '9'))
					|| (p_parser->p_pb_item->sNumber[i] == '+')
					|| (p_parser->p_pb_item->sNumber[i] == '*')
					|| (p_parser->p_pb_item->sNumber[i] == '#')) {
						sNumber[j++] = p_parser->p_pb_item->sNumber[i];
				}
			}

			sNumber[j] = 0;
			strcpy(p_parser->p_pb_item->sNumber, sNumber);

			if (!VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG)) {
				LOG("%s", "PB JX OK");
				memset(p_parser->p_pb_item->sLine, 0, MAX_VCF_ITEM_LEN);

				if (p_parser->store_type < 2) { //PhoneBook
				LOG("%s", "PB JX OK 222");
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE

					if (strlen(p_parser->p_pb_item->sName) > 0x40)
						LOG("%s", "PB Error");
					
					//snprintf(pb_buf, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d", 1, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
					if(p_parser->p_pb_item->pb_index == 1  && (!dpb_number))//init
					{
						//memset(&st_pb_buff, 0, sizeof(vcf_content_buff));
						hlp_pb_buff_init();
						dpb_number = 0;
					}
					if(dpb_number == 0)
					{
						//snprintf(dpb_buf, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d", 1, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
						memset(st_pb_buff.sName, 0, MAX_VCF_NAME_LEN);
						memcpy(st_pb_buff.sName, p_parser->p_pb_item->sName, strlen(p_parser->p_pb_item->sName));
						memcpy(st_pb_buff.sNumber[dpb_number], p_parser->p_pb_item->sNumber, strlen(p_parser->p_pb_item->sNumber));
						st_pb_buff.numberType[dpb_number] = p_parser->p_pb_item->numberType;
						st_pb_buff.pb_index = p_parser->p_pb_item->pb_index;
						pb_flag = 1;
						dpb_number++;
						
					} 
					else if(p_index == p_parser->p_pb_item->pb_index)
					{
						//int dpb_len = strlen(dpb_buf);
						//snprintf(&dpb_buf[dpb_len], MAX_VCF_ITEM_LEN - dpb_len, "%c%d%c%s",0xff ,p_parser->p_pb_item->numberType ,0xff , p_parser->p_pb_item->sNumber);
						if(dpb_number < PB_BUFF_NUMBER_MAX) {
							memcpy(st_pb_buff.sNumber[dpb_number], p_parser->p_pb_item->sNumber, strlen(p_parser->p_pb_item->sNumber));
							st_pb_buff.numberType[dpb_number] = p_parser->p_pb_item->numberType;
							dpb_number++;
						}
					}
					else
					{
						//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%s",dpb_buf);
						//memset(dpb_buf, 0, MAX_VCF_ITEM_LEN);
						//char number_buff[256] = {0};
						memset(number_buff, 0, 256);
						//int number_index = 0;
						int pd_i = 0;
						for(pd_i = 0; pd_i < dpb_number; pd_i++)
						{
							//strcat(number_buff, st_pb_buff.sNumber[pd_i]);
							//strcat(number_buff, ',');
							sprintf(number_buff, "%s%s,", number_buff, st_pb_buff.sNumber[pd_i]);
						}
						number_buff[strlen(number_buff)-1] = 0xff;

						for(pd_i = 0; pd_i<dpb_number; pd_i++)
						{
							//strcat(number_buff, st_pb_buff.numberType[pd_i]+32, 1);
							//strcat(number_buff, ',' , 1);
							sprintf(number_buff, "%s%d,", number_buff, st_pb_buff.numberType[pd_i]);
						}
						number_buff[strlen(number_buff)-1] = 0;

						snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d",1, 0xff, st_pb_buff.sName, 0xff, number_buff, 0xff, st_pb_buff.pb_index);

						//memset(&st_pb_buff, 0, sizeof(vcf_content_buff));
						hlp_pb_buff_init();
						dpb_number = 0;

						//snprintf(dpb_buf, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d", 1, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
						memset(number_buff, 0, 256);
						memcpy(number_buff, p_parser->p_pb_item->sNumber, strlen(p_parser->p_pb_item->sNumber));

						memset(st_pb_buff.sName, 0, MAX_VCF_NAME_LEN);
						memcpy(st_pb_buff.sName, p_parser->p_pb_item->sName, strlen(p_parser->p_pb_item->sName));
						memcpy(st_pb_buff.sNumber[dpb_number], p_parser->p_pb_item->sNumber, strlen(p_parser->p_pb_item->sNumber));
						st_pb_buff.numberType[dpb_number] = p_parser->p_pb_item->numberType;
						st_pb_buff.pb_index = p_parser->p_pb_item->pb_index;
						dpb_number++;
						pb_flag = 1;
						p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
					}
					p_index = p_parser->p_pb_item->pb_index;

					//printf("p_parser->p_pb_item->sName = %s\n", p_parser->p_pb_item->sName);
					//printf("p_parser->p_pb_item->sNumber = %s\n", p_parser->p_pb_item->sNumber);
					//printf("p_parser->p_pb_item->sTime = %s\n", p_parser->p_pb_item->sTime);
					//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%s%c%s", 1, 0xff, p_parser->p_pb_item->sName, 0xff, p_parser->p_pb_item->sNumber, 0xff, p_parser->p_pb_item->sTime);
					//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%d%c%s%c%d%c%s%c%s",p_parser->p_pb_item->pb_index, 0xff, 1, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType ,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime);
					////snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d", 1, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
					//printf("p_parser->p_pb_item->sLine = %s\n", p_parser->p_pb_item->sLine);
					//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%s%c%s", p_parser->p_pb_item->sName,0xff, p_parser->p_pb_item->sNumber);
#else
					snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%02d%02d%s%s", p_parser->store_type, strlen(p_parser->p_pb_item->sName), strlen(p_parser->p_pb_item->sNumber), p_parser->p_pb_item->sName, p_parser->p_pb_item->sNumber);
#endif
					////p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);

					//memset(p_parser->p_pb_item->sNumber,0,MAX_VCF_NUMBER_LEN);
				} else if (p_parser->store_type == 5 || p_parser->store_type == 6) {
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
					//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%04X%X%04x%02d%s%s", p_parser->store_type, p_parser->p_pb_item->pb_index, p_parser->p_pb_item->numberType, strlen(p_parser->p_pb_item->sRawName), strlen(p_parser->p_pb_item->sNumber), p_parser->p_pb_item->sRawName, p_parser->p_pb_item->sNumber);
					snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%s%c%s", p_parser->p_pb_item->sName, 0xff, p_parser->p_pb_item->sNumber);
#else
					//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%04X%04x%02d%s%s", p_parser->store_type, p_parser->p_pb_item->pb_index, strlen(p_parser->p_pb_item->sRawName), strlen(p_parser->p_pb_item->sNumber), p_parser->p_pb_item->sRawName, p_parser->p_pb_item->sNumber);
#endif
					p_parser->item_callback(EV_PBAP_INDEX_DATA_IND, (void *)p_parser->p_pb_item->sLine);
					//memset(p_parser->p_pb_item->sNumber,0,MAX_VCF_NUMBER_LEN);
				}
			}
		}
		p_parser->p_pb_item->numberType = 0;
	} else if (hlp_vcfparser_check_item_name(p_parser, "X-IRMC-CALL-DATETIME")) {
		if (p_parser->p_item_prop_end) {
			if (strncmp(p_parser->content.prop, "DIALED", 6) == 0) {
				p_parser->p_pb_item->call_log_type = PBAP_VCF_CALLLOG_DIALED;
			} else if (strncmp(p_parser->content.prop, "MISSED", 6) == 0) {
				p_parser->p_pb_item->call_log_type = PBAP_VCF_CALLLOG_MISSED;
			} else if (strncmp(p_parser->content.prop, "RECEIVED", 8) == 0) {
				p_parser->p_pb_item->call_log_type = PBAP_VCF_CALLLOG_RECEIVED;
			}
		}

		if (p_parser->p_item_val_end) {
			memset(p_parser->p_pb_item->sTime, 0, MAX_VCF_TIME_LEN);

			if (p_parser->p_item_val_end >= MAX_VCF_TIME_LEN) {
				memcpy(p_parser->p_pb_item->sTime, p_parser->content.val, MAX_VCF_TIME_LEN - 1);
			} else {
				memcpy(p_parser->p_pb_item->sTime, p_parser->content.val, p_parser->p_item_val_end);
			}
		}
	} else if (hlp_vcfparser_check_item_name(p_parser, "CHARSET")) {
		UINT8 item_value[16];
		hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));

		if (strcmp(item_value, "UTF-8") != 0) {
			LOG("The message format is error, {Format=[%s]}\r\n", item_value);
		}
	} else if (hlp_vcfparser_check_item_name(p_parser, "LENGTH")) {
		UINT8 item_value[16];
		UINT32 length = 0;
		//UINT32 msg_length = 0;
		hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));
		length = atoi(item_value)+1;//FIXME It's a invaild value for xiaomi phone.
		if (VCF_PARSE_DEBUG)
			LOG("length = %d", length);
		p_parser->p_pb_item->sMsg_length = length < MAX_VCF_MSG_LEN ? length : MAX_VCF_MSG_LEN;

		if (p_parser->p_pb_item->sMsg) {
			FREE(p_parser->p_pb_item->sMsg);
		}

		p_parser->p_pb_item->sMsg = NEW(p_parser->p_pb_item->sMsg_length + 1);
		memset(p_parser->p_pb_item->sMsg, 0, p_parser->p_pb_item->sMsg_length + 1);
	} else if (hlp_vcfparser_check_item_name(p_parser, "STATUS") && VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG)) {
		UINT8 item_value[32] = {0};
		hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));
		if (VCF_PARSE_DEBUG)
			LOG("status = %s", item_value);
		if (vcf_strstr(item_value, item_value+32, "UNREAD")) {
			p_parser->p_pb_item->msg_status = 0;//unread
		} else {
			p_parser->p_pb_item->msg_status = 1;//read
		}
	} else if (hlp_vcfparser_check_item_name(p_parser, "TYPE") && VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG)) {
		UINT8 item_value[32] = {0};
		hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));
		if (VCF_PARSE_DEBUG)
			LOG("TYPE = %s", item_value);
		strcpy(p_parser->p_pb_item->msg_type, item_value);
	} else if (hlp_vcfparser_check_item_name(p_parser, "FOLDER") && VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG)) {
		UINT8 item_value[32] = {0};
		hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));
		if (VCF_PARSE_DEBUG)
			LOG("FOLDER = %s", item_value);
		strcpy(p_parser->p_pb_item->msg_folder, item_value);
	} else if (hlp_vcfparser_check_item_name(p_parser, "END")) {
		if (p_parser->p_item_val_end) {
			UINT8 item_value[16];
			hlp_vcf_get_item_value(p_parser->content.val, item_value, sizeof(item_value));

			if (strcmp(item_value, "VCARD") == 0) {
				if (!(VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG))) {
					if ((p_parser->store_type >= 2) && (p_parser->store_type <= 4)) { //Call Log
						memset(p_parser->p_pb_item->sLine, 0, MAX_VCF_ITEM_LEN);

						if (p_parser->p_pb_item->call_log_type == 0x00) {
                            LOG("fixme call_log_type_unknown, but not sync cch.");
							p_parser->p_pb_item->call_log_type = p_parser->store_type;
						}
						//printf("p_parser->p_pb_item->sName = %s\n", p_parser->p_pb_item->sName);
						//printf("p_parser->p_pb_item->sNumber = %s\n", p_parser->p_pb_item->sNumber);
						//printf("p_parser->p_pb_item->sTime = %s\n", p_parser->p_pb_item->sTime);
						//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%d%c%s%c%d%c%s%c%s",p_parser->p_pb_item->pb_index, 0xff,p_parser->store_type, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime);
						snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d",p_parser->store_type, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);

						p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
						//printf("p_parser->p_pb_item->sLine = %s\n", p_parser->p_pb_item->sLine);
						//p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, p_parser->p_pb_item->sLine);
					} else if ((p_parser->store_type >= 5) && (p_parser->store_type <= 6)) {
						if (p_parser->p_pb_item->sNumber[0] == '\0') {
#ifdef PBAP_VCF_PARSE_CARE_TEL_TYPE
							//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%d%04X%X%02d%02d%s%s", p_parser->store_type, p_parser->p_pb_item->pb_index, p_parser->p_pb_item->numberType, strlen(p_parser->p_pb_item->sName), strlen(p_parser->p_pb_item->sNumber), p_parser->p_pb_item->sName, p_parser->p_pb_item->sNumber);
							snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%d%02X%s%02X%s", p_parser->store_type, 0xFF, p_parser->p_pb_item->sName, 0xFF, p_parser->p_pb_item->sNumber);
#else
							snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%04X%02d%02d%s%s", p_parser->store_type, p_parser->p_pb_item->pb_index, strlen(p_parser->p_pb_item->sName), strlen(p_parser->p_pb_item->sNumber), p_parser->p_pb_item->sName, p_parser->p_pb_item->sNumber);
#endif
							p_parser->item_callback(EV_PBAP_INDEX_DATA_IND, (void *)p_parser->p_pb_item->sLine);
						}
					} else if (p_parser->store_type == 7) {
						unsigned int callog_type = PBAP_VCF_CALLLOG_UNKOWN;
						memset(p_parser->p_pb_item->sLine, 0, MAX_VCF_ITEM_LEN);

						if (p_parser->p_pb_item->call_log_type == 0x00) {
							callog_type = PBAP_VCF_CALLLOG_UNKOWN;
						} else if (p_parser->p_pb_item->call_log_type == PBAP_VCF_CALLLOG_DIALED) {
							callog_type = PT_PBAP_OCH;
						} else if (p_parser->p_pb_item->call_log_type == PBAP_VCF_CALLLOG_MISSED) {
							callog_type = PT_PBAP_MCH;
						} else if (p_parser->p_pb_item->call_log_type == PBAP_VCF_CALLLOG_RECEIVED) {
							callog_type = PT_PBAP_ICH;
						}
						//printf("p_parser->p_pb_item->sName = %s\n", p_parser->p_pb_item->sName);
						//printf("p_parser->p_pb_item->sNumber = %s\n", p_parser->p_pb_item->sNumber);
						//printf("p_parser->p_pb_item->sTime = %s\n", p_parser->p_pb_item->sTime);
						//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%s%c%s", callog_type, 0xff, p_parser->p_pb_item->sName, 0xff, p_parser->p_pb_item->sNumber, 0xff, p_parser->p_pb_item->sTime);
						//snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%d%c%s%c%s%c%d",p_parser->store_type, 0xff, p_parser->p_pb_item->sName,0xff ,p_parser->p_pb_item->numberType,0xff , p_parser->p_pb_item->sNumber,0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
						snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%s%c%s%c%d", callog_type, 0xff, p_parser->p_pb_item->sName, 0xff, p_parser->p_pb_item->sNumber, 0xff, p_parser->p_pb_item->sTime, 0xff, p_parser->p_pb_item->pb_index);
						p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
						//printf("p_parser->p_pb_item->sLine = %s\n", p_parser->p_pb_item->sLine);
					} else if(p_parser->store_type == 1 && tel_flag) {
						LOG("app_vcfparse_start, TEL NULL tel_flag: %d p_parser->p_pb_item->sNumber = %s p_parser->p_pb_item->pb_index = %d st_pb_buff.pb_index = %d", tel_flag, p_parser->p_pb_item->sNumber, p_parser->p_pb_item->pb_index, st_pb_buff.pb_index);
						if(p_parser->p_pb_item->pb_index != 1 && st_pb_buff.pb_index != 0)
						{
							if(dpb_number > 1){
								snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d",1, 0xff, st_pb_buff.sName, 0xff, number_buff, 0xff, st_pb_buff.pb_index);
								p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
							} else {
								snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d%c%d",1, 0xff, st_pb_buff.sName, 0xff, number_buff, 0xff, st_pb_buff.numberType[0], 0xff, st_pb_buff.pb_index);
								p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
							}
							hlp_pb_buff_init();
							dpb_number = 0;
							memset(p_parser->p_pb_item->sLine, 0, MAX_VCF_ITEM_LEN);
						}
						
						snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%d%c%s%c%s%c%c%d", 1, 0xff, p_parser->p_pb_item->sName, 0xff, p_parser->p_pb_item->sNumber, 0xff, 0xff, p_parser->p_pb_item->pb_index);
						p_parser->item_callback(EV_PBAP_PHONEBOOK_DATA_IND, (void *)p_parser->p_pb_item->sLine);
					
					}

					memset(p_parser->p_pb_item->sName, 0, MAX_VCF_NAME_LEN);
					memset(p_parser->p_pb_item->sNumber, 0, MAX_VCF_NUMBER_LEN);
					memset(p_parser->p_pb_item->sTime, 0, MAX_VCF_TIME_LEN);
					p_parser->p_pb_item->call_log_type = 0;
				}

				tel_flag = 1;
				VCF_PARSE_END_TYPE(p_parser->item_type, VCF_PARSE_TYPE_VCARD);
			} else if ((strncmp(item_value, "BMSG", 4) == 0) && (VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG))) {
				if (p_parser->p_pb_item->sMsg && p_parser->item_callback) {
					p_parser->item_callback(EV_MAP_BMSG_DATA_IND, p_parser->p_pb_item);
#if 0
					snprintf(p_parser->p_pb_item->sLine, MAX_VCF_ITEM_LEN, "%s|%s|%s|%s",
						p_parser->p_pb_item->msg_handle, p_parser->p_pb_item->sName, p_parser->p_pb_item->sNumber, p_parser->p_pb_item->sMsg);
#ifdef CONFIG_MULTI_CONNECTION
					p_parser->item_callback(EV_MAP_BMSG_DATA_IND,p_parser->bd_addr, p_parser->p_pb_item->sLine);
#else
					p_parser->item_callback(EV_MAP_BMSG_DATA_IND, p_parser->p_pb_item->sLine);
#endif
#endif
					FREE(p_parser->p_pb_item->sMsg);
					p_parser->p_pb_item->sMsg = NULL;
				}

				p_parser->p_pb_item->sMsg_length = 0;
				p_parser->p_pb_item->sMsg = NULL;
				VCF_PARSE_END_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BMSG);
			} else if ((strcmp(item_value, "BENV") == 0) && (VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BENV))) {
				VCF_PARSE_END_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BENV);
			} else if ((strcmp(item_value, "BBODY") == 0) && (VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BBODY))) {
				VCF_PARSE_END_TYPE(p_parser->item_type, VCF_PARSE_TYPE_BBODY);
			} else if ((strcmp(item_value, "MSG") == 0) && (VCF_PARSE_CHECK_TYPE(p_parser->item_type, VCF_PARSE_TYPE_MSG))) {				
				VCF_PARSE_END_TYPE(p_parser->item_type, VCF_PARSE_TYPE_MSG);
			}
		}
	}
}


void app_vcfparser_msg_handle_ind(vcf_parser_t *p_parser, UINT8 *msg_handle)
{
	if(p_parser) {
		if ((p_parser->p_pb_item != NULL) && (msg_handle != NULL)) {
			memcpy(p_parser->p_pb_item->msg_handle, msg_handle, strlen(msg_handle) + 1);
		}
	}
}

vcf_parser_t *app_vcfparser_alloc(UINT8 store_type, UINT8 *p_parser_name)
{
	vcf_parser_t *p_parser = (vcf_parser_t *)NEW(sizeof(vcf_parser_t));
	LOG("app_vcfparse_start, parser name: %s", p_parser_name);

	if (p_parser != NULL) {
		memset(p_parser, 0, sizeof(vcf_parser_t));
#ifdef VCF_PARSER_DEBUG_ENABLE
		memcpy(p_parser->parser_name, p_parser_name, strlen(p_parser_name) + 1);
#endif
		p_parser->p_pb_item = (PBITEM *)NEW(sizeof(PBITEM));

		if (p_parser->p_pb_item == NULL) {
			FREE(p_parser);
			return NULL;
		}

		memset(p_parser->p_pb_item, 0, sizeof(PBITEM));
		p_parser->item_type = VCF_PARSE_TYPE_NONE;
		p_parser->store_type = store_type;
		p_parser->status = PARSE_IDLE;
	}

	return p_parser;
}

void app_vcfparser_data_ind(vcf_parser_t *p_parser, UINT8 *p_input, UINT16 data_len, UINT16 offset)
{
	UINT8 *p = 0;
	if (!p_parser) {
		return ;
	}
	p = p_input;
	if  (p != NULL) {
		switch (p_parser->status)
		{
				case PARSE_IDLE:
					{
						if ((*p) == ';') {
							p_parser->status = PARSE_ITEM_NAME_GOT;
						} else if ((*p) == ':') {
							p_parser->status = PARSE_ITEM_PROP_GOT;
						} else if (p_parser->p_item_name_end < VCF_CONTENT_NAME_LEN){
							p_parser->content.name[p_parser->p_item_name_end++] = *p;
						}
					}
					break;

				case PARSE_ITEM_NAME_GOT:
					{
						if ((*p) == ':') {
							p_parser->status = PARSE_ITEM_PROP_GOT;
						}
						else if (((*p) == '\n') && (p_parser->content.prop[p_parser->p_item_prop_end - 1] == '\r')) {/*"\r\n;;;\r\n*/
							hlp_vcfparser_clean_cache(p_parser);
						} else if (p_parser->p_item_prop_end < VCF_CONTENT_PROP_LEN) {
							p_parser->content.prop[p_parser->p_item_prop_end++] = *p;
						}
					}
					break;

				case PARSE_ITEM_PROP_GOT:
					{
						//for same type who's value contains \r\n, such as msg
						UINT8 *tmp = NULL;
						if (*p == '\n')
						{
							if (!p_parser->b_continue) {
								UINT8 res1 = memcmp(p_parser->content.name, "BEGIN", 5);
								UINT8 res2 = memcmp(p_parser->content.val, "MSG", 3);
								if (res1 == 0 && res2 == 0) {
									LOG("b_continue = true");
									p_parser->b_continue = TRUE;
								}
							} else if ((tmp = (unsigned char *)vcf_strstr(p+1, p+1+data_len-offset, "\r\n")) != NULL) {
								UINT8 next_line[MAX_VCF_ITEM_LEN] = {0};
								memcpy(next_line, p+1, tmp-p < MAX_VCF_ITEM_LEN ? tmp-p : MAX_VCF_ITEM_LEN-1);
								if (memcmp(next_line, "END:MSG", 7) == 0) {
									LOG("b_continue = false");
									p_parser->b_continue = FALSE;
								}
							}
						}
						
						if (!p_parser->b_continue && ((*p) == '\n') && (*(p-1) == '\r' || p_parser->content.val[p_parser->p_item_val_end - 1] == '\r')) {
							p_parser->status = PARSE_ITEM_VAL_GOT;
							p_parser->content.val[p_parser->p_item_val_end - 1] = 0;//discard '\r'.
							p_parser->p_item_val_end--;
							if (hlp_vcfparser_check_item_name(p_parser, "END")) {
								hlp_vcfparser_process(p_parser);
								hlp_vcfparser_clean_cache(p_parser);
							}
						} else if (p_parser->p_item_val_end < VCF_CONTENT_VAL_LEN) {
							p_parser->content.val[p_parser->p_item_val_end++] = *p;
						}
					}
					break;

				case PARSE_ITEM_VAL_GOT:
					{
						if ((*p) == '=') {
							p_parser->status = PARSE_ITEM_PROP_GOT;//discard this '='.
						} else {
							hlp_vcfparser_process(p_parser);
							hlp_vcfparser_clean_cache(p_parser);
							if ((*p) == ';') {
								p_parser->status = PARSE_ITEM_NAME_GOT;
							} else if ((*p) == ':') {
								p_parser->status = PARSE_ITEM_PROP_GOT;
							} else {
								p_parser->content.name[p_parser->p_item_name_end++] = *p;
							}
						}
					}
					break;
				default:
					break;
		}
	}
}


void app_vcfparser_stor(vcf_parser_t *p_parser)
{
#if 0
	unsigned short nItemNameLen = 0;
	unsigned short nItemPropLen = 0;
	unsigned short nItemValLen = 0;

	if (p_parser == NULL)
		return;

	if (p_parser->p_item_name_start) {
		nItemNameLen = p_parser->p_item_name_end - p_parser->p_item_name_start + 1;

		if (nItemNameLen >= MAX_VCF_ITEM_LEN) {
			LOG("Name Item(%d) too long(larger than %d), should be cut.",
			              nItemNameLen, MAX_VCF_ITEM_LEN);
			nItemNameLen = MAX_VCF_ITEM_LEN - 1;
		}

		if (nItemNameLen > 0) {
			memcpy(p_parser->p_pb_item->sLine, p_parser->p_item_name_start, nItemNameLen);
			if(*(p_parser->p_item_name_end + 1) == ';' || *(p_parser->p_item_name_end + 1) == ':') {
				memcpy(p_parser->p_pb_item->sLine + nItemNameLen, p_parser->p_item_name_end + 1 , 1);
				nItemNameLen += 1;
				p_parser->p_item_name_start = p_parser->p_pb_item->sLine;
				p_parser->p_item_name_end = p_parser->p_item_name_start + nItemNameLen - 1 - 1;//  More out of a ';'

			} else {
				p_parser->p_item_name_start = p_parser->p_pb_item->sLine;
				p_parser->p_item_name_end = p_parser->p_item_name_start + nItemNameLen - 1;
			}

		}
	}

	if (p_parser->p_item_prop_start) {
		nItemPropLen = p_parser->p_item_prop_end - p_parser->p_item_prop_start + 1;

		if (nItemNameLen + nItemPropLen >= MAX_VCF_ITEM_LEN) {
			LOG("Name plus Property item(%d) too long(larger than %d), should be cut.",
			              (nItemNameLen + nItemPropLen), MAX_VCF_ITEM_LEN);
			nItemPropLen = MAX_VCF_ITEM_LEN - nItemNameLen - 1;
		}

		if (nItemPropLen > 0) {
			memcpy(p_parser->p_pb_item->sLine + nItemNameLen, p_parser->p_item_prop_start, nItemPropLen);
			if(*(p_parser->p_item_prop_end + 1) == ':' || *(p_parser->p_item_prop_end + 1) == ';') {
				memcpy(p_parser->p_pb_item->sLine + nItemNameLen + nItemPropLen, p_parser->p_item_prop_end + 1, 1);
				// nItemNameLen += 1;
				nItemPropLen += 1;
				p_parser->p_item_prop_start = p_parser->p_pb_item->sLine + nItemNameLen;
				p_parser->p_item_prop_end = p_parser->p_item_prop_start + nItemPropLen - 1 - 1;// More out of a ':'
			} else {
				p_parser->p_item_prop_start = p_parser->p_pb_item->sLine + nItemNameLen;
				p_parser->p_item_prop_end = p_parser->p_item_prop_start + nItemPropLen - 1;
			}
		}
	}

	if (p_parser->p_item_val_start) {
		nItemValLen = p_parser->p_item_val_end - p_parser->p_item_val_start + 1;

		if (nItemNameLen + nItemPropLen + nItemValLen >= MAX_VCF_ITEM_LEN) {
			LOG("Name plus Property plus Value item(%d) too long(larger than %d), should be cut.",
			              (nItemNameLen + nItemPropLen + nItemValLen), MAX_VCF_ITEM_LEN);
			nItemValLen = (MAX_VCF_ITEM_LEN - nItemPropLen - nItemNameLen - 1);
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
#endif
}

void app_vcfparser_free(vcf_parser_t *p_parser, int is_done)
{
	if(is_done)
	{
		if (PB_name_stru) {
			FREE(PB_name_stru);
			PB_name_stru = NULL;
		}
	}
	
	if (p_parser != NULL) {
		if (p_parser->p_pb_item != NULL) {
			FREE(p_parser->p_pb_item);
		}

		FREE(p_parser);
	}
}

typedef struct {
    UINT8 version[32];
    vcard_xml_entry_t *entries;
    int entry_count;
    int capacity;
} vcard_xml_list_t;


vcard_xml_list_t* vcard_xml_list_init() {
    vcard_xml_list_t *list = (vcard_xml_list_t*)NEW(sizeof(vcard_xml_list_t));
    if (!list) return NULL;

    list->entry_count = 0;
    list->capacity = 10;
    list->entries = (vcard_xml_entry_t*)NEW(sizeof(vcard_xml_entry_t) * list->capacity);
    if (!list->entries) {
        FREE(list);
        return NULL;
    }

    memset(list->version, 0, sizeof(list->version));
    return list;
}

void vcard_xml_list_free(vcard_xml_list_t *list) {
    if (!list) return;

    if (list->entries) {
        FREE(list->entries);
    }
    FREE(list);
}

int vcard_xml_list_add_entry(vcard_xml_list_t *list, UINT8 *handle, UINT8 *name) {
    if (!list || !handle || !name) return -1;

    if (list->entry_count >= list->capacity) {
        int new_capacity = list->capacity * 2;
        vcard_xml_entry_t *new_entries = (vcard_xml_entry_t*)realloc(
                list->entries, sizeof(vcard_xml_entry_t) * new_capacity);
        if (!new_entries) return -1;

        list->entries = new_entries;
        list->capacity = new_capacity;
    }

    vcard_xml_entry_t *entry = &list->entries[list->entry_count];
    strncpy(entry->handle, handle, sizeof(entry->handle) - 1);
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->handle[sizeof(entry->handle) - 1] = '\0';
    entry->name[sizeof(entry->name) - 1] = '\0';

    list->entry_count++;
    return 0;
}
vcard_xml_list_t* parse_xml_vcard_listing(UINT8 *data, UINT16 data_len) {
    if (!data || data_len == 0) return NULL;

    vcard_xml_list_t *list = vcard_xml_list_init();
    if (!list) return NULL;

    UINT8 *xml_start = strstr((UINT8 *)data, "<?xml");
    if (!xml_start) {
        xml_start = strstr((UINT8 *)data, "<vCard-listing");
    }

    if (!xml_start) {
        LOG("Error: XML start not found in data");
        vcard_xml_list_free(list);
        return NULL;
    }

    UINT16 xml_len = data_len - (xml_start - (UINT8 *)data);
    UINT8 *xml_copy = (UINT8 *)NEW(xml_len + 1);
    if (!xml_copy) {
        vcard_xml_list_free(list);
        return NULL;
    }

    memcpy(xml_copy, xml_start, xml_len);
    xml_copy[xml_len] = '\0';

    LOG("XML Data to parse:\n%s", xml_copy);

    UINT8 *version_start = strstr(xml_copy, "version=\"");
    if (version_start) {
        version_start += 9; 
        UINT8 *version_end = strchr(version_start, '"');
        if (version_end && (version_end - version_start) < (int)sizeof(list->version) - 1) {
            strncpy(list->version, version_start, version_end - version_start);
            list->version[version_end - version_start] = '\0';
        }
    }

    UINT8 *card_start = xml_copy;
    while ((card_start = strstr(card_start, "<card")) != NULL) {
        UINT8 *handle_start = strstr(card_start, "handle=\"");
        UINT8 *name_start = strstr(card_start, "name=\"");

        if (handle_start && name_start) {
            handle_start += 8;
            name_start += 6;

            UINT8 *handle_end = strchr(handle_start, '"');
            UINT8 *name_end = strchr(name_start, '"');

            if (handle_end && name_end) {
                UINT8 handle[256] = {0};
                UINT8 name[256] = {0};

                strncpy(handle, handle_start, handle_end - handle_start);
                strncpy(name, name_start, name_end - name_start);
                handle[handle_end - handle_start] = '\0';
                name[name_end - name_start] = '\0';

                LOG("Found entry - Handle: %s, Name: %s", handle, name);

                vcard_xml_list_add_entry(list, handle, name);
            }
        }

        card_start++;
    }

    FREE(xml_copy);
    return list;
}

UINT8* extract_xml_obex_body(UINT8 *obex_response, UINT16 response_len, UINT16 *body_len) {
    if (!obex_response || response_len < 6) return NULL;

    UINT8 *xml_start = obex_response;
    UINT16 xml_data_len = response_len;

    UINT8 *xml_data = (UINT8*)NEW(xml_data_len);
    if (xml_data) {
        memcpy(xml_data, xml_start, xml_data_len);
        *body_len = xml_data_len;
        return xml_data;
    }

    return NULL;
}

void app_process_obex_xml_vcard_listing(UINT8 *obex_data, UINT16 data_len, vcard_xml_entry_t *first_entry) {
    LOG("Data length: %zu bytes", data_len);

    if (data_len<= 0) {
        LOG("Error: Data too short for XML extraction");
        return;
    }

    vcard_xml_list_t *list = parse_xml_vcard_listing(obex_data, data_len);
    if (!list) {
        LOG("Error: Failed to parse XML vCard listing");
        return;
    }

    LOG("Total Entries: %d", list->entry_count);
    LOG("----------------------------------------");

    for (int i = 0; i < list->entry_count; i++) {
        vcard_xml_entry_t *entry = &list->entries[i];
        LOG("Entry %d:", i + 1);
        LOG("  Handle: %s", entry->handle);
        LOG("  Name: %s", entry->name);
    }

#if 1
    if(list->entry_count > 0) {
        //default first ind
//        vcard_xml_entry_t *pbap_cbk_entry = &list->entries[0];
//	    char dpb_buf_ex[MAX_VCF_ITEM_LEN] = {0};
//	    snprintf(dpb_buf_ex, MAX_VCF_ITEM_LEN,"%d%c%s%c%s%c%d",1, 0xff, pbap_cbk_entry->name, 0xff, vcard_number, 0xff, 1);
//	    event_pbap_indication(EV_PBAP_PHONEBOOK_DATA_IND, (void *)dpb_buf_ex);
//
         memcpy(first_entry->name, list->entries[0].name, sizeof(list->entries[0].name));
         memcpy(first_entry->handle, list->entries[0].handle, sizeof(list->entries[0].handle));

         LOG("first_entry.name: %s, first_entry.handle:%s", first_entry->name, first_entry->handle);
    }
#endif

    vcard_xml_list_free(list);

    return;
}

