/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspui.h														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the prototype of the functions defined in			 *
 *	  bcspui.c.																 *
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
#ifndef BRT_BCSP_UI_H
#define BRT_BCSP_UI_H

/*****************************************************************************
 *    Prototypes of BCSP UI functions											 *
 *****************************************************************************/
void BcspInit(void);
void BcspDone(void);
void BcspSyncWithLocalDeviceReq(void);
void BcspResync(void);

WORD BcspSendPacketFromHci(UCHAR protid, UCHAR *buff, UINT16 len);
void BcspRcvBytesFromUart(UCHAR *data, DWORD len);

#endif
