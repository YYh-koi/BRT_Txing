#include "porting.h"
#include "porting_utils.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "bt_debug.h"
#include <unistd.h> 
#include <fcntl.h>
#include <sys/types.h>    
//#include <sys/stat.h>
#include <errno.h>

#define MAX_INI_FILE_SIZE (8*1024)

static char s_tmp_file[MAX_FILE_NAME_LEN] = {0};

char *compose_file_path(const char *dir, const char *file)
{
	if (!dir || !file) return 0;

	char *p = s_tmp_file;
	int len = 0;
	int n;
	n = strlen(dir);
	len += n;

	if (len >= MAX_FILE_NAME_LEN) return 0;

	memcpy(p, dir, n);
	p += n;
	n = 1;
	len += n;

	if (len >= MAX_FILE_NAME_LEN) return 0;

	memcpy(p, "/", n);
	p += n;
	n = strlen(file);
	len += n;

	if (len >= MAX_FILE_NAME_LEN) return 0;

	memcpy(p, file, n);
	p += n;
	*p = '\0';
	return s_tmp_file;
}

char *ltrim(char *str)
{
	char *p = str;

	while (*p == ' ')
		p ++;

	return p;
}

/****************************************
 *  return:
 *  	NULL	 ---  no match
 *	NOT Null --- zero-terminated string
 ****************************************/
char* getlname(char* line, char token)
{
	char* p = NULL;
	char* pt = NULL;
	char* name = NULL;
	int len;

	if( *line == '#' )
		return NULL;

	pt = ltrim(line);
	p = strchr(pt, token);
	if( p == NULL )
		return NULL;
	len = strlen(pt) - strlen(p) ;

	name = (char*)malloc(len + 1);
	strncpy( name, pt, len );
	name[len] = '\0';
	return name;
}

int set_private_profile_string(const char *sec, const char *key,
                               const char *val, const char *name)
{
#if 0
	FILE *file = NULL;
	char line[MAX_LINE_LEN];
	size_t nread, write;
	size_t len = 0;
	char *name1;
	int ret = 0;
	int isfound = 0;
	char *newbuff;

	if( (file = fopen(name, "r")) == NULL ) {
		return 0;
	}
	if (NULL == (newbuff = (char*)malloc(MAX_INI_FILE_SIZE))) {
		fclose(file);
		return 0;
	}

	while (fgets(line, MAX_LINE_LEN, file)) {
		if (!(ret = strlen(line)) || line[0] != '['
		    || line[ret - 2] != ']') {
			len += sprintf(newbuff + len, "%s", line);
			continue;
		}
chksec:
		line[ret - 2] = '\000';
		len += sprintf(newbuff + len, "[%s]\n", line + 1);
		if (strcasecmp(line + 1, sec)) {
			continue;
		}

		while (fgets(line, MAX_LINE_LEN, file)) {
			if ((ret = strlen(line)) && line[0] == '['
			    && line[ret - 2] == ']')
				goto chksec;

			nread = strlen(line);
			if(nread <= 0)
				continue;

			name1 = getlname(line, '=');
			if(name1 && !strcasecmp(name1, key)) {
				isfound = 1;
				len += sprintf(newbuff + len, "%s=%s\n", key, val);
				if(name1)
					free(name1);
				break;
			}

			if(strlen(line) >= 0)
				len += sprintf(newbuff + len, "%s", line);
			if(name1)
				free(name1);
			name1 = NULL;
		}

		if(!isfound)
			continue;
		else {
			//read the remaining part of ini file
			while (fgets(line, MAX_LINE_LEN, file)) {
				if(strlen(line) > 0)
					len += sprintf(newbuff + len, "%s", line);
			}
			break;
		}
	}

	fclose(file);

	if( isfound == 0) {
		free(newbuff);
		return 0;
	}

	if( (file = fopen(name, "w")) == NULL ) {
		free(newbuff);
		return 0;
	}

	write = fwrite((void*) newbuff, sizeof(char), len, file);

	fflush(file);
	fclose(file);
	free(newbuff);
#endif
	return 1;
}
int get_private_profile_string(const char *sec, const char *key,
                               const char *defval, char *val, unsigned int size,
                               const char *name)
{
#if 0
	FILE *file;
	char *line;
	int line_len;
	int isfound;
	char *str;
	int ret;
	isfound = 0;

	if (!(file = fopen(name, "r"))) {
		line = NULL;
		goto defret;
	}

	line_len = MAX_LINE_LEN + strlen(key) + 2;   /*[Key name]=[Key Value]*/
	line = (char *) malloc(line_len);

	if (line == NULL) {
		goto defret;
	}

	memset(line, 0, line_len);

	while (fgets(line, line_len, file)) {
		if (!(ret = strlen(line)) || line[0] != '['
		    || line[ret - 2] != ']') {
			continue;
		}

chksec:
		line[ret - 2] = '\000';

		if (strcmp(line + 1, sec)) {
			continue;
		}

		while (fgets(line, line_len, file)) {
			if ((ret = strlen(line)) && line[0] == '['
			    && line[ret - 2] == ']') {
				goto chksec;
			}

			if (!(str = strtok(line, "=\n")) || (strcmp(str, key))) {
				continue;
			}

			if (NULL != (str = strtok(NULL, "=\n"))) {
				strncpy(val, str, size);
				isfound = 1;
				break;
			}

			memset(line, 0, line_len);
		}

		break;
	}

defret:

	if (!isfound) {
		memset(val, 0, size);

		if (defval != NULL) {
			ret = (strlen(defval) + 1) > size ? size - 1 : strlen(defval);
			memcpy(val, defval, ret);
		}
	}

	if (file) {
		fclose(file);
	}

	if (line != NULL) {
		free(line);
	}

	return strlen(val);
#else
    return 0;
#endif
}

