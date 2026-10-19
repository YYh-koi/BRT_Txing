/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspfsm.c														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the FSM functions of BCSP stack.					 *
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
#include "brt_trace.h"
#define LOG(fmt, ...) bse_log_info("bcsp", fmt, ## __VA_ARGS__)

static int s_bcsp_sync_times = 0;
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Private Variables																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    FSM Functions																	 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/***************************************************************************
Description:
	Start to establish link (synchronize) with BCSP stack of local device in
	ST_BCSP_CTRL_IDLE state.
Arguments:
    bcsp_fsm_user.
Return:
	NONE.
***************************************************************************/
void BcspCtrlSyncInIdle(struct BcspStru *bcsp_fsm_user)
{
	if (!bcsp_fsm_user->muzzled)
		TransmitEPPacket(SYNC_REQ_PACKET);
	bcsp_fsm_user->ft_shy = FsmAddTimerCx(g_bcsp_fsminst, CTRL_SHY_TO_VALUE, BCSP_MSG_SHY_TO,
	                                      Bcsp_FI_ShyTimeout, NULL, timer_shy_str, 0);
	bcsp_fsm_user->ctrl_state = ST_BCSP_CTRL_SHY;
}

/***************************************************************************
Description:
	Start to establish link (synchronize) with BCSP stack of local device in
	ST_BCSP_CTRL_CURIOUS state.
Arguments:
	bcsp_fsm_user
Return:
	NONE.
***************************************************************************/
void BcspCtrlSyncInCurious(struct BcspStru *bcsp_fsm_user)
{
	FsmDelTimer2(bcsp_fsm_user->ft_ack);
	bcsp_fsm_user->ft_ack = NULL;
	FsmDelTimer2(bcsp_fsm_user->ft_conf);
	bcsp_fsm_user->ft_conf = NULL;
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
	bcsp_fsm_user->ft_shy = FsmAddTimerCx(g_bcsp_fsminst, CTRL_SHY_TO_VALUE, BCSP_MSG_SHY_TO,
	                                      Bcsp_FI_ShyTimeout, NULL, timer_shy_str, 0);
	LinkFailInd(BCSP_LINK_RESYNC_START);
}

/***************************************************************************
Description:
	Start to establish link (synchronize) with BCSP stack of local device in
	ST_BCSP_CTRL_GARRULOUS state.
Arguments:
    bcsp_fsm_user
Return:
	NONE.
***************************************************************************/
void BcspCtrlSyncInGarrulous(struct BcspStru *bcsp_fsm_user)
{
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
	bcsp_fsm_user->ft_shy = FsmAddTimerCx(g_bcsp_fsminst, CTRL_SHY_TO_VALUE, BCSP_MSG_SHY_TO,
	                                      Bcsp_FI_ShyTimeout, NULL, timer_shy_str, 0);
	LinkFailInd(BCSP_LINK_RESYNC_START);
}

/***************************************************************************
Description:
	Receive establishment protocol packet in ST_BCSP_CTRL_SHY state.
Arguments:
	bcsp_fsm_user
	buf		A pointer to a BuffStru, which contains the information of
	        the establishment packet received.
Return:
	NONE.
***************************************************************************/
void BcspCtrlRxEPPacketInShy(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf)
{
	if (GET_PAYLOAD_LEN(buf) != SYNC_PAYLOAD_LEN) {
		Bcsp_Dbg_Error(buf, 0, BCSP_ERR_LEN_MISMATCH, (SYNC_PAYLOAD_LEN << 16) + GET_PAYLOAD_LEN(buf));
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), syncreq_packet, SYNC_PAYLOAD_LEN)) {
		LOG("SYNC Request received in Shy state!");
		bcsp_fsm_user->muzzled = FALSE;
		TransmitEPPacket(SYNC_RESP_PACKET);
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), syncresp_packet, SYNC_PAYLOAD_LEN)) {
		FsmDelTimer2(bcsp_fsm_user->ft_shy);
		bcsp_fsm_user->ft_shy = NULL;
		LOG("SYNC response received in Shy state! Choke is removed now");
		bcsp_fsm_user->choke = FALSE;
		bcsp_fsm_user->conf_cnt = 0;
		bcsp_fsm_user->ft_conf = FsmAddTimerCx(g_bcsp_fsminst, CTRL_CONF_TO_VALUE, BCSP_MSG_CONF_TO,
		                                       Bcsp_FI_ConfTimeout, NULL, timer_conf_str, 0);
		bcsp_fsm_user->ctrl_state = ST_BCSP_CTRL_CURIOUS;
	}
}

