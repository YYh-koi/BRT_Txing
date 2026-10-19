/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcsphlp.h														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the macros used in the whole project.				 *
	  It also contains prototype of the functions defined in bcsphlp.c.		 *
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
#ifndef BRT_BCSP_HELP_H
#define BRT_BCSP_HELP_H

/*****************************************************************************
 *    Public macros	used to handle the packet header						 *
 *****************************************************************************/
#define GET_PACKET_HEADER(buf)		   (buf->data + buf->off)
#define GET_PACKET_PAYLOAD(buf)		   (GET_PACKET_HEADER(buf) + BCSP_HEADER_LEN)
#define GET_PACKET_LEN(buf)			   (buf->len - buf->off)

#define GET_SEQ_VAL(buf)		 ((*GET_PACKET_HEADER(buf)) & 0x07)
#define GET_ACK_VAL(buf)		 (((*GET_PACKET_HEADER(buf)) & 0x38) >> 3)
#define GET_CRC_PRESENT(buf)	 ((*GET_PACKET_HEADER(buf)) & 0x40)
#define GET_PROT_TYPE(buf)	     ((*GET_PACKET_HEADER(buf)) & 0x80)
#define GET_PROT_ID(buf)		 ((*(GET_PACKET_HEADER(buf) + 1)) & 0x0f)
#define	GET_PAYLOAD_LEN(buf)	 ((WORD)((*(GET_PACKET_HEADER(buf) + 1)) >> 4) \
                                  + (((WORD)(*(GET_PACKET_HEADER(buf) + 2)) << 4) & 0xff0))
#define GET_CHECK_SUM(buf)	     (*(GET_PACKET_HEADER(buf) + 3))

#define GET_RETRIES(buf)		 (buf->data[UI_TX_HDL_FIELD_LEN])

#define SET_PROTID_LEN(buf, pid, len)	 {*(GET_PACKET_HEADER(buf) + 1) = (pid > 7)? TYPE_NONE : pid; \
										  *(GET_PACKET_HEADER(buf) + 1) |= (UCHAR)((len & 0x0f) << 4); \
										  *(GET_PACKET_HEADER(buf) + 2) = (UCHAR)(len >> 4); }

#define SET_FLAGS_ALL(buf, type, ack, seq)    {*GET_PACKET_HEADER(buf) = type + \
                                                                         CRC_PRESENT_MASK + \
																	     (ack << 3) + seq;}

#define SET_FLAGS_ACK(buf, ack)	     {*GET_PACKET_HEADER(buf) = ((*GET_PACKET_HEADER(buf)) & 0xC7) \
                                                                + (ack << 3);}

#define SET_CHECK_SUM(buf)	 {*(GET_PACKET_HEADER(buf) + 3) = CALC_CHKSUM(GET_PACKET_HEADER(buf));}

#define INCR_1_MOD_7(a)  do {\
++a; \
a &= 7; \
} while (0);

#define INCR_RETRIES(buf)	 {buf->data[UI_TX_HDL_FIELD_LEN] ++;}

#define CHECK_SEQ_VALUE(seq)	(seq == bcsp_fsm_user->cur_ack)


/*****************************************************************************
 *    Prototypes of functions												 *
 *****************************************************************************/
void              DemuxPacket(struct BuffStru *buf);
UCHAR             MatchAckVal(UCHAR ack);
void              ResolveRxAckVal(UCHAR ack);
void              PacketArrived(struct BuffStru *pstru);

struct BuffStru*  AddPacketToTransLayer(struct BuffStru *pstru);
WORD              TransmitPacket(struct BuffStru* pstru);
void              TransmitEPPacket(UCHAR type);
void              TransmitAckPacket(UCHAR txack);
WORD              TransmitPacketInWait4TxList(void);


#endif
