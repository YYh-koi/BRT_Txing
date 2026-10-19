#include <stdio.h>
#include <stdarg.h>
#include "brt_trace.h"
#include "debug.h"
#include "brlinkd.h"

#define DBG_STRING_MAX_LEN	2048
static char string[DBG_STRING_MAX_LEN];
void bse_log_print_ex(const char *tag, const int level, const char *lf_str, const char *fmt, ...)
{
	if(sw_get_debug_mode()) {
		va_list ap;
		va_start(ap,fmt);
#ifdef CONFIG_OS_LINUX
		vsnprintf(string, DBG_STRING_MAX_LEN , fmt, ap);
#elif CONFIG_OS_WIN32
		_vsnprintf(string, DBG_STRING_MAX_LEN , fmt, ap);
#else
		vsprintf(string, fmt, ap);
#endif
		va_end(ap);
		DBG_PRINT_EX(tag, level, lf_str, string);
	}
}

void bse_log_print(const char *tag, const char *lf_str, const char *fmt, ...)
{
	if(sw_get_debug_mode()) {
		va_list ap;
		va_start(ap,fmt);
#ifdef CONFIG_OS_LINUX
		vsnprintf(string, DBG_STRING_MAX_LEN , fmt, ap);
#elif CONFIG_OS_WIN32
		_vsnprintf(string, DBG_STRING_MAX_LEN , fmt, ap);
#else
		vsprintf(string, fmt, ap);
#endif
		va_end(ap);
		DBG_PRINT_EX(tag, BSE_DEBUG_LEVEL_INFO, lf_str, string);
	}
}
