#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include "autoconf_bt.h"
#include "os_adapt.h"
#include "FreeRTOS.h"
#include "FreeRTOS_POSIX.h"
#include <sys/types.h>
#include "board.h"

#ifdef CONFIG_OS_LINUX
#include <linux_types.h>
#endif

#ifndef ABS
#define ABS(a) (a>0?a:-a)
#endif

#ifndef BT_IS_DIGIT
#define BT_IS_DIGIT(c)	(((c) >= '0') && ((c) <= '9'))
#endif

#ifndef CAST_TO_INTPTR
#define CAST_TO_INTPTR intptr_t
#endif

/* Data type definition */
#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef int8_t
#define int8_t signed char
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef UCHAR
#define UCHAR	unsigned char
#endif

#ifndef CHAR
#define CHAR	signed char
#endif

#ifndef ULONG
#define ULONG	unsigned long
#endif

#ifndef LONG
#define LONG	signed long
#endif

#ifndef UINT
#define UINT	unsigned int
#endif

#ifndef INT32
#define	INT32	signed int
#endif

#ifndef UINT8
#define UINT8   unsigned char
#endif

#ifndef UINT16
#define UINT16   unsigned short
#endif

#ifndef UINT32
#define UINT32	unsigned int
#endif

#ifndef WCHAR
#define WCHAR   unsigned short
#endif

#ifndef DWORD
#define DWORD UINT32
#endif

#ifndef BYTE
#define BYTE	unsigned char
#endif

#ifndef PBYTE
#define PBYTE   unsigned char *
#endif

#ifndef WORD
#define WORD UINT16
#endif

#ifndef PDWORD
#define PDWORD  DWORD *
#endif

#ifndef PVOID
#define PVOID	void *
#endif

#ifndef PCHAR
#define PCHAR   signed char *
#endif

#ifndef SYSTEM_LOCK
#define SYSTEM_LOCK	void *
#endif

#ifndef TIMER_ID
#define TIMER_ID	DWORD
#endif

/* (x) >> PTR_BITS == (x) / sizeof(PVOID) */
#ifdef CONFIG_CPU_BIT64
#define PTR_BITS 3
#else
#define PTR_BITS 2
#endif

#ifndef NULL
#define NULL	0
#endif
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifdef CONFIG_IAR
#define ULONGLONG double
#define LONGLONG double
#else
#ifndef CONFIG_MICROSOFT
#define ULONGLONG unsigned long long
#define LONGLONG signed long long
#else
#define ULONGLONG unsigned __int64
#define LONGLONG signed __int64
#define BOOL	int
#endif
#endif

#ifndef BOOL
#define BOOL	int
#endif


#ifndef LPCSTR
#define LPCSTR 	const char *
#endif

#ifndef LPSTR
#define LPSTR 	char *
#endif

#ifndef LPVOID
#define LPVOID	void *
#endif

#ifndef LPDWORD
#ifdef CONFIG_CPU_BIT64
#define LPDWORD unsigned int *
#elif defined(CONFIG_CPU_BIT32)
#define LPDWORD unsigned long *
#endif
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

#ifndef HANDLE
#define HANDLE void *
#endif

#ifndef INLINE
#ifdef CONFIG_OS_WIN32
#define INLINE __inline
#elif defined(CONFIG_OS_LINUX)
#define INLINE inline
#else
#define INLINE
#endif
#endif

typedef	void *BTHANDLE;
typedef	void *BT_FILE_HDL;
typedef void *BT_FIND_HDL;

#ifndef INVALID_FILE_HDL
#define INVALID_FILE_HDL    NULL
#endif

#ifndef INVALID_FIND_HDL
#define INVALID_FIND_HDL    NULL
#endif

#define BDLENGTH		6
#define LINKKEYLENGTH	16
#define MAXPINLENGTH	16
#define DEVCLSLENGTH	3
#define MAX_NAME_LEN	64

#define BT_SUCCESS		1
#define BT_FAIL			0
#define INVALID_STATUS	-1

#define BD_ADDR_LEN	6	/* bluetooth address length */

#define MINSELF(x, y)	if ((y) < (x)) {(x) = (y);}
#define MAXSELF(x, y)	if ((y) > (x)) {(x) = (y);}

#ifndef MIN
#define MIN(a,b) (((UINT32)(a) > (UINT32)(b)) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b) (((UINT32)(a) > (UINT32)(b)) ? (a) : (b))
#endif

typedef struct _BtUUIDStru {
	DWORD Data1;
	WORD  Data2;
	WORD  Data3;
	BYTE  Data[8];
} BtUUIDStru;

typedef struct _BtUINT64Stru {
	UINT32	low32;
	UINT32	high32;
} BtUINT64Stru;

#endif