/***************************************************************************
Description:
	Receive establishment protocol packet in ST_BCSP_CTRL_CURIOUS state.
Arguments:
	bcsp_fsm_user
	buf		A pointer to a BuffStru, which contains the information of
	        the establishment packet received.
Return:
	NONE.
***************************************************************************/
void BcspCtrlRxEPPacketInCurious(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf)
{
	if (GET_PAYLOAD_LEN(buf) != SYNC_PAYLOAD_LEN) {
		Bcsp_Dbg_Error(buf, 0, BCSP_ERR_LEN_MISMATCH, (SYNC_PAYLOAD_LEN << 16) + GET_PAYLOAD_LEN(buf));
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), syncreq_packet, SYNC_PAYLOAD_LEN)) {
		LOG("SYNC Request received in Curious state!\r\n");
		if (!bcsp_fsm_user->start_string_detected)
			TransmitEPPacket(SYNC_RESP_PACKET);
		else
			FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_StartSync, 0);
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), confreq_packet, SYNC_PAYLOAD_LEN)) {
		LOG("CONF request received in Curious state!\r\n");
		TransmitEPPacket(CONF_RESP_PACKET);
		TransmitEPPacket(CONF_REQ_PACKET);
		bcsp_fsm_user->conf_cnt++;
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), confresp_packet, SYNC_PAYLOAD_LEN)) {
		FsmDelTimer2(bcsp_fsm_user->ft_conf);
		bcsp_fsm_user->ft_conf = NULL;
		LOG("CONF response received in Curious state!\r\n");
		bcsp_fsm_user->conf_cnt = 0;
		bcsp_fsm_user->ctrl_state = ST_BCSP_CTRL_GARRULOUS;
		if (bcsp_fsm_user->is_resync)
			LinkFailInd(BCSP_LINK_RESYNC_SUCCEED);
		else
			LinkEstablishedInd(BCSP_LINK_SYNC_SUCCEED);
	}
}

