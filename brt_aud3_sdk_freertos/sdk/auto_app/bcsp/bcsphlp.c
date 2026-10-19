/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcsphlp.c														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the auxiliary functions called by other modules.	 *
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
#include "autoconf_bt.h"
#include "global.h"
#include "g_bcsp.h"
#include "bcspbuflist.h"
#include "bcspslip.h"
#include "bcspintegrity.h"
#include "bcsphlp.h"
#include "bcspfsm.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Public Functions																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*******************************************************************************
Description:
	Put a packet through MUX layer and pass it to upper layer.
Arguments:
    UCHAR rxbyte;			Data byte coming from UART.
Return:
	Packet received if whole packet is gotten, NULL if else.
*******************************************************************************/
const char csr_uart_start_str[] = {0x0f, 0x04, 0x00, 0x01,0x00, 0x00};
void DemuxPacket(struct BuffStru *buf)
{
	UCHAR  ack, seq, protid;
	DWORD  payloadlen;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	ack = GET_ACK_VAL(buf);
	seq = GET_SEQ_VAL(buf);
	protid = GET_PROT_ID(buf);
	payloadlen = GET_PAYLOAD_LEN(buf);
	/* Only link establishment protocol packet can be passed when choke is set. */
	if (bcsp_fsm_user->choke) {
		Bcsp_Dbg_Error(buf, 0, BCSP_ERR_CHOKE_ON, 0);
		if (!payloadlen) {
			FREE(buf);
			return;
		}
		/* CSR UART driver will transfer a start string to host when receiving
		   SYNC response from host. Respond to this packet even when choke is set. */
		if (!memcmp(GET_PACKET_PAYLOAD(buf), csr_uart_start_str, 6)) {
			DBG_PRINT("CSR UART start string received when choke is set.\r\n");
			bcsp_fsm_user->cur_ack = seq + 1;
			bcsp_fsm_user->start_string_detected = TRUE;
			TransmitAckPacket(bcsp_fsm_user->cur_ack);
		}
		FREE(buf);
		return;
	}
	ResolveRxAckVal(ack);
	if ((protid == ACK_PACKET) && (!seq) && (!payloadlen)) {
		/* ACK packet arrived. */
		Bcsp_Dbg_RXPacket(buf);
		FREE(buf);
		return;
	}
	if (GET_PROT_TYPE(buf) == UNRELIABLE_DATAGRAM) {
		Bcsp_Dbg_RXPacket(buf);
		PacketArrived(buf);
		return;
	} else if (CHECK_SEQ_VALUE(seq)) {
		Bcsp_Dbg_RXPacket(buf);
		INCR_1_MOD_7(bcsp_fsm_user->cur_ack);
#if (BCSP_ACK_WIN_SIZE > 1)
		if (((bcsp_fsm_user->last_ack + BCSP_ACK_WIN_SIZE) & 7) == bcsp_fsm_user->cur_ack) {
			TransmitAckPacket(bcsp_fsm_user->cur_ack);
			bcsp_fsm_user->last_ack = bcsp_fsm_user->cur_ack;
		}
#else
		TransmitAckPacket(bcsp_fsm_user->cur_ack);
#endif
		PacketArrived(buf);
		return;
	} else {
		Bcsp_Dbg_Error(buf, 0, BCSP_ERR_INVALID_SEQ, seq);
		TransmitAckPacket(bcsp_fsm_user->cur_ack);
		FREE(buf);
	}
}

/*******************************************************************************
Description:
	Remove ACK message and stop ACK timer.
Arguments:
	None.
Return:
	None.
*******************************************************************************/
void DeleteAckTimer(void)
{
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;
	FsmDelTimer2(bcsp_fsm_user->ft_ack);
	bcsp_fsm_user->ft_ack = NULL;
}

