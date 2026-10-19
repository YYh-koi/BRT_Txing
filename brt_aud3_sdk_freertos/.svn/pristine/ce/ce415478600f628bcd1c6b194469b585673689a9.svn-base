/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspfsm.h														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the constants used in bcspfsm.c.					 *
	  It also contains prototype of the functions defined in bcspfsm.c.		 *
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
#ifndef BRT_BCSP_FSM_H
#define BRT_BCSP_FSM_H

/*****************************************************************************
 *    Private constants														 *
 *****************************************************************************/

/********************** FSM State Definition ****************/
enum {
    ST_BCSP_CTRL_IDLE,
    ST_BCSP_CTRL_SHY,
    ST_BCSP_CTRL_CURIOUS,
    ST_BCSP_CTRL_GARRULOUS
};

/*****************************************************************************
 *    Prototypes of functions												 *
 *****************************************************************************/
void BcspCtrlSyncInIdle(struct BcspStru *bcsp_fsm_user);

void BcspCtrlRxEPPacketInShy(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf);

void BcspCtrlRxEPPacketInCurious(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf);
void BcspCtrlSyncInCurious(struct BcspStru *bcsp_fsm_user);

void BcspCtrlRxEPPacketInGarrulous(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf);
void BcspCtrlSyncInGarrulous(struct BcspStru *bcsp_fsm_user);

void Bcsp_FI_StartSync(struct FsmInst *fi, UCHAR event, void *arg);
void Bcsp_FI_ShyTimeout(struct FsmInst *fi, UCHAR event, void *arg);
void Bcsp_FI_ConfTimeout(struct FsmInst *fi, UCHAR event, void *arg);
void Bcsp_FI_AckTimeout(struct FsmInst *fi, UCHAR event, void *arg);
void Bcsp_FI_SendReliable(struct FsmInst *fi, UCHAR event, struct BuffStru *buf);
void Bcsp_FI_SendSco(struct FsmInst *fi, UCHAR event, struct BuffStru *buf);
void Bcsp_FI_ReceiveLEPPacket(struct FsmInst *fi, UCHAR event, struct BuffStru *buf);
void Bcsp_FI_ReceiveDataPacket(struct FsmInst *fi, UCHAR event, struct BuffStru *buf);
void Bcsp_FI_Done(struct FsmInst *fi, UCHAR event, void *arg);

#endif
