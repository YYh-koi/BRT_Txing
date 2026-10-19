#ifndef POLLING_UTILS_H_
#define POLLING_UTILS_H_

#define MAX_FILE_NAME_LEN 256
#define MAX_LINE_LEN 1024

#ifdef CONFIG_OS_ANDROID
#define SAVE_FILE_ROOT_DIR "/etc/backup"
#define CONFIG_FILE_ROOT_DIR "/etc/backup"
#define FW_FILE_ROOT_DIR "/etc/backup"
#else
#define SAVE_FILE_ROOT_DIR "/data/misc/brlink"
#define CONFIG_FILE_ROOT_DIR "/etc/brlink"
#define FW_FILE_ROOT_DIR "/etc/brlink"
#endif

#define BRLINK_CONFIG_FILE "BtSettings.INI"
#define TL_CONFIG_FILE "TL.INI"
#define TL_CONFIG_FILE_PATH             compose_file_path(CONFIG_FILE_ROOT_DIR, TL_CONFIG_FILE)
#define BTSNOOP_LOG_FILE "btsnoop.log"

#define DEFAULT_BD_ADDR_CONFIG_FILE "/data/misc/brlink/BD_ADDR.INI"
#define DEFAULT_DATA_CONFIG_FILE "/data/misc/brlink/TL.INI"
#define DEFAULT_SYSTEM_CONFIG_FILE "/etc/brlink/TL.INI"


char *compose_file_path(const char *dir, const char *file);
int set_private_profile_string(const char *sec, const char *key,
                               const char *val, const char *name);
int get_private_profile_string(const char *sec, const char *key,
                               const char *defval, char *val, unsigned int size,
                               const char *name);
int get_private_profile_int(const char *sec, const char *key, unsigned int defval,
                            const char *name);
int get_port_name(char *name_buf, unsigned int len);

void debug_print_data(const char *tag, unsigned char *data, unsigned long len);
const char *porting_get_configure_path(unsigned char type);
unsigned short get_snoop_file_size(void);

int set_rand_bdaddr(unsigned char* bd);

unsigned long get_system_time();


#endif // POLLING_UTILS_H_