/*******************************************************************************
Description:
	Match the rxack value with the seq value of all the packets unacknowledged
	and remove those acknowledged by this rxack value.
Arguments:
	UCHAR ack;	The rxack to be matched.
Return:
	The space of the transmit window to be increased.
*******************************************************************************/
UCHAR MatchAckVal(UCHAR ack)
{
	struct BuffStru **pcur, **pprev, **pnext;
	UCHAR seq;
	UCHAR dwin = 0;
	UCHAR match = FALSE;
	struct BuffStru *pstru = NULL;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	if (bcsp_fsm_user->packet_wait4ack_list.head == NULL) {
		DeleteAckTimer();
		return MAX_TX_WINDOW_SIZE;
	}
	pcur = bcsp_fsm_user->packet_wait4ack_list.tail;
	while (pcur != NULL) {
		pprev = pcur;
		pstru = *pcur;
		pcur = LPREV(pcur);
		seq = GET_SEQ_VAL(pstru);
		INCR_1_MOD_7(seq);
		if ((pstru != NULL) && (seq == ack)) {
			match = TRUE;
			break;
		}
	}
	if (match) {
		pnext = bcsp_fsm_user->packet_wait4ack_list.head;
		while (pnext != NULL) {
			pcur = pnext;
			pnext = RemoveBufferListAt(&bcsp_fsm_user->packet_wait4ack_list, pcur);
			dwin ++;
			if(pcur == pprev)
				break;
		}
	}
	if (dwin) {
		DeleteAckTimer();	/* Ensure that no other ack timer is running. */
		if (bcsp_fsm_user->packet_wait4ack_list.head != NULL)
			bcsp_fsm_user->ft_ack = FsmAddTimerCx(g_bcsp_fsminst, TL_WAIT_ACK_TO_VALUE, BCSP_MSG_WAIT_ACK_TO,
			                                      Bcsp_FI_AckTimeout, NULL, timer_wack_str, 0);
	}
	return dwin;
}

/*******************************************************************************
Description:
	Check rxack value; transmit packets waiting to be sent in the list if
	transmit window if not full; if rxack is not right or no reliable
	data packet to be transmitted, transmit an ACK packet if needed.
Arguments:
	UCHAR ack;	The rxack value to be examined.
Return:
	NONE.
*******************************************************************************/
void ResolveRxAckVal(UCHAR ack)
{
	UCHAR count;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	count = MatchAckVal(ack);
	if (!count) {
		return;
	}
	bcsp_fsm_user->win_space = ((bcsp_fsm_user->win_space + count) < MAX_TX_WINDOW_SIZE) ?
	                           (bcsp_fsm_user->win_space + count) : MAX_TX_WINDOW_SIZE;

	if (bcsp_fsm_user->packet_wait4tx_list.head != NULL)
		TransmitPacketInWait4TxList();
}

/******************************************************************************
Description:
	Pass whole packets up to the layer above.
Arguments:
	buf ;    Pointer to the BuffStru structure containing information
	         about the packet.
Return:
     NONE.
*******************************************************************************/
void PacketArrived(struct BuffStru *buf)
{
	UCHAR protid;

	protid = GET_PROT_ID(buf);
	switch(protid) {
	case LINK_ESTABLISHMENT_PROTOCOL :
		DBG_PRINT("Link establishment protocol packet received.\r\n");
		break;
	case CHIP_MANAGEMENT_HC :
		DBG_PRINT("BCCMD channel packet received.\r\n");
		/*Notify the HCI through Vender specific event*/
		buf->data[buf->off + 1] = 0xff;			/*Vender specific event*/
		buf->data[buf->off + 2] = buf->len - 3;	/*Event content length*/
		buf->data[buf->off + 3] = 0xc2;			/*Payload descriptor of BCCMD, only one pacekt*/
		ReceiveDataFromBcsp(EVENT, buf);
		return;
	case CHIP_MANAGEMENT_CH :
		DBG_PRINT("Host query channel packet received.\r\n");
#ifdef CONFIG_BT_CHIP_CSR8350
		buf->data[buf->off + 1] = 0xff;			/*Vender specific event*/
		buf->data[buf->off + 2] = buf->len - 3;	/*Event content length*/
		buf->data[buf->off + 3] = 0xc3;			/*Payload descriptor of BCCMD, only one pacekt*/
		ReceiveDataFromBcsp(EVENT, buf);
		return;
#endif
		break;
	case DEVICE_MANAGEMENT :
		DBG_PRINT("Device management packet received.\r\n");
		break;
	case EVENT :
	case ACL_DATA :
	case SCO_DATA :
		/* header bytes will not be passed to upper layer, and 1 byte
		   transport layer packet type will be added before payload bytes. */
		buf->off = buf->off + BCSP_HEADER_LEN - HCI_TYPE_LEN;
		ReceiveDataFromBcsp(protid, buf);
		return;
	default:
		break;
	}
	BuffFree(buf);
}

