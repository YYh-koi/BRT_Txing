#include "btdatatype.h"
#include "porting.h"
#include <sys/types.h>
#include "task.h"
#include "sfud.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <libgen.h>
#include "autoconf_bt.h"
#include "tl.h"
/* for transport power management */
#include "upio.h"

#include "porting_utils.h"
#include "bt_debug.h"
#include <errno.h>
#include "bt_snoop.h"
#include "internal_manager.h"

#ifdef CONFIG_FLASH_SAVE_FILE
static sfud_flash *flash_BtSettings; 
#define BT_FLASH_USE_SIZE	((DEV_INFO_LEN/4096) * 4096 + 4096)  //for 4K * X
#define BT_FLASH_OFFSET (flash_BtSettings->chip.capacity - BT_FLASH_USE_SIZE) //use flash last addr
#endif

//#define PORTINGS_DBG
#ifdef PORTINGS_DBG
#define PORTING_DBG(fmt, ...) bse_print_va("portings", fmt, ## __VA_ARGS__)
#else
#define PORTING_DBG(fmt, ...) {}
#endif


#define BRLINK_CONFIG_FILE_PATH        compose_file_path(SAVE_FILE_ROOT_DIR, BRLINK_CONFIG_FILE)
#define BRLINK_BTSNOOP_FILE_PATH       compose_file_path(SAVE_FILE_ROOT_DIR, BTSNOOP_LOG_FILE)
#define TL_CONFIG_FILE_PATH             compose_file_path(CONFIG_FILE_ROOT_DIR, TL_CONFIG_FILE)
#define LOG_FILE             			compose_file_path(SAVE_FILE_ROOT_DIR, BTSNOOP_LOG_FILE)

extern UINT8 scheduleloop_p;

static int s_bse_debug_level = 0x0006003F;; // 0 means not output btsnoop log

static void check_and_set_debug_flag();

static tl_data_ind s_uart_tl_data_cb;
// static data_cb s_data_cb;
static void uart_data_ind_callback(unsigned char *data, size_t len)
{
	if (!data) {
		return;
	}

	s_uart_tl_data_cb(data, len);
}

static void uart_tl_init(tl_data_ind cb)
{
	s_uart_tl_data_cb = cb;
	open_port(uart_data_ind_callback);
	upio_init();
}

static void uart_tl_init_ex(transport_layer_callbacks_t *cbs)
{
	tl_init_ex(cbs);
}

static void uart_tl_done(void)
{
	close_port();
	upio_cleanup();
}

static void uart_tl_send(unsigned char *data, unsigned short len)
{
	write_port(data, len);
}

static void uart_tl_power(unsigned char on)
{
	if (on) {
		upio_set_bluetooth_power(1);
	} else {
		upio_set_bluetooth_power(0);
	}
}

static void uart_tl_fw_config(void *param)
{
	tl_fw_config(param);
}

static SYSTEM_LOCK porting_create_critical(const char *name)
{
	pthread_mutex_t *pmutex;
	if (!(pmutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))))
		return (SYSTEM_LOCK)NULL;
	pthread_mutex_init(pmutex, NULL);
	return (SYSTEM_LOCK) pmutex;
}

static void porting_enter_critical(SYSTEM_LOCK lock)
{
	if (lock != NULL) {
		pthread_mutex_lock((pthread_mutex_t *) lock);
	}
}

static void porting_leave_critical(SYSTEM_LOCK lock)
{
	if (lock != NULL) {
		pthread_mutex_unlock((pthread_mutex_t *) lock);
	}
}

static void porting_free_critical(SYSTEM_LOCK lock)
{
	if (lock != NULL) {
		int ret = 0;
		ret = pthread_mutex_destroy((pthread_mutex_t*) lock);
		if(ret != 0)
		{
			PORTING_DBG("porting_free_critical  fail, %s\n", strerror(ret));
		}
		free((pthread_mutex_t*) lock);
	}
}

static void* porting_create_event(const char *name)
{
	sem_t *sem;

	if (name) {
		bse_print_va("portings", "%s(%s)", __FUNCTION__, name);
	}

	if (!(sem = (sem_t *)malloc(sizeof(sem_t))))
		return NULL;

	if (sem_init(sem, 0, 0) == -1) {
		free(sem);
		return NULL;
	}

	return (HANDLE) sem;
}

