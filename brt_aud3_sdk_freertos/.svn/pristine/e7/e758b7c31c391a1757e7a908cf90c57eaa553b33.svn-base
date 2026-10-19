/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspbuflist.h													 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the prototype of the functions defined in			 *
 *	  bcspbuflist.c.														 *
 *																			 *
 * AUTHOR : Ye Dongxiang                                                     *
 *                                                                           *
 * HISTORY :                                                                 *
 *                                                                           *
 * ------------------------------------------------------------------------- *
 * Error No. |   Date   |   Changed by   |         Short description         *
 * ---------+----------+----------------+----------------------------------- *
 *           |          |                |                                   *
 ****************************************************************************/
#ifndef BRT_BCSP_BUFLIST_H
#define BRT_BCSP_BUFLIST_H


/*****************************************************************************
 *    Prototypes of functions	  											 *
 *****************************************************************************/
struct BuffStru**	   RemoveBufferListAt(struct ListStru *plist, void *pos);
void                   RemoveAllBufferListMembers(struct ListStru *plist);
void                   BufferListFree(struct ListStru *plist);
WORD                   AddPacketToBufferListTail(struct ListStru *plist,
        struct BuffStru *pstru);



#endif
