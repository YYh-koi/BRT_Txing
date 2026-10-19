#include "porting.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "bt_debug.h"
#include "autoconf_bt.h"
#ifdef CONFIG_OS_ANDROID
#include <cutils/properties.h>
#include <cutils/log.h>
#else
#include <stdio.h>
#endif
#include "porting_utils.h"
#include "brlinkd.h"

#ifndef CONFIG_OS_ANDROID
#undef OUT_PUT_LOG_TO_FILE
#undef OUT_PUT_LOG_TO_FILE_BACKUP
#endif
static int s_debug_level = 0x0006003F;
static void output_linux_log_to_file(const char *tag, const char *str);
//static unsigned short s_logcat_size = 1024;

struct timeval {
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};

void bse_do_noting(const char *tag, const char *fmt, ...)
{

}
void bse_print(const char *str)
{
#ifdef CONFIG_OS_ANDROID
	__android_log_print(ANDROID_LOG_DEBUG, BSE_TAG, " %s", str);
#else
	printf("%s - %s\n", BSE_TAG, str);
#endif
}

void bse_print_x(const char *tag, const char *str)
{
#ifdef CONFIG_OS_ANDROID
	__android_log_print(ANDROID_LOG_DEBUG, tag, " %s", str);
#else
	if(sw_get_debug_mode()) {
		printf("%s - %s\n", tag, str);
	}
	//output_linux_log_to_file(tag, str);
#endif
}

void bse_print_va(const char *tag, const char *fmt, ...)
{
    if(sw_get_debug_mode()) {
        va_list ap;
        char buf[1024];
        va_start(ap,fmt);
        vsnprintf(buf, 1024, fmt, ap);
        bse_print_x(tag, buf);
        va_end(ap);
    }
}

static char time_string[64];

char *get_current_time_string()
{
#if 0
	struct tm *time_info;
	struct timeval tv;
	memset(&tv, 0, sizeof(struct timeval));
	memset(time_string, 0, 64);

	if (gettimeofday(&tv, NULL) < 0) {
	}

	time_info = localtime(& (tv.tv_sec));
	sprintf(time_string, "[%02d:%02d:%02d:%u]", time_info->tm_hour, time_info->tm_min, time_info->tm_sec, (unsigned int) tv.tv_usec/1000);
    printf("//TODO");
#endif
	return time_string;
}

int get_debug_level(const char *tag)
{
#if 0
#ifndef CONFIG_OS_ANDROID
#define PROPERTY_VALUE_MAX 92
#endif
	char buf[PROPERTY_VALUE_MAX];
	char key[64];
	sprintf(key, "log.tag.%s", tag);
	int ret = get_private_profile_string("local", "brlink_e", " ", buf, 64, TL_CONFIG_FILE_PATH);
	if(ret <= 1) {
#ifdef CONFIG_OS_ANDROID
		property_get(key, buf, "00000000");
#else
		return 0;
#endif
	}

	s_debug_level = (int) strtol(buf, NULL, 16);

	return (int) strtol(buf, NULL, 16);
#else
	return s_debug_level;
#endif
}

//#include <sys/syscall.h>
#ifdef OUT_PUT_LOG_TO_FILE
#include "porting.h"
#include <time.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <dirent.h>

#include <fcntl.h>
#include <unistd.h>

void update_oldest_file(char *d_name, short d_name_len, char *oldest_file);

static int s_fd;
static int s_logcat_max = 0;
#define MAX_TIME_LENGTH 64
#define MAX_LOGCAT_NAME_LEN 128
static char s_string_time[MAX_TIME_LENGTH];
static char s_snoop_name[MAX_LOGCAT_NAME_LEN];

#define MAX_LOGCAT_FILE_LEN 255
//logcat_20220301_094917.txt
#define FORMAT_SNOOP_FILE_NAME_LEN 26
#define MAX_SNOOP_FILE_COUNT 3

unsigned short get_logcat_file_size()
{
	printf("get_logcat_file_size");
    unsigned short size = 0;
    /*char *tl_config_file_path = compose_file_path(
			get_configure_path(CONFIGURATION_TYPE_READONLY), TL_CONFIG_FILE);*/
	size = get_private_profile_int("local", "logcat_size", 1024, TL_CONFIG_FILE_PATH);
	return size;
}