int get_private_profile_int(const char *sec, const char *key, unsigned int defval,
                            const char *name)
{
	char str[16];
	int ret;
	char *p;
	get_private_profile_string(sec, key, "", str, sizeof(str), name);

	if (!str[0]) {
		return defval;
	}

	ret = strtol(str, &p, 0);

	if (*p == 0) return ret;

	return defval;
}


unsigned short get_snoop_file_size(void)
{
    unsigned short size = 0;
    size = get_private_profile_int("local", "snoop_size", 500, DEFAULT_SYSTEM_CONFIG_FILE); 
    //UART_Printf("get snoop file size: %d", size);
    return size;
}

int get_port_name(char *name_buf, unsigned int len)
{
	if (name_buf && len > 0) {
		get_private_profile_string("uartsetting", "port", " ", name_buf, len, TL_CONFIG_FILE_PATH);
		return 1;
	}

	return 0;
}

void debug_print_data(const char *tag, unsigned char *data, unsigned long len)
{
	unsigned long i, j, l;
	unsigned char tmp_str[140];
	unsigned char tmp_str1[10];

	for (i = 0; i < len; i += 16) {
		int n ;
		tmp_str[0] = '\0';
		n = i ;

		for (j = 0; j < 4; j++) {
			l = n % 16;

			if (l >= 10)
				tmp_str[3 - j] = (unsigned char)('A' + l - 10);
			else
				tmp_str[3 - j] = (unsigned char)(l + '0');

			n >>= 4 ;
		}

		tmp_str[4] = '\0';
		strcat((char *) tmp_str, ": ");

		/*
		   Output the hex bytes
		 */
		for (j = i; j < (i + 16); j ++) {
			int m ;

			if (j < len) {
				m = ((unsigned int)((unsigned char) * (data + j))) / 16 ;

				if (m >= 10)
					tmp_str1[0] = 'A' + (unsigned char) m - 10;
				else
					tmp_str1[0] = (unsigned char) m + '0';

				m = ((unsigned int)((unsigned char) * (data + j))) % 16 ;

				if (m >= 10)
					tmp_str1[1] = 'A' + (unsigned char) m - 10;
				else
					tmp_str1[1] = (unsigned char) m + '0';

				tmp_str1[2] = '\0';
				strcat((char *) tmp_str, (char *) tmp_str1);
				strcat((char *) tmp_str, " ");
			} else {
				strcat((char *) tmp_str, "   ");
			}
		}

		strcat((char *) tmp_str, "  ");
		l = strlen((char *) tmp_str);

		/* Output the ASCII bytes */
		for (j = i; j < (i + 16); j++) {
			if (j < len) {
				char c = * (data + j);

				if (c < ' ' || c > 'z') {
					c = '.';
				}

				tmp_str[l ++] = c;
			} else {
				tmp_str[l ++] = ' ';
			}
		}

		tmp_str[l ++] = '\r';
		tmp_str[l ++] = '\n';
		tmp_str[l ++] = '\0';
		bse_print_x(tag, (const char *) tmp_str);
	}
}


