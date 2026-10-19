/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspui.c														 *
 *                                                                           *
 * VERSION : BCSP-2.0.0-100613                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains UI functions exported by the BCSP stack to the		 *
 *	  upper host HCI and lower UART driver.								     *
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
#include "csr_pskey.h"
#include "bcspbuflist.h"
#include "bcsphlp.h"
#include "bcspslip.h"
#include "bcspintegrity.h"
#include "bcspfsm.h"
#include "bcspui.h"
#include "brt_trace.h"

#define LOG(fmt, ...) bse_log_info("bcsp", fmt, ## __VA_ARGS__)
/***************************************************************************
Description:
	Initialize the BCSP stack.
Arguments:
	NONE.
Return:
	NONE.
***************************************************************************/
void BcspInit(void)
{
	struct BcspStru *bcsp_fsm_user;

	g_bcsp_fsminst = FsmInstNew(0, "BcspFsm");
	bcsp_fsm_user = (struct BcspStru *)NEW(sizeof(struct BcspStru));
	memset(bcsp_fsm_user, 0, sizeof(struct BcspStru));
	g_bcsp_fsminst->user_data = bcsp_fsm_user;
	bcsp_fsm_user->ctrl_state = ST_BCSP_CTRL_IDLE;
	bcsp_fsm_user->choke = TRUE;
	bcsp_fsm_user->win_space = MAX_TX_WINDOW_SIZE;
}

/***************************************************************************
Description:
	Destory the BCSP stack.
Arguments:
	NONE.
Return:
	NONE.
***************************************************************************/
void BcspDone(void)
{
	FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_Done, 0);
}

/************************************************************************
Description:
	Establish a BCSP link with lower BCSP stack.
Arguments:
    NONE
Return:
	NONE.
************************************************************************/
void BcspSyncWithLocalDeviceReq(void)
{
	FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_StartSync, 0);
}

/***********************************************************************
Description:
	Reset the BCSP states and resynchronize with the lower BCSP stack.
Arguments:
	NONE.
Return:
	NONE.
***********************************************************************/
void BcspResync(void)
{
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	FsmDelTimer2(bcsp_fsm_user->ft_ack);
	bcsp_fsm_user->ft_ack = NULL;
	bcsp_fsm_user->choke = TRUE;
	bcsp_fsm_user->muzzled = FALSE;
	bcsp_fsm_user->cur_ack = 0;
	bcsp_fsm_user->cur_seq = 0;
	bcsp_fsm_user->start_string_detected = FALSE;
	bcsp_fsm_user->last_ack = 0;
	RemoveAllBufferListMembers(&bcsp_fsm_user->packet_wait4ack_list);
	RemoveAllBufferListMembers(&bcsp_fsm_user->packet_wait4tx_list);
	bcsp_fsm_user->win_space = MAX_TX_WINDOW_SIZE;
	bcsp_fsm_user->is_resync = TRUE;
	bcsp_fsm_user->ctrl_state = ST_BCSP_CTRL_SHY;
	SlipDone();
	bcsp_fsm_user->ft_shy = FsmAddTimerCx(g_bcsp_fsminst, CTRL_SHY_TO_VALUE, BCSP_MSG_SHY_TO,
	                                      Bcsp_FI_ShyTimeout, NULL, timer_shy_str, 0);
	LinkFailInd(BCSP_LINK_RESYNC_START);
}

/*****************************************************************************
Description:
	Take packets which come from the layer above, and send them out over
	the UART link .
Arguments:
    protid: 	Protocol identification of the packet.
	buf   :   	Packet to be transmited.
Return:
    TRUE if succeed, FALSE else.
*****************************************************************************/
WORD BcspSendPacketFromHci(UCHAR protid, UCHAR *buff, UINT16 len)
{
	WORD result = FALSE;
	if (g_bcsp_fsminst != NULL) {
		struct BuffStru *pstru;
		struct HandleStruct *uihdl = NULL;
		struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

		if (!bcsp_fsm_user->choke) {
			pstru = BuffNew((WORD)(BCSP_HEADER_LEN + len + BCSP_CRC_LEN - HCI_TYPE_LEN), 0);
			memset(pstru->data, 0, pstru->len);
			memcpy(GET_PACKET_PAYLOAD(pstru), buff + HCI_TYPE_LEN,
			       len - HCI_TYPE_LEN);
			SET_PROTID_LEN(pstru, protid, (len - HCI_TYPE_LEN));
			if (protid != SCO_DATA) {
				SET_FLAGS_ALL(pstru, RELIABLE_DATAGRAM, 0, 0);
				FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_SendReliable, pstru);
			} else {
				FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_SendSco, pstru);
			}
			result = TRUE;
		} else {
			Bcsp_Dbg_Error(NULL, 1, BCSP_ERR_CHOKE_ON, 0);
		}
	}
	return result;
}

/*******************************************************************************
Description:
	RcvBytesFromUart is called when a stream of bytes are received from
	UART and put them through slip, integrity and MUX layer.
Arguments:
    data:  	Byte stream coming from the UART.
	len :	Length of the bytes received.
Return:
    NONE.
*******************************************************************************/
void BcspRcvBytesFromUart(UCHAR *data, DWORD len)
{
#ifdef CONFIG_BCSP_PRINT_DATA
	LOG("[BCSP] Rx: %d", len);
	DbgPrintData(data, len ,0);
#endif
	if (g_bcsp_fsminst != NULL) {
		PutByteIntoSlipLayer(data, len);
	}
}