int get_file_num(char *path, char *oldest_file)
{
	DIR *dir = opendir(path);
	if (dir == NULL) {
		printf("opendir error");
		return -1;
	}

	struct dirent *ptr;

	int file_num = 0;

	while ((ptr = readdir(dir)) != NULL) {
		char *d_name = ptr->d_name;

		//discard . and ..
		if (strcmp(d_name, ".") == 0 || strcmp(d_name, "..") == 0) {
			continue;
		}

		if (ptr->d_type == DT_DIR) {//directory FIXME
			char new_path[MAX_LOGCAT_FILE_LEN] = {0};
			sprintf(new_path, "%s/%s", path, d_name);

			file_num += get_file_num(new_path, oldest_file);
		}

		if (ptr->d_type == DT_REG) {//file
			if (strlen(d_name) != FORMAT_SNOOP_FILE_NAME_LEN) {
				printf("### NOT btsnoop format");
				continue;
			}
			if (memcmp(d_name, "logcat_", 7) == 0) {
				printf("GOT file %s\r\n", d_name);
				file_num++;
				update_oldest_file(d_name, strlen(d_name), oldest_file);
			}
		}
	}

	closedir(dir);

	return file_num;
}

void update_oldest_file(char *d_name, short d_name_len, char *oldest_file)
{
	if (oldest_file[0] == 0) {
		memcpy(oldest_file, d_name, d_name_len);
		return;
	}

	//beacuse the format is the same, so we just use strcmp!!!!
	int ret = strcmp(d_name, oldest_file);
	//printf("ret = %d\r\n", ret);

	if (ret < 0) {
		memset(oldest_file, 0, MAX_LOGCAT_FILE_LEN);
		memcpy(oldest_file, d_name, d_name_len);
		//printf("oldest file is %s\r\n", oldest_file);
	}
}

void delete_oldest_file(char *root, char *f_name)
{
	if (f_name[0] == 0) {
		return;
	}
	char file[MAX_LOGCAT_FILE_LEN] = {0};
	snprintf(file, MAX_LOGCAT_FILE_LEN, "%s/%s", root, f_name);

	remove(file);
}