/*******************************************************************************
Description:
	Put a packet from upper layer through integrity and slip layer.
Arguments:
	pack;	 Pointer to the BuffStru structure containing the packet
	         to be handled.
Return:
	Packet after integrity and slipping if succeed, NULL if fail.
*******************************************************************************/
struct BuffStru* AddPacketToTransLayer(struct BuffStru *pstru)
{
	SET_CHECK_SUM(pstru);
#ifdef BCSP_CRC_PRESENT
	CALCULATE_CRC(GET_PACKET_HEADER(pstru), GET_PACKET_LEN(pstru) - BCSP_CRC_LEN);
#endif
	return PutPacketIntoSlipLayer(pstru);
}

/***************************************************************************
Description:
	TransmitPacket is called to transmit a packet through a BCSP link.
Arguments:
	pstru       A pointer to a BuffStru, which contains the information of
				packet to be transmitted.
Return:
	Nonezero if the function succeeds.
	Zero if the function fails.
***************************************************************************/
WORD TransmitPacket(struct BuffStru* pstru)
{
	struct BuffStru *buf;
	WORD result;
	UCHAR type;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	if (!pstru)
		return TRUE;
	type = GET_PROT_TYPE(pstru);
	if (type) {
		SET_FLAGS_ALL(pstru, RELIABLE_DATAGRAM, bcsp_fsm_user->cur_ack, bcsp_fsm_user->cur_seq);
		AddPacketToBufferListTail(&bcsp_fsm_user->packet_wait4ack_list, pstru);
		INCR_1_MOD_7(bcsp_fsm_user->cur_seq);
		bcsp_fsm_user->win_space --;
	}
	buf = AddPacketToTransLayer(pstru);
	if (buf) {
		Bcsp_Dbg_TXPacket(pstru);
		result = TransmitBytesToUart(buf->data + buf->off, buf->len);
		if (type && (bcsp_fsm_user->packet_wait4ack_list.head == bcsp_fsm_user->packet_wait4ack_list.tail))
			bcsp_fsm_user->ft_ack = FsmAddTimerCx(g_bcsp_fsminst, TL_WAIT_ACK_TO_VALUE, BCSP_MSG_WAIT_ACK_TO,
			                                      Bcsp_FI_AckTimeout, NULL, timer_wack_str, 0);
		BuffFree(buf);
	} else {
		result = FALSE;
		Bcsp_Dbg_Error(pstru, 1, BCSP_ERR_CREATE_BUFFER_FAIL, 0);
	}
	if (!type) {
		BuffFree(pstru);
	}
	return result;
}