static void porting_wait_event(void *event)
{
	if (event != NULL) {
		sem_wait((sem_t *) event);
	}
}

static void porting_wait_timeout_event(void *event, unsigned int milliseconds)
{
	bse_print_va("portings", "%s(%u)", __FUNCTION__, milliseconds);
	if (event != NULL) {
		struct timespec ts;
		memset(&ts, 0, sizeof(struct timespec));
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += milliseconds / 1000;
		ts.tv_nsec += (milliseconds % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000) {
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}
		sem_timedwait((sem_t *) event, &ts);
	}
}

static void porting_signal_event(void *event)
{
	if (event != NULL) {
		sem_post((sem_t *) event);
	}
}

static void porting_delete_event(void *event)
{
	sem_destroy((sem_t *) event);
	free(event);
}

static void porting_thread_proc(void *param)
{
	porting_thread_func func = (porting_thread_func) param;
	if (func != NULL) {
		(*func)();
	}
	//pthread_exit(NULL);
	vTaskDelete(NULL);
	//return NULL;
}

void *porting_create_thread_double_stack(porting_thread_func func)
{
#ifdef CONFIG_THREAD_SCHED
	pthread_t id;
	struct sched_param sched;

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	//sched.sched_priority = 98;//1
	sched.sched_priority = sched_get_priority_max(SCHED_RR);//99;//1
	pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
	//pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&thread_attr, &sched);
    pthread_create(&id, &thread_attr, porting_thread_proc, func);
#else
	//pthread_create(&id, NULL, porting_thread_proc, func);
	int id = 0;
	id = xTaskCreate(
   	 porting_thread_proc,
     "porting_thread_proc",
   	 2048*2,
   	 func,
   	 scheduleloop_p,//configMAX_PRIORITIES/2,
     NULL
  );
#endif
	//pthread_setschedparam(id, SCHED_FIFO,&sched );
	//pthread_detach(id);

	bse_print_va("portings", "%s(id:0x%08X)", __FUNCTION__, id);
	return (void *) id;
}

static void *porting_create_thread(porting_thread_func func)
{
#ifdef CONFIG_THREAD_SCHED
	pthread_t id;
	struct sched_param sched;

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	//sched.sched_priority = 98;//1
	sched.sched_priority = sched_get_priority_max(SCHED_RR);//99;//1
	pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
	//pthread_attr_setinheritsched(&thread_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&thread_attr, &sched);
    pthread_create(&id, &thread_attr, porting_thread_proc, func);
#else
	//pthread_create(&id, NULL, porting_thread_proc, func);
	int id = 0;
	id = xTaskCreate(
   	 porting_thread_proc,
     "porting_thread_proc",
   	 2048,
   	 func,
   	 configMAX_PRIORITIES/2,
     NULL
  );
#endif
	//pthread_setschedparam(id, SCHED_FIFO,&sched );
	//pthread_detach(id);

	bse_print_va("portings", "%s(id:0x%08X)", __FUNCTION__, id);
	return (void *) id;
}

static void porting_join_thread(void *hdl)
{
//	pthread_t id = (pthread_t) hdl;
//
//	bse_print_va("portings", "%s(id:0x%08X)", __FUNCTION__, id);
//	if (id != 0) {
//		pthread_join(id, NULL);
//	}
}

static unsigned long porting_get_current_time()
{
#if 1
    unsigned long time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);//CLOCK_MONOTONIC,CLOCK_MONOTONIC_RAW
    time = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    return time;
    //return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else
    struct tm *time_info;
    struct timeval tv;
    unsigned long ul_time = 0;
    memset(&tv, 0, sizeof(struct timeval));

    if (gettimeofday(&tv, NULL) < 0) {
    }

    time_info = localtime(& (tv.tv_sec));
    ul_time = (time_info->tm_hour*3600+time_info->tm_min*60+time_info->tm_sec) * 1000 + tv.tv_usec / 1000;
    return ul_time;
#endif
}

unsigned long get_system_time()
{
    return porting_get_current_time();
}

