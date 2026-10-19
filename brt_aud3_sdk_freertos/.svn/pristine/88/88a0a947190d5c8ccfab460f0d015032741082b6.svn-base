#ifndef _TRACE_H_
#define _TRACE_H_
#include "btdatatype.h"
#include "../../include/autoconf_bt.h"
#include "../../include/porting.h"

#ifdef MODULE_TAG
#define BRT_DBG(fmt, ...)  bse_log_print(MODULE_TAG, "\r\n", " %s: " fmt, __FUNCTION__ , ## __VA_ARGS__)
#define BRT_DBG_ENTER()  bse_log_print(MODULE_TAG, "\r\n", " Enter %s", __FUNCTION__)
#define BRT_DBG_LEAVE()  bse_log_print(MODULE_TAG, "\r\n", " Leave %s", __FUNCTION__)
#else
#define BRT_DBG(fmt, ...) ((void) 0)
#define BRT_DBG_ENTER() ((void) 0)
#define BRT_DBG_LEAVE() ((void) 0)
#endif

void bse_log_print(const char *tag, const char *lf_str, const char *fmt, ...);
void bse_log_print_ex(const char *tag, const int level, const char *lf_str, const char *fmt, ...);

#define bse_log_api(tag, fmt, args...) bse_log_print_ex(tag, BSE_DEBUG_LEVEL_API, "", fmt, ##args)
#define bse_log_info(tag, fmt, args...) bse_log_print_ex(tag, BSE_DEBUG_LEVEL_INFO, "", fmt, ##args)
#define bse_log_warning(tag, fmt, args...) bse_log_print_ex(tag, BSE_DEBUG_LEVEL_WARNING, "", fmt, ##args)
#define bse_log_error(tag, fmt, args...) bse_log_print_ex(tag, BSE_DEBUG_LEVEL_ERROR, "", fmt, ##args)
#define bse_log_verbose(tag, fmt, args...) bse_log_print_ex(tag, BSE_DEBUG_LEVEL_VERBOSE, "", fmt, ##args)
#endif
