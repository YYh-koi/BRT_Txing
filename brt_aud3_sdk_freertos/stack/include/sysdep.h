/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sysdep.h
Abstract:
	This file includes the definition of system dependent functions.
	Definition for different Operating system:
		CONFIG_OS_WIN32: For Windows 98/2000
		CONFIG_OS_LINUX: For Linux
		CONFIG_OS_WINCE: For Windows CE 2.12
		CONFIG_OS_WINKERNEL: For Windows Kernel module which is developed using VTOOLSD
		CONFIG_OS_REX:   For Rex
		CONFIG_OS_PSOS:	 For PSOS+
		CONFIG_OS_VXWORK:  For Vxwork 5.3 and above
		CONFIG_OS_NUCLEUS:	 For Nucleus
		CONFIG_OS_UC:	 For uC/OS-II
		CONFIG_OS_NONE:	 For no opeating system.

	Definition for other options:
		CONFIG_MEMORY_STATIC: use static memory allocation.
		CONFIG_ALIGN_ONEBYTE: one byte alignment for data structure.
		CONFIG_ALIGN_TWOBYTES: two byte alignment for data structure.
		CONFIG_LITTLE_ENDIAN: Using little endian encoding for integer.
Author:
    He Gang
Revision History:
    1.  2000.02.10  Created
	2.  2002.01.20  Change the module to different library.
	3.  2006.04.18	Declare WideToUTF7 and UTF7ToWide.
	4.  2011.03.16  (CR158)
---------------------------------------------------------------------------*/
#ifndef SYSTEMDEP_H
#define SYSTEMDEP_H

#include "btdatatype.h"
#ifndef AUTOCONF_INCLUDED
#include "autoconf_bt.h"
#endif
#include "include.h"

#include "bluedroid/bluetooth.h"
#include "bterrno.h"
#ifdef  CONFIG_MEMORY_STATIC
#include "btmem.h"
#endif

typedef void(THREAD_FUNC)(void);
typedef void(THREAD_FUNC2)(void *);

void InitTimerEnv();
void DoneTimerEnv();
void TimerHandle();

/* System dependent functions */
BOOL BTOSDepInit(void);
void BTOSDepDone(void);

SYSTEM_LOCK CREATE_CRITICAL(LPCSTR name);
void ENTER_CRITICAL(SYSTEM_LOCK mylock);
void LEAVE_CRITICAL(SYSTEM_LOCK mylock);
void FREE_CRITICAL(SYSTEM_LOCK mylock);
void WAITFOR_SINGLEOBJECT(HANDLE myobj);
void WAITFOR_SINGLEOBJECT_TIME(HANDLE myobj, DWORD milliseconds);
void BTCLOSE_HANDLE(HANDLE myobj);
void SET_EVENT(HANDLE myobj);
HANDLE CREATE_EVENT(LPCSTR name);
void CLOSE_EVENT(HANDLE myev);
void *CREATE_THREAD(THREAD_FUNC * p);
void JOIN_THREAD(void *thread_handle);
void CREATE_THREAD2(THREAD_FUNC2 * p, void* param);
void CREATE_THREADEX(
	UCHAR *name,
	THREAD_FUNC2 * p, 		/* Start address */
	WORD		stksize,	/* Stack size */
	UCHAR*		stk,		/* Stack */
	WORD		pri,		/* Priority */
	void* param				/* Parameter */
);

void SLEEP(DWORD milli_sec);
DWORD GetCurrTime(void);

#ifdef  CONFIG_MEMORY_LEAK_DETECT
#define NEW(A)  DBG_NEW((int)A,__FILE__,__LINE__)
#define FREE(A)  DBG_FREE((void*)A,__FILE__,__LINE__)
void * DBG_NEW (int size, char* filename, int line);
void DBG_FREE (void * buf, char* filename, int line);
SYSTEM_LOCK DBG_CREATE_CRITICAL(char *a, char* filename, int line);
int mem_info_out(int flag_earse);
#else
#ifdef CONFIG_MEMORY_STATIC
#define NEW mem_malloc
#define FREE mem_free
void * mem_malloc (int size);
void mem_free(void * buf);
#else
void * NEW(int size);
void FREE(void * buf);
#endif
#endif

#ifdef CONFIG_BT_DEBUG
int DBG_PRINT( const char *format, ...);
#endif


/*  System dependent timer functions */
struct FsmTimer;
struct BuffStru;

int InitTimer(struct FsmTimer *ft);
int AddTimer(struct FsmTimer *ft);
int DelTimer(struct FsmTimer *ft);

/*
 * Codeing and decoding functions
*/
#define STRU_NOTRANS	 (0x80000000)
#define STRU_TRANS_MULTI (0x40000000)

WORD EncodeNew(UCHAR * dest, UCHAR * src, DWORD mask,UCHAR reverse);
WORD DecodeNew(UCHAR * dest, UCHAR * src, DWORD mask,UCHAR reverse);
#define Encode(a,b,c) EncodeNew(a,b,c,0)
#define Decode(a,b,c) DecodeNew(a,b,c,0)
UCHAR StruLen( DWORD mask );

WORD MyEncode( UCHAR * buf, char * fmt, ...);

/*-----------------------------------------------------------------------------
Description:
	CPU Endian with Protocal Endian.
-----------------------------------------------------------------------------*/
#define DECODE2BYTE_LITTLE(ptr) 	(UINT16)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8))
#define DECODE4BYTE_LITTLE(ptr)\
	(UINT32)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8) \
	| (*(UINT8 *)((ptr) + 2) << 16) | (*(UINT8 *)((ptr) + 3) << 24))