static void porting_print(const char *tag, const int debug_level, const char *newline_break, const char *str)
{
	if (s_bse_debug_level & debug_level) {
		bse_print_x(tag, str ? str : "NULL");
	}
}

static void porting_set_debug_level(unsigned int level)
{
	unsigned int top_byte = (level & 0xF0000000) >> 28;
	unsigned int other_bytes = level & 0x0FFFFFFF;

	// TODO local debug level should be notified to upper layer in the initiablization.
	switch (top_byte) {
	case 0:
		s_bse_debug_level = other_bytes;
		break;
	case 1:
		s_bse_debug_level |= BSE_DEBUG_LEVEL_BTSNOOP;
		btsnoop_init();
		btsnoop_open(LOG_FILE);
		break;
	case 2:
		s_bse_debug_level &= ~BSE_DEBUG_LEVEL_BTSNOOP;
		btsnoop_close();
		btsnoop_cleanup();
		break;
	default:
		bse_print_va(BSE_TAG, "Error in set_debug_level: 0x%08X.",
		             level);
		break;
	}
}

const char *porting_get_configure_path(unsigned char type) {
	switch (type) {
		case CONFIGURATION_TYPE_READONLY:
			return CONFIG_FILE_ROOT_DIR;
		case CONFIGURATION_TYPE_READWRITE:
			return SAVE_FILE_ROOT_DIR;
		case CONFIGURATION_TYPE_BINARY:
			return FW_FILE_ROOT_DIR;
	}

	return NULL;
}

void porting_set_configure_path(unsigned char type, char *path)
{
//	int len = strlen(path)>MAX_FILE_NAME_LEN?MAX_FILE_NAME_LEN:strlen(path);
	int len = strlen(path);
	printf("porting_set_configure_path: type is %d, path is %s, len is %d\r\n", type, path, len);
#if 0
	switch (type) {
		case CONFIGURATION_TYPE_READONLY:
			memset(s_root_dir_config, 0, len);
			memcpy(s_root_dir_config, path, len);
			break;
		case CONFIGURATION_TYPE_READWRITE:
			memset(s_root_dir_save, 0, len);
			memcpy(s_root_dir_save, path, len);
			break;
		case CONFIGURATION_TYPE_BINARY:
			memset(s_root_dir_fw, 0, len);
			memcpy(s_root_dir_fw, path, len);
			//TODO
			break;

	}
#endif

}



static void porting_hci_file_open()
{

	s_bse_debug_level = get_debug_level(BSE_TAG);

	bse_print_va(BSE_TAG, "get_debug_level of %s:0x%08X.", BSE_TAG, s_bse_debug_level);
#if 0
	// FIXME It is best to create dir in common initialization function
	struct stat st;
	if (stat(SAVE_FILE_ROOT_DIR, &st) != 0) {
		if (mkdir(SAVE_FILE_ROOT_DIR, 0777) != 0) {
			bse_print_va(BSE_TAG, "mkdir %s failed!", SAVE_FILE_ROOT_DIR);
		}
		chmod(SAVE_FILE_ROOT_DIR, 0777);
	}

	char log_path[256] = {0};

	//int ret = porting_get_property("local", "save_path", " ", log_path, 128, TL_CONFIG_FILE_PATH);
	int ret = get_private_profile_string("local", "save_path", "/data/misc/brlink", log_path, 128, TL_CONFIG_FILE_PATH);

	if(ret <= 0)
	{
		memset(log_path, 0, 256);
		memcpy(log_path, "/data/misc/brlink", strlen("/data/misc/brlink/btsnoop.log"));
	}

	printf("ret = %d, log_path = %s LOG_FILE = %s\n", ret, log_path, LOG_FILE);
	//check_and_set_debug_flag();

	if ((s_bse_debug_level & BSE_DEBUG_LEVEL_BTSNOOP) > 0) {

		btsnoop_init();
		printf("s_bse_debug_level & BSE_DEBUG_LEVEL_BTSNOOP ret = %d\n", ret);
		//if(ret > 0){//ret > 0
			sprintf(log_path, "%s/%s", log_path, "btsnoop.log");

			porting_local_log_backup(&log_path);

			//printf("log_path = %s\n", log_path);
			btsnoop_open(log_path);
		//} else {
			//btsnoop_open("/data/misc/brlink/btsnoop.log");
		//}
	}

#ifndef CONFIG_OS_ANDROID
	brt_debug_open_linux();
#endif
#endif
}