/***************************************************************************
Description:
	Receive establishment protocol packet in ST_BCSP_CTRL_GARRULOUS state.
Arguments:
	bcsp_fsm_user
	buf		A pointer to a BuffStru, which contains the information of
	        the establishment packet received.
Return:
	NONE.
***************************************************************************/
void BcspCtrlRxEPPacketInGarrulous(struct BcspStru *bcsp_fsm_user, struct BuffStru *buf)
{
	if (GET_PAYLOAD_LEN(buf) != SYNC_PAYLOAD_LEN) {
		Bcsp_Dbg_Error(buf, 0, BCSP_ERR_LEN_MISMATCH, (SYNC_PAYLOAD_LEN << 16) + GET_PAYLOAD_LEN(buf));
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), syncreq_packet, SYNC_PAYLOAD_LEN)) {
		LOG("SYNC Request received in Garrulous state!\r\n");
		FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_StartSync, 0);
	} else if (!memcmp(GET_PACKET_PAYLOAD(buf), confreq_packet, SYNC_PAYLOAD_LEN)) {
		LOG("CONF request received in Garrulous state!\r\n");
		TransmitEPPacket(CONF_RESP_PACKET);
	}
}
/***************************************************************************
Description:
	Start to establish link (synchronize) with BCSP stack of local device.
Arguments:
    fi
    event
    arg
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_StartSync(struct FsmInst *fi, UCHAR event, void *arg)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	switch (bcsp_fsm_user->ctrl_state) {
	case ST_BCSP_CTRL_IDLE:
		BcspCtrlSyncInIdle(bcsp_fsm_user);
		break;
	case ST_BCSP_CTRL_CURIOUS:
		BcspCtrlSyncInCurious(bcsp_fsm_user);
		break;
	case ST_BCSP_CTRL_GARRULOUS:
		BcspCtrlSyncInGarrulous(bcsp_fsm_user);
		break;
	default:
		break;
	}
}

/***************************************************************************
Description:
	Timer Tshy expires in ST_BCSP_CTRL_SHY state, that is to say, no SYNC
	response has been received in given time.
Arguments:
    fi
    event
    arg		NULL.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_ShyTimeout(struct FsmInst *fi, UCHAR event, void *arg)
{
	LOG("[BCSP]  Bcsp_FI_ShyTimeout Times: %d\r\n", s_bcsp_sync_times);

	struct BcspStru *bcsp_fsm_user = fi->user_data;

	if(s_bcsp_sync_times++ == 20) {
		s_bcsp_sync_times = 0;
		//system("echo btrst  > /sys/devices/platform/csr_pm.0/power_state");// patch for xinshian
		LOG("bcsp sync BT timeout , reset BT hw!!!");
	}
	if (bcsp_fsm_user->ctrl_state == ST_BCSP_CTRL_SHY) {
		if (!bcsp_fsm_user->muzzled)
			TransmitEPPacket(SYNC_REQ_PACKET);
		bcsp_fsm_user->ft_shy = FsmAddTimerCx(g_bcsp_fsminst, CTRL_SHY_TO_VALUE, BCSP_MSG_SHY_TO,
		                                      Bcsp_FI_ShyTimeout, NULL, timer_shy_str, 0);
	} else {
		bcsp_fsm_user->ft_shy = NULL;
	}
}

/***************************************************************************
Description:
	Timer Tconf expires in ST_BCSP_CTRL_CURIOUS state, that is to say, no CONF
	response has been received in given time.
Arguments:
    fi
    event
	arg		NULL
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_ConfTimeout(struct FsmInst *fi, UCHAR event, void *arg)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	bcsp_fsm_user->ft_conf = NULL;
	if (bcsp_fsm_user->ctrl_state == ST_BCSP_CTRL_CURIOUS) {
		if (bcsp_fsm_user->conf_cnt < CONF_CNT_LIMIT) {
			bcsp_fsm_user->conf_cnt ++;
			TransmitEPPacket(CONF_REQ_PACKET);
			bcsp_fsm_user->ft_conf = FsmAddTimerCx(g_bcsp_fsminst, CTRL_CONF_TO_VALUE, BCSP_MSG_CONF_TO,
			                                       Bcsp_FI_ConfTimeout, NULL, timer_conf_str, 0);
		} else
			bcsp_fsm_user->conf_cnt = 0;
	}
}

/***************************************************************************
Description:
	Timer Tack expires in any state, that is to say, no ACK has been received
	in given time. 	It is called to retransmit the packets not acknowledged.
Arguments:
    fi
    event
	arg		NULL
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_AckTimeout(struct FsmInst *fi, UCHAR event, void *arg)
{
	struct BuffStru **p;
	struct BuffStru *pstru;
	struct BuffStru *buf;
	int count = MAX_NUMBER_OF_RETRIES;
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	p = bcsp_fsm_user->packet_wait4ack_list.head;
	bcsp_fsm_user->ft_ack = NULL;
	while (p != NULL) {
		pstru = *p;
		if (count && (GET_RETRIES(pstru) >= count)) {
			Bcsp_Dbg_Error(pstru, 1, BCSP_ERR_TOO_MORE_RETRIES, 0);
			FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_StartSync, 0);
			return;
		} else {
			SET_FLAGS_ACK(pstru, bcsp_fsm_user->cur_ack);
			buf = AddPacketToTransLayer(pstru);
			TransmitBytesToUart(buf->data + buf->off, buf->len);
			Bcsp_Dbg_Error(buf, 1, BCSP_ERR_ACKTO, 0);
			BuffFree(buf);
			if (count)
				INCR_RETRIES(pstru);
		}
		p = LNEXT(p);
	}
	bcsp_fsm_user->ft_ack = FsmAddTimerCx(g_bcsp_fsminst, TL_WAIT_ACK_TO_VALUE, BCSP_MSG_WAIT_ACK_TO,
	                                      Bcsp_FI_AckTimeout, NULL, timer_wack_str, 0);
}

/***************************************************************************
Description:
	It is called to transmit a packet from HCI through a BCSP link.
Arguments:
    fi
    event
	arg		Pointer to a BuffStru structure containing the packet to be
			transmitted.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_SendReliable(struct FsmInst *fi, UCHAR event, struct BuffStru *buf)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	if (bcsp_fsm_user->packet_wait4tx_list.head != NULL) {
		TransmitPacketInWait4TxList();
	}
	if (bcsp_fsm_user->win_space) {
		TransmitPacket(buf);
	} else {
		if (!AddPacketToBufferListTail(&bcsp_fsm_user->packet_wait4tx_list, buf)) {
			BuffFree(buf);
			Bcsp_Dbg_Error(NULL, 1, BCSP_ERR_ADDLIST, 0);
		}
	}
}

/***************************************************************************
Description:
	It is called to transmit a SCO packet from HCI through a BCSP link.
Arguments:
    fi
    event
	arg		Pointer to a BuffStru structure containing the packet to be
			transmitted.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_SendSco(struct FsmInst *fi, UCHAR event, struct BuffStru *buf)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	SET_FLAGS_ALL(buf, UNRELIABLE_DATAGRAM, bcsp_fsm_user->cur_ack, 0);
	TransmitPacket(buf);
}

/***************************************************************************
Description:
	It is called to deal with a link established protocol packet received form
	the BCSP link.
Arguments:
    fi
    event
	arg		Pointer to a BuffStru structure containing the packet to be
			parsed.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_ReceiveLEPPacket(struct FsmInst *fi, UCHAR event, struct BuffStru *buf)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	switch (bcsp_fsm_user->ctrl_state) {
	case ST_BCSP_CTRL_SHY:
		BcspCtrlRxEPPacketInShy(bcsp_fsm_user, buf);
		break;
	case ST_BCSP_CTRL_CURIOUS:
		BcspCtrlRxEPPacketInCurious(bcsp_fsm_user, buf);
		break;
	case ST_BCSP_CTRL_GARRULOUS:
		BcspCtrlRxEPPacketInGarrulous(bcsp_fsm_user, buf);
		break;
	default:
		break;
	}
	BuffFree(buf);
}

/***************************************************************************
Description:
	It is called to deal with a data packet received form the BCSP link.
Arguments:
    fi
    event
	arg		Pointer to a BuffStru structure containing the packet to be
			parsed.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_ReceiveDataPacket(struct FsmInst *fi, UCHAR event, struct BuffStru *buf)
{
	DemuxPacket(buf);
}

/***************************************************************************
Description:
	It is called to destory BCSP resource.
Arguments:
	NONE.
Return:
	NONE.
***************************************************************************/
void Bcsp_FI_Done(struct FsmInst *fi, UCHAR event, void *arg)
{
	struct BcspStru *bcsp_fsm_user = fi->user_data;

	LOG("[Done] Bcsp_FI_Done-------->");
	if (bcsp_fsm_user->ft_shy != NULL) {
		FSM_DelTimer(&bcsp_fsm_user->ft_shy);
	}
	if (bcsp_fsm_user->ft_conf != NULL) {
		FSM_DelTimer(&bcsp_fsm_user->ft_conf);
	}
	if (bcsp_fsm_user->ft_ack != NULL) {
		FSM_DelTimer(&bcsp_fsm_user->ft_ack);
	}

	RemoveAllBufferListMembers(&bcsp_fsm_user->packet_wait4tx_list);
	RemoveAllBufferListMembers(&bcsp_fsm_user->packet_wait4ack_list);

	SlipDone();
	psk_env_done();

	FREE(bcsp_fsm_user);
	FsmInstFree(g_bcsp_fsminst);
	g_bcsp_fsminst = NULL;
}

void Bcsp_Clear(void)
{
	s_bcsp_sync_times = 0;
}