/*******************************************************************************
Description:
	Transmit a link establishment protocol packet.
Arguments:
    UCHAR type;  Type of link establishment packet to be transmitted.
Return:
	NONE.
*******************************************************************************/
void TransmitEPPacket(UCHAR type)
{
	struct BuffStru *pstru;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	pstru = BuffNew(BCSP_HEADER_LEN + SYNC_PAYLOAD_LEN + BCSP_CRC_LEN, 0);
	memset(GET_PACKET_HEADER(pstru), 0, BCSP_HEADER_LEN);
	SET_FLAGS_ALL(pstru, UNRELIABLE_DATAGRAM, bcsp_fsm_user->cur_ack, 0);
	SET_PROTID_LEN(pstru, LINK_ESTABLISHMENT_PROTOCOL, SYNC_PAYLOAD_LEN);
	switch (type) {
	case SYNC_REQ_PACKET:
		memcpy(GET_PACKET_PAYLOAD(pstru), syncreq_packet, SYNC_PAYLOAD_LEN);
		break;
	case SYNC_RESP_PACKET:
		memcpy(GET_PACKET_PAYLOAD(pstru), syncresp_packet, SYNC_PAYLOAD_LEN);
		break;
	case CONF_REQ_PACKET:
		memcpy(GET_PACKET_PAYLOAD(pstru), confreq_packet, SYNC_PAYLOAD_LEN);
		break;
	case CONF_RESP_PACKET:
		memcpy(GET_PACKET_PAYLOAD(pstru), confresp_packet, SYNC_PAYLOAD_LEN);
		break;
	}
	TransmitPacket(pstru);
}

/*******************************************************************************
Description:
	Transmit an ACK packet.
Arguments:
	UCHAR txack;	ACK value to be sent out.
Return:
	NONE
*******************************************************************************/
void TransmitAckPacket(UCHAR txack)
{
	struct BuffStru *pstru;

	pstru = BuffNew(BCSP_HEADER_LEN + BCSP_CRC_LEN, 0);
	memset(GET_PACKET_HEADER(pstru), 0, BCSP_HEADER_LEN);
	SET_FLAGS_ALL(pstru, UNRELIABLE_DATAGRAM, txack, 0);
	SET_PROTID_LEN(pstru, ACK_PACKET, ACK_PAYLOAD_LEN);
	TransmitPacket(pstru);
}

/*******************************************************************************
Description:
	Transmit packets waiting in the queue of packet_wait4tx_list.
Arguments:
	NONE.
Return:
	Nonezero if the function succeeds.
	Zero if the function fails.
*******************************************************************************/
WORD TransmitPacketInWait4TxList(void)
{
	struct BuffStru *pstru;
	UCHAR *p;
	DWORD pi;
	WORD result = FALSE;
	struct BcspStru *bcsp_fsm_user = g_bcsp_fsminst->user_data;

	while (bcsp_fsm_user->win_space) {
		p = List_RemoveHead(&bcsp_fsm_user->packet_wait4tx_list);
		if (!p) {
			return TRUE;
		}
		memcpy(&pi, p, sizeof(struct BuffStru*));
		pstru = (struct BuffStru*)pi;
		result = TransmitPacket(pstru);
		LFREE(p);
	}
	return result;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Functions for debug trace														 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#if (defined(CONFIG_BT_DEBUG) && defined(CONFIG_BCSP_TRACE_ON))
UINT32 g_bcsp_dbg_flags = DBG_ERROR_TRACE_ON;

void Bcsp_Dbg_SetDbgFlags(UINT32 flags)
{
	g_bcsp_dbg_flags = flags;
}

void Bcsp_Dbg_TXPacket(struct BuffStru *buf)
{
	if (g_bcsp_dbg_flags & DBG_TX_PACKET_TRACE_ON) {
		UINT8 *dbg_str = DBG_GetBuf();
		struct BcspStru *bcsp_user_data = g_bcsp_fsminst->user_data;

		DBG_OUT(dbg_str, "[PID:%d][SEQ:%d][ACK:%d]"), GET_PROT_ID(buf), GET_SEQ_VAL(buf), GET_ACK_VAL(buf));
		DBG_OUT(dbg_str, "[WIN:%d][ACK_LIST:%d][PEND_LIST:%d]"), bcsp_user_data->win_space, List_Count(&bcsp_user_data->packet_wait4ack_list), List_Count(&bcsp_user_data->packet_wait4tx_list));
		DBG_AutoWarpPrint("BCSP", "TX", dbg_str);
	}
}

