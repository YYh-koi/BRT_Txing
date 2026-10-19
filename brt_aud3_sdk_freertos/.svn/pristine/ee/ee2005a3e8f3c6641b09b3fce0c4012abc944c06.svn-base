/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Filename:      utils.h
 *
 *  Description:   Utility functions declaration
 *
 ******************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "os_adapt.h"

#if 0
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;
typedef unsigned long long      UINT64;

typedef signed   long   INT32;
typedef signed   char   INT8;
typedef signed   short  INT16;
#endif
typedef unsigned char   BOOLEAN;

#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif

/******************************************************************************
**  Type definitions
******************************************************************************/

/************************************************************************
  ** Mailbox definitions. Each task has 4 mailboxes that are used to
  ** send buffers to the task.
  */
#define TASK_MBOX_0    0
#define TASK_MBOX_1    1
#define TASK_MBOX_2    2
#define TASK_MBOX_3    3

#define NUM_TASK_MBOX  4

/* Start and Stop system time tick callback
    * true for start system tick if time queue is not empty
     * false to stop system tick if time queue is empty
     */
typedef void (SYSTEM_TICK_CBACK)(BOOLEAN);

/************************************************************************
  **  Max Time Queue
  **/
#ifndef GKI_MAX_TIMER_QUEUES
#define GKI_MAX_TIMER_QUEUES    3
#endif

/* Timer list entry callback type
   */
typedef void (TIMER_CBACK)(void *p_tle);
#ifndef TIMER_PARAM_TYPE
#ifdef  WIN2000
#define TIMER_PARAM_TYPE    void *
#else
#define TIMER_PARAM_TYPE    UINT32
#endif
#endif

/* Define a timer list entry
   */
typedef struct _tle {
	struct _tle  *p_next;
	struct _tle  *p_prev;
	TIMER_CBACK  *p_cback;
	INT32         ticks;
	TIMER_PARAM_TYPE   param;
	UINT16        event;
	UINT8         in_use;
} TIMER_LIST_ENT;

/* Define a timer list queue
   */
typedef struct {
	TIMER_LIST_ENT   *p_first;
	TIMER_LIST_ENT   *p_last;
	INT32             last_ticks;
} TIMER_LIST_Q;

typedef struct {
	void        *p_first;
	void        *p_last;
	uint16_t    count;
} BUFFER_Q;

typedef struct _buffer_hdr {
	struct _buffer_hdr *p_next;   /* next buffer in the queue */
	UINT8   q_id;                 /* id of the queue */
	UINT8   task_id;              /* task which allocated the buffer*/
	UINT8   status;               /* FREE, UNLINKED or QUEUED */
	UINT8   Type;
} BUFFER_HDR_T;

typedef struct _free_queue {
	BUFFER_HDR_T *p_first;      /* first buffer in the queue */
	BUFFER_HDR_T *p_last;       /* last buffer in the queue */
	UINT16           size;          /* size of the buffers in the pool */
	UINT16           total;         /* toatal number of buffers */
	UINT16           cur_cnt;       /* number of  buffers currently allocated */
	UINT16           max_cnt;       /* maximum number of buffers allocated at any time */
} FREE_QUEUE_T;

/* Buffer related defines
   */
#define ALIGN_POOL(pl_size)  ( (((pl_size) + 3) / sizeof(UINT32)) * sizeof(UINT32))
#define BUFFER_HDR_SIZE     (sizeof(BUFFER_HDR_T))                  /* Offset past header */
#define BUFFER_PADDING_SIZE (sizeof(BUFFER_HDR_T) + sizeof(UINT32)) /* Header + Magic Number */
#define MAX_USER_BUF_SIZE   ((UINT16)0xffff - BUFFER_PADDING_SIZE)  /* pool size must allow for header */
#define MAGIC_NO            0xDDBADDBA

#define BUF_STATUS_FREE     0
#define BUF_STATUS_UNLINKED 1
#define BUF_STATUS_QUEUED   2

// btla-specific ++
#define GKI_USE_DEFERED_ALLOC_BUF_POOLS
// btla-specific --

#endif /* UTILS_H */

