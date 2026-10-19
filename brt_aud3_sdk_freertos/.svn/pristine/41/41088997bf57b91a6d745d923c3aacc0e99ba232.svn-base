#ifndef BT_DEBUG_H__
#define BT_DEBUG_H__

#define BSE_TAG "brlink_e"

void bse_do_noting(const char *tag, const char *fmt, ...);
void bse_print(const char *str);
void bse_print_x(const char *tag, const char *str);
void bse_print_va(const char *tag, const char *fmt, ...);
char* get_current_time_string();
int get_debug_level(const char *tag);

#ifndef CONFIG_OS_ANDROID
void brt_debug_open_linux();
void brt_debug_close_linux();
#endif

void porting_local_log_backup(char *p_path);

#endif // BT_DEBUG_H__
