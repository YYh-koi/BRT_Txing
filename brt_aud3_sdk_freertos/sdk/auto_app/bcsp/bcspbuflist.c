/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspbuflist.c													 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the functions managing the packet lists.			 *
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


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Inclusion of header files														 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "global.h"
#include "g_bcsp.h"


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Public Functions																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*******************************************************************************
Description:
	Remove a member from the packet list.
Arguments:
    struct Btlist * plist;  Pointer to the list from which the head is removed.
	UCHAR * pos;	The member to be removed.
Return:
	The member that is next to the pos in the list.
*******************************************************************************/
struct BuffStru** RemoveBufferListAt(struct ListStru *plist, void *pos)
{
	struct BuffStru **r = NULL;
	ULONG pi;
	struct BuffStru *buf;

	if(pos != NULL) {
		r = LNEXT(pos);
		List_RemoveAt(plist, pos);
		memcpy(&pi, pos, sizeof(struct BuffStru*));
		buf = (struct BuffStru*)pi;
		FREE(buf);
		LFREE(pos);
	}
	return r;
}

/*******************************************************************************
Description:
	Remove all the members from packet list.
Arguments:
	struct BtList * plist;	The list whose members are to be removed.
Return:
	NONE.
*******************************************************************************/
void RemoveAllBufferListMembers(struct ListStru *plist)
{
	struct BuffStru **r;

	r = plist->head;
	while(r != NULL) {
		r = RemoveBufferListAt(plist, r);
	}
}

/*******************************************************************************
Description:
	Free a packet list.
Arguments:
	struct BtList * plist;	The list to be freed.
Return:
	NONE.
*******************************************************************************/
void BufferListFree(struct ListStru *plist)
{
	RemoveAllBufferListMembers(plist);
}

/*********************************************************************************
Description:
	Add a packet to the tail of the packet list, without protection.
Arguments:
    struct Btlist * plist;  Pointer to the list to which the packet is added.
	struct BuffStru * pstru;	Pointer to the BuffStru structure
	                        containing the packet to be added.
Return:
	TRUE if succeed, else FALSE.
*********************************************************************************/
WORD AddPacketToBufferListTail(struct ListStru *plist, struct BuffStru *pstru)
{
	struct BuffStru **p = NULL;

	if ((plist == NULL) || (pstru == NULL))
		return FALSE;
	p = List_NodeNew(sizeof(struct BuffStru **));
	if (p) {
		*p = pstru;
		List_AddTail(plist, p);
		return TRUE;
	} else {
		return FALSE;
	}
}