#define ENCODE2BYTE_LITTLE(ptr, data)\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);
#define ENCODE4BYTE_LITTLE(ptr, data)\
	*(UINT8 *)((ptr) + 3) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);

#define DECODE2BYTE_BIG(ptr) 	(UINT16)(*(UINT8 *)((ptr) + 1) | (*(UINT8 *)(ptr) << 8))
#define DECODE4BYTE_BIG(ptr)\
	(UINT32)(*(UINT8 *)((ptr) + 3) | (*(UINT8 *)((ptr) + 2) << 8) \
	| (*(UINT8 *)((ptr) + 1) << 16) | (*(UINT8 *)(ptr) << 24))
#define ENCODE2BYTE_BIG(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 1) = (UINT8)(data);
#define ENCODE4BYTE_BIG(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 3) = (UINT8)(data);

/*
 * Unicode and ASCII transaction
*/
/* using CP_ACP code page */
WORD Get_ACP(void);
void Set_ACP(WORD acp);
int WideToByte(char* lpw,char* lpa,int len);
int ByteToWide(char* lpa,char* lpw,int len);
/* using CP_UTF8 code page */
int WideToUTF8(char* lpw,char* lpa,int len);
int UTF8ToWide(char* lpa,char* lpw,int len);
/* using CP_UTF7 cod page */
int WideToUTF7(char* lpw,char* lpa,int len);
int UTF7ToWide(char* lpa,char* lpw,int len);

/* String Routine. */
int BTUtil_StrICmp(const char * dst, const char * src);
UINT BTUtil_AtoUI(const char *string);
char *BTUtil_StrStrI(const char *str, const char *charset);
void BTUtil_AtoU64(const char *src_str, int src_len, UINT32 *phigh, UINT32 *plow);
int BTUtil_U64toA(UINT32 high, UINT32 low, char *dest_str, int dest_size);
int BTUtil_UTF8StrnCpy(char *lstr, int llen, char *sstr, int slen);
int BTUtil_WcsLen(const WCHAR *str);
WCHAR *BTUtil_WcsCpy(WCHAR *dest_str, const WCHAR *src_str);
WCHAR *BTUtil_WcsMemset(WCHAR *dest, const WCHAR cval, int count);
WCHAR *BTUtil_WcsReverseByteOrder(WCHAR *str, UINT8 endian);
#define BTUtil_WcsBE2Sys(_str)		BTUtil_WcsReverseByteOrder(_str, 1)
#define BTUtil_WcsSys2BE(_str)		BTUtil_WcsReverseByteOrder(_str, 1)

/* For compatibility */
#define STRICMP         BTUtil_StrICmp
#define EnhancedAtoI    BTUtil_AtoUI
#define STRSTRI         BTUtil_StrStrI
#define ATOU64          BTUtil_AtoU64
#define U64TOA          BTUtil_U64toA
#define ShortenString   BTUtil_UTF8StrnCpy

#ifndef FUNC_EXPORT 
#define FUNC_EXPORT 
#endif

#ifdef CONFIG_CHECK_STACK
#ifdef CONFIG_OS_WIN32
void STK_StartChk(UCHAR f,const char *func);
WORD STK_EnterFunc(const char *func);
void STK_LeaveFunc(const char *func);
void STK_StopChk(UCHAR f);
#else
#define STK_StartChk(f,d)
#define STK_EnterFunc(f)
#define STK_LeaveFunc(f)
#define STK_StopChk(f)
#endif
#else
#define STK_StartChk(f,d)
#define STK_EnterFunc(f)
#define STK_LeaveFunc(f)
#define STK_StopChk(f)
#endif

#ifdef  CONFIG_MEMORY_LEAK_DETECT
#if defined(CONFIG_OS_WIN32) || defined(CONFIG_OS_LINUX)
void SetMemLeakFlag(void);
void MapFilenameToProt(char *fn,char *prot);
void RegProtForStat(char *affix);
void RegStackProtForStat(void);
void EndMemDetect();
#else
#define SetMemLeakFlag()
#define MapFilenameToProt(fn,prot) 
#define RegProtForStat(af)
#define RegStackProtForStat()
#define EndMemDetect()
#endif
#else
#define SetMemLeakFlag()
#define MapFilenameToProt(fn,prot) 
#define RegProtForStat(af)
#define RegStackProtForStat()
#define EndMemDetect()
#endif

void porting_hci_dump_data(unsigned char direction, unsigned int length, unsigned char *data);
int ReadFrom(unsigned char flag, unsigned short offset, int len, unsigned char *data);
int WriteTo(unsigned char flag, unsigned short offset, int len, unsigned char *data);
void GetLocalProp(const char *key, char *value, int *value_len);
void SetLocalProp(const char *key, char *value);
void GetProp(const char* sec, const char *key, char *value, int *value_len, char* file_name);
int dep_i2c_write_data(unsigned int reg_addr, unsigned char *p_buff, unsigned int length);
int dep_i2c_read_data(unsigned int reg_addr, unsigned char *p_buff, unsigned int length);
void *get_porting_transport_layer();
void set_debug_level(int level);
const char * get_configure_path(unsigned char type);
void set_configure_path(unsigned char type, const char* path);

#endif 