static void porting_hci_file_close()
{
	if ((s_bse_debug_level & BSE_DEBUG_LEVEL_BTSNOOP) > 0) {
		btsnoop_close();
		btsnoop_cleanup();
	}

#ifndef CONFIG_OS_ANDROID
	brt_debug_close_linux();
#endif
}

static void porting_capture_hci_data(unsigned char direction, unsigned int length, unsigned char *data)
{
	if ((s_bse_debug_level & BSE_DEBUG_LEVEL_BTSNOOP) > 0) {
		btsnoop_capture(data, length, direction);
	}
}

int porting_get_debug_level()
{
	return s_bse_debug_level;
}

static int porting_read_configure(unsigned char nFlag, unsigned short offset, int len, unsigned char *data)
{
#ifdef CONFIG_FLASH_SAVE_FILE 
	if(flash_BtSettings != NULL) {
		printf("%s:BT_FLASH_OFFSET + offset:%d, len:%d\n", __func__, BT_FLASH_OFFSET + offset, len);
		if (sfud_read(flash_BtSettings, BT_FLASH_OFFSET + offset, len, data) != SFUD_SUCCESS) {
			printf("%s:Read failed!\n", __func__);
			return 0;
		}
	}
#else
	int ret;
	if (len <= 0 || data == NULL) {
		return 0;
	}

	FILE *fp = fopen(BRLINK_CONFIG_FILE_PATH, "rb");

	if (fp != NULL) {
		ret = fseek(fp, offset, SEEK_SET);

		size_t rd = 0;
		if (ret == 0) {
			rd = fread(data, 1, len, fp);
		}
		fclose(fp);
		return rd;
	} else {
		printf("%s open %s failed\n", __func__, BRLINK_CONFIG_FILE_PATH);
	}
#endif
	return 0;
}

static int porting_write_configure(unsigned char nFlag, unsigned short offset, int len, unsigned char *data)
{
#ifdef CONFIG_FLASH_SAVE_FILE
	if(flash_BtSettings != NULL) {
		printf("%s:BT_FLASH_OFFSET + offset:%d, len:%d\n", __func__, BT_FLASH_OFFSET + offset, len);
		if (sfud_erase(flash_BtSettings, BT_FLASH_OFFSET + offset, len) != SFUD_SUCCESS) {
			printf("%s:Erase failed!\n", __func__);
			return 0;
		}
	
		if (sfud_write(flash_BtSettings, BT_FLASH_OFFSET + offset, len, data) != SFUD_SUCCESS) {
			printf("%s:Write failed!\n", __func__);
			return 0;
		}
	}
#else
	int ret;
	if (len <= 0 || data == NULL) {
		return 0;
	}

	FILE *fp = fopen(BRLINK_CONFIG_FILE_PATH, "rb+");

	if (fp == NULL && errno == ENOENT) {
		if (access(dirname(BRLINK_CONFIG_FILE_PATH), F_OK) == -1) {
			mkdir(dirname(BRLINK_CONFIG_FILE_PATH), 0777);
		}

		fp = fopen(BRLINK_CONFIG_FILE_PATH, "wb");
	}

	if (fp != NULL) {
		ret = fseek(fp, offset, SEEK_SET);

		size_t wd = 0;
		if (ret == 0) {
			wd = fwrite(data, 1, len, fp);
		}
		fflush(fp);
		system("sync");
		/*fsync(fp);*/
		fclose(fp);
		chmod(BRLINK_CONFIG_FILE_PATH, 0666);
		return wd;
	}
#endif
	return 0;
}

#if 0
int porting_get_local_property_ex(const char *tag, unsigned char *value, int *value_len, char *file_path)
{
	if (value && (*value_len) > 0) {
		if(file_path) {
			get_private_profile_string("local", tag, "", value, *value_len, file_path);
		} else {
			get_private_profile_string("local", tag, "", value, *value_len, TL_CONFIG_FILE_PATH);
		}
		*value_len = (*value_len < strlen(value))?*value_len:strlen(value);
		return 1;
	}
	return 0;
}
#endif