static char* get_current_time()
{
	struct tm *time_info;
	struct timeval tv;
	memset(&tv, 0, sizeof(struct timeval));
	memset(s_string_time, 0, MAX_TIME_LENGTH);
	if (gettimeofday(&tv, NULL) < 0) {
		//TODO
	}
	time_info = localtime(& (tv.tv_sec));
	snprintf(s_string_time, MAX_TIME_LENGTH, "%04d%02d%02d_%02d%02d%02d", time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
	         time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	return s_string_time;
}

static char* compose_logcat_name()
{
	memset(s_snoop_name, 0, MAX_LOGCAT_NAME_LEN);
	//char *time = get_current_time();
	//snprintf(s_snoop_name, MAX_LOGCAT_NAME_LEN, "logcat_%s.txt", time);
	snprintf(s_snoop_name, MAX_LOGCAT_NAME_LEN, "logcat_1551.txt");
	return s_snoop_name;
}
#ifdef OUT_PUT_LOG_TO_FILE_BACKUP
void porting_local_log_backup(char *p_path)
{
	if(porting_get_debug_level() & BSE_DEBUG_BTDEBUG_LOG_BACK)
	{
		printf("%s p_path = %s",__func__, p_path);
		if(p_path == NULL)
			return ;
			
		if (access(p_path, F_OK) == 0) {
			char new_name[128];
			size_t dot = 0;
			size_t len = strlen(p_path);
			strncpy(new_name, p_path, 125); /* 128 - sizeof(".bk")*/
			for (dot = len - 1; dot >= 0; dot--) {
				if (new_name[dot] == '.') {
					break;
				}
			}

			if (dot > 0) {
				strcpy(&new_name[dot + 1], "bk.log");
			}

			if (access(new_name, F_OK) == 0) {
				unlink(new_name);
			}
			if (rename(p_path, new_name)) {
				printf("%s Can rename %s to %s\n.", __func__, p_path, new_name);
				//_Printf_format_string_impl_("%s Can rename %s to %s:%s.", __func__, p_path, new_name, strerror(errno));
			}
		}
	}

}
#else
void porting_local_log_backup(char *p_path)
{
	return ;
}
#endif


void brt_debug_open_linux()
{
	if ((s_debug_level & BSE_DEBUG_LEVEL_OUTPUTFILE) == 0) {
		return;
	}
	/*char *root_path = (char *)get_configure_path(CONFIGURATION_TYPE_LOG_FILE);//CONFIGURATION_TYPE_READWRITE

	s_logcat_size = get_logcat_file_size();

	char oldest_file[MAX_LOGCAT_FILE_LEN] = {0};
	int num = get_file_num(root_path, oldest_file);
	if (num >= MAX_SNOOP_FILE_COUNT) {
		delete_oldest_file(root_path, oldest_file);
	}*/
	
	//char *p_path = compose_file_path(root_path, compose_logcat_name());

	s_logcat_size = get_logcat_file_size();

	char *p_path =	"/data/misc/brlink/logcat.log";

	char logcat_path[256] = {0};

	//int ret = porting_get_property("local", "save_path", " ", log_path, 128, TL_CONFIG_FILE_PATH);
	int ret = get_private_profile_string("local", "save_path", "/data/misc/brlink", logcat_path, 128, TL_CONFIG_FILE_PATH);

	sprintf(logcat_path, "%s/%s", logcat_path, "logcat.log");

	printf("logcat_path = %s s_logcat_size = %d\n", logcat_path, s_logcat_size);

	porting_local_log_backup(&logcat_path);

	s_fd = open(logcat_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

	if (s_fd == -1) {
		//s_fd = -1;
		if(memcmp(logcat_path, "/data/misc/brlink/logcat.log", strlen("/data/misc/brlink/logcat.log")) == 0) {
			printf("brt_debug_open_linux: Unable to open logcat log file\n");
			return;
		} else {
			porting_local_log_backup(&p_path);
			s_fd = open(p_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (s_fd == -1) {
				printf("double brt_debug_open_linux: Unable to open logcat log file\n");
			}
		}		
	}
	s_logcat_max = 0;
	chmod(p_path, 0755);
		
}

void brt_debug_close_linux()
{
	if (s_fd >0 ) {
		printf("brt_debug_close_linux: Closing logcat log file\n");
		close(s_fd);
		s_fd = -1;
	}
}

static int check_logcat_oversized(unsigned int length)
{
	unsigned long file_size;
	struct stat stbuf;
	
	if (s_fd == -1) {
		return 1;
	}
	if ((fstat(s_fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
		//LOG("%s, get logcat file size failed(%s)", __func__, strerror(errno));
		//printf("%s, get logcat file size failed(%s)", __func__, strerror(errno));
		printf("%s, get logcat file size failed\n", __func__);
		return 1;
	}
	file_size = stbuf.st_size;

	if ((int)(file_size + length) >= s_logcat_size * 1024 * 1024) {

		s_logcat_max = 1;
		printf("logcat file is full(%dM)", s_logcat_size);
		//LOG("logcat2 file is full(%dM)", s_logcat_size);
		//LOG("logcat file is full(%dM)", s_logcat_size);
		return 1;
	}
	return 0;
}

static void output_linux_log_to_file(const char *tag, const char *str)
{
	if (s_debug_level & BSE_DEBUG_LEVEL_OUTPUTFILE)
	{
		if (s_fd > 0) {
			unsigned int loglen = strlen(tag) + strlen(str) + 32;
			if(s_logcat_max || check_logcat_oversized(loglen))
				return ;

			char log_head[128] = {0};
			snprintf(log_head, 128, "%s[%d, %ld]", get_current_time_string(), getpid(), syscall(SYS_gettid));
			write(s_fd, log_head, strlen(log_head));
			write(s_fd, tag, strlen(tag));
			write(s_fd, "  ", strlen("  "));
			write(s_fd, str, strlen(str));
			write(s_fd, "\r\n", 2);
		}
	} else {
		printf("%s[%d, %ld] %s - %s\n", get_current_time_string(), getpid(), syscall(SYS_gettid), tag, str);
	}
}

#else
void brt_debug_open_linux(){};
void brt_debug_close_linux(){};
static void output_linux_log_to_file(const char *tag, const char *str)
{
}
#endif
