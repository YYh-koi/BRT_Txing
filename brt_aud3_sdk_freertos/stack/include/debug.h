/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* 
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    debug.h
Abstract:
    This file contains generic debugging macros for driver development.
    If (DBG == 0) no code is generated; Otherwise macros will expand.
Author:
	He Gang
Environment:
    Development Only.
    debug.c must be linked into the driver code to support output.
Revision History:
    1.  2000.02.10  Created
    2.  2011.03.16  (CR158)
---------------------------------------------------------------------------*/
#ifndef _DEBUG_H
#define _DEBUG_H

#include "btdatatype.h"

/*#define CONFIG_BT_DEBUG*/

/*
 * DEBUG FLAG DEFINITIONS
 */

#define DBG_ERROR_ON        0x00010000L     /* Display DBG_ERROR messages */
#define DBG_WARNING_ON      0x00020000L     /* Display DBG_WARNING messages */
#define DBG_NOTICE_ON       0x00040000L     /* Display DBG_NOTICE messages */
#define DBG_TRACE_ON        0x00080000L     /* Display ENTER/TRACE/LEAVE messages */
#define DBG_REQUEST_ON      0x00100000L     /* Enable set/query request display */
#define DBG_PARAMS_ON       0x00200000L     /* Enable function parameter display */
#define DBG_HEADERS_ON      0x00400000L     /* Enable Tx/Rx MAC header display */
#define DBG_PACKETS_ON      0x00800000L     /* Enable Tx/Rx packet display */
#define DBG_FILTER1_ON      0x01000000L     /* Display DBG_FILTER 1 messages */
#define DBG_FILTER2_ON      0x02000000L     /* Display DBG_FILTER 2 messages */
#define DBG_FILTER3_ON      0x04000000L     /* Display DBG_FILTER 3 messages */
#define DBG_FILTER_PROTO    0x08000000L     /* Display DBG_FILTER 4 messages */
#define DBG_BREAK_ON        0x10000000L     /* Enable breakpoints */
#define DBG_DATA_ON         0x20000000L     /* Enable packet print */

int DBG_PRINT_EX(const char *tag, const int level, const char *newline_break, const char *content);
#if defined(CONFIG_BT_DEBUG) || defined(CONFIG_DEBUG_SIMPLE)
int DBG_PRINT(const char * fmt, ...);
int DBG_MEM_PRINT(const char * fmt, ...);

extern void
DbgPrintData(
    UCHAR * data,
    WORD num_bytes,
    WORD offset
    );

extern INT32 dbg_flags;
extern INT32 dbg_level;
typedef void (DbgPrintFunc)(unsigned int length, unsigned char *data);
extern DbgPrintFunc * dbg_func;
void DbgEnter(int A, const char *func);
void DbgLeave(int A, const char *func);

/*---------------------------------------------------------------------------
Description:
	DEBUG tool functions.
---------------------------------------------------------------------------*/
#define DBG_OUT(d_, a_)		sprintf(&((char *)(d_))[strlen((char *)(d_))], a_
#define DBG_STR(d_, a_)		strcat((char *)(d_), a_);/* other head file is better? */
#define DBG_BACK(d_)		((char *)(d_))[strlen((char *)(d_)) - 1] = 0;/* As a backspace, this may cause crash, becareful */

void DBG_RemoveLastComma(UINT8 *dbg_str);
void DBG_SpaceAlign(UINT8 *s, UINT8 num);
void DBG_AutoWarpPrint(UINT8 *name, UINT8 *prompt, UINT8 *param);
UINT8 *DBG_GetBuf(void);

void SetDbgPrint(DbgPrintFunc* p);
/*
 *  A - is a protocol ID
 *  B - is a debug flag
 *  S - is a debug info string
 *  F - is a function name
 *  C - is a C conditional
 */

#define STATIC
#define DBG_FUNC(F)      static const char __FUNC__[] = F;
#define DBG_BREAK(A)     {if ((dbg_flags&DBG_BREAK_ON)&&(dbg_flags&(1<<A))) BREAKPOINT;}
/* WARNING DBG_PRINT(A,S)   (A) can be NULL!!!*/
#define DBG_PRINT_DATA(B,S)   {if (dbg_flags&B) DBG_PRINT S; }
#define DBG_TRACE(A)     {if ((dbg_flags & DBG_TRACE_ON)&& (dbg_flags&(1<<A))) \
								{DBG_PRINT("%-d---%s:line %d\n",dbg_level,__FUNC__,__LINE__);}}


#define DBG_ENTER(A) DbgEnter(A, __FUNC__);
#define DBG_LEAVE(A) DbgLeave(A, __FUNC__);

#define DBG_ERROR(S)   {if (dbg_flags & DBG_ERROR_ON)   \
                                {DBG_PRINT("%s: ERROR: ",__FUNC__);DBG_PRINT S;}}
#define DBG_WARNING(S) {if (dbg_flags & DBG_WARNING_ON) \
                                {DBG_PRINT("%s: WARNING: ",__FUNC__);DBG_PRINT S;}}
#define DBG_FILTER(A,B,S){if ((dbg_flags & B)&&(dbg_flags&(1<<A)))   \
                                {DBG_PRINT("%s: ",__FUNC__);DBG_PRINT S;}}

#define DBG_MSG(s) DBG_PRINT s
/*	extern int DbgInit(char * FileName);
	extern int DbgDone(void); */
#else
#ifdef DBG_PRINT
#undef DBG_PRINT
#endif
#define DBG_PRINT
#ifdef DBG_MEM_PRINT
#undef DBG_MEM_PRINT
#endif
#define DBG_MEM_PRINT
#ifdef BREAKPOINT
#undef BREAKPOINT
#endif
#define BREAKPOINT
#define STATIC           static
#define DBG_FUNC(F)
#define DBG_BREAK
#define DBG_PRINT_DATA(B,S) ;
#define DBG_ENTER(A)	STK_EnterFunc(A);
#define DBG_TRACE(A)
#define DBG_LEAVE(A)	STK_LeaveFunc(A);
#define DBG_ERROR(S)
#define DBG_WARNING(S)
#define DBG_FILTER(A,B,S)
#define DbgInit(s)
#define DbgDone()
#define DbgPrintData(a,b,c)
#define DBG_MSG(s)
#endif

#ifdef CONFIG_ANALYZER
void LogTesterData( UCHAR flag, INT32 length, UCHAR *data);
#else
#define LogTesterData(a,b,c)
#endif

#ifdef CONFIG_INTER_TESTER
#define LogInterTrace(a) LogTesterData(TESTER_TRACE, strlen(a)+1, a)
#else
#define LogInterTrace(a)
#endif

/* Definition for Tester */
#define TESTER_DBG			1
#define TESTER_IN			2
#define TESTER_OUT			4
#define TESTER_TRACE		8

#ifdef CONFIG_INTER_TESTER
#define TESTER_TRC_SPP		0x00000001
#define TESTER_TRC_BNEP		0x00000002
#define TESTER_TRC_GOEP		0x00000004	
extern INT32 g_tester_trace_level;
void SetIntTesterTraceLevel(INT32 level);
/*extern tester's func to trace data*/
extern void Trace2IntTester(UCHAR flag, INT32 length, UCHAR *data);
#define TRACE_INT_TESTER(level, flag, len, data) \
	{	if (level & g_tester_trace_level) Trace2IntTester(flag, len, data); }

#endif
#endif