void Bcsp_Dbg_RXPacket(struct BuffStru *buf)
{
	if (g_bcsp_dbg_flags & DBG_RX_PACKET_TRACE_ON) {
		UINT8 *dbg_str = DBG_GetBuf();
		struct BcspStru *bcsp_user_data = g_bcsp_fsminst->user_data;

		DBG_OUT(dbg_str, "[PID:%d][SEQ:%d][ACK:%d]"), GET_PROT_ID(buf), GET_SEQ_VAL(buf), GET_ACK_VAL(buf));
		DBG_OUT(dbg_str, "[WIN:%d][ACK_LIST:%d][PEND_LIST:%d]"), bcsp_user_data->win_space, List_Count(&bcsp_user_data->packet_wait4ack_list), List_Count(&bcsp_user_data->packet_wait4tx_list));
		DBG_AutoWarpPrint("BCSP", "RX", dbg_str);
	}
}

void Bcsp_Dbg_Error(struct BuffStru *buf, UINT8 direction, UINT8 err_code, UINT32 Hexp_Lgot)
{
	if (g_bcsp_dbg_flags & DBG_ERROR_TRACE_ON) {
		UINT8 *dbg_str = DBG_GetBuf();
		struct BcspStru *bcsp_user_data = g_bcsp_fsminst->user_data;

		if (buf != NULL) {
			DBG_OUT(dbg_str, "[PID:%d][SEQ:%d][ACK:%d]"), GET_PROT_ID(buf), GET_SEQ_VAL(buf), GET_ACK_VAL(buf));
		} else {
			DBG_OUT(dbg_str, "[NULL]"));
		}
		DBG_OUT(dbg_str, "[WIN:%d][ACK_LIST:%d][PEND_LIST:%d][ERR:"), bcsp_user_data->win_space, List_Count(&bcsp_user_data->packet_wait4ack_list), List_Count(&bcsp_user_data->packet_wait4tx_list));
		switch (err_code) {
	case BCSP_ERR_NONE:
		DBG_OUT(dbg_str, "NONE]"));
			break;
		case BCSP_ERR_INVALID_CHKSUM:
				DBG_OUT(dbg_str, "Invalid Check-sum][exp(%02X)!=got(%02X)]"), (UINT8)(Hexp_Lgot >> 16), (UINT8)Hexp_Lgot);
				break;
			case BCSP_ERR_INVALID_SEQ:
					DBG_OUT(dbg_str, "Invalid SEQ number][exp(%02X)!=got(%02X)]"), bcsp_user_data->cur_ack, (UINT8)Hexp_Lgot);
					break;
				case BCSP_ERR_LEN_TOO_SHORT:
						DBG_OUT(dbg_str, "Invalid packet header size][got(%d)]"), Hexp_Lgot);
						break;
					case BCSP_ERR_LEN_MISMATCH:
							DBG_OUT(dbg_str, "Packet size mismatched][exp(%d)!=got(%d)]"), (UINT16)(Hexp_Lgot >> 16), (UINT16)Hexp_Lgot);
							break;
						case BCSP_ERR_INVALID_CRC:
								DBG_OUT(dbg_str, "Invalid CRC value]"));
								break;
							case BCSP_ERR_CHOKE_ON:
									DBG_OUT(dbg_str, "Being synchronizing]"));
									break;
								case BCSP_ERR_ADDLIST:
										DBG_OUT(dbg_str, "Can't add to pending list]"));
										break;
									case BCSP_ERR_ACKTO:
											DBG_OUT(dbg_str, "Retransmit]"));
											break;
										case BCSP_ERR_TOO_MORE_RETRIES:
												DBG_OUT(dbg_str, "Too more retransmission and restart synchronization]"));
												break;
											default:
													DBG_OUT(dbg_str, "%d]"), err_code);
												}
		if (direction) {
		DBG_AutoWarpPrint("BCSP", "TX", dbg_str);
		} else {
			DBG_AutoWarpPrint("BCSP", "RX", dbg_str);
		}
	}
}

#endif