int porting_get_property(const char *sec, const char *tag, unsigned char *value, int *value_len, char* file_path)
{
#if 0
	if (value && (*value_len) > 0) {
		if(!sec) {
			return -1;
		}
		if(file_path) {
			get_private_profile_string(sec, tag, "", value, *value_len, file_path);
		} else {
			get_private_profile_string(sec, tag, "", value, *value_len, TL_CONFIG_FILE_PATH);
		}
		*value_len = (*value_len < strlen(value))?*value_len:strlen(value);
		return 1;
	}
#endif
	return 0;
}

int porting_set_local_property(const char *tag, unsigned char *value)
{
	return 0;
}

void porting_sleep(unsigned long ms)
{
	struct timespec delay;
	int err;
	delay.tv_sec = ms / 1000;
	delay.tv_nsec = 1000 * 1000 * (ms % 1000);

	/* [u]sleep can't be used because it uses SIGALRM */
	do {
		err = nanosleep(&delay, &delay);
	} while (err < 0);// && errno == EINTR);
}

static transport_layer_t s_tl = {
	uart_tl_init,
	uart_tl_send,
	uart_tl_done,
	uart_tl_init_ex,
	uart_tl_power,
	uart_tl_fw_config,
};

static porting_functions_t s_porting_functions = {
	&s_tl,
	porting_get_current_time,

	porting_create_critical,
	porting_enter_critical,
	porting_leave_critical,
	porting_free_critical,

	porting_create_event,
	porting_wait_event,
	porting_wait_timeout_event,
	porting_signal_event,
	porting_delete_event,

	porting_create_thread,
	porting_join_thread,

	porting_print,
	porting_hci_file_open,
	porting_hci_file_close,
	porting_capture_hci_data,
	porting_get_debug_level,

	porting_read_configure,
	porting_write_configure,
	porting_get_property,
	porting_set_local_property,
	NULL,
	NULL,
	porting_sleep,
	porting_set_debug_level,
	porting_get_configure_path,
	NULL,
};

porting_functions_t *get_porting_functions()
{
#ifdef CONFIG_FLASH_SAVE_FILE
	printf("DEV_INFO_LEN;%d, BT_FLASH_USE_SIZE:%d\n", DEV_INFO_LEN, BT_FLASH_USE_SIZE);

	flash_BtSettings= sfud_get_device(0);
 	if (flash_BtSettings == NULL) {
		printf("brt No Flash device found!\n");	
 	} else {
		printf("brt Flash detected: %s, Size: %lu bytes\n", flash_BtSettings->name, flash_BtSettings->chip.capacity);
		printf("BT_FLASH_OFFSET:0x%x\n", BT_FLASH_OFFSET);
	}
#endif
	return &s_porting_functions;
}

#if 0
static void check_and_set_debug_flag()
{
	unsigned char flag = DBG_PART_APP;

	if (s_bse_debug_level & BSE_DEBUG_LEVEL_HCI)
		flag |= DBG_PART_DATA;
	if (s_bse_debug_level & BSE_DEBUG_LEVEL_STACK)
		flag |= DBG_PART_STACK;

	g_trace_flag = flag;
#ifdef CONFIG_BT_DEBUG
	if (!(g_trace_flag & DBG_PART_DATA))
		dbg_flags &=(~DBG_DATA_ON);
#endif
}
#endif


void btsnoop_save_avdtp_cid(UINT16 cid, long stream_handle)
{
	printf("%s cid = %d stream_handle = %x\n",__func__, cid, stream_handle);
    #ifdef CONFIG_OPEN_FILTER_DATA
    brt_debug_save_avdtp_cid(cid, stream_handle);
    #endif
}


void btsnoop_clean_avdtp_cid(long stream_handle)
{
	printf("%s stream_handle = %x\n",__func__, stream_handle);
    #ifdef CONFIG_OPEN_FILTER_DATA
    brt_debug_clean_avdtp_cid(stream_handle);
    #endif
}

void btsnoop_cleanup_all_avdtp()
{
	printf("%s\n",__func__);
    #ifdef CONFIG_OPEN_FILTER_DATA
    brt_debug_cleanup_all_avdtp();
    #endif
}