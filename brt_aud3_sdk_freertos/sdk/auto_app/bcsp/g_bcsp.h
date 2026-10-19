/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcsphlp.h														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains the constants and macros used in the whole project. *
 *	  It also contains prototypes of following type functions:				 *
 *			Functions exported by HCI and UART to BCSP stack;				 *
 *			Functions used to trace BCSP stack debug information;			 *
 * 																			 *
 * AUTHOR : Ye Dongxiang                                                     *
 *                                                                           *
 * HISTORY :                                                                 *
 *                                                                           *
 * ------------------------------------------------------------------------- *
 * Error No. |   Date   |   Changed by   |         Short description         *
 * ---------+----------+----------------+----------------------------------- *
 *           |          |                |                                   *
 ****************************************************************************/
#ifndef BCSP_GLOBAL_H
#define BCSP_GLOBAL_H

/***************************************************************************
 *    Public constants			                                           *
 ***************************************************************************/
/*#define MAX_PACKET_IN_WAIT4TX_LIST 21*/
#define MAX_TX_WINDOW_SIZE		2
#define MAX_NUMBER_OF_RETRIES	20
#define CONF_CNT_LIMIT			2
#define BCSP_ACK_WIN_SIZE		1

#define SYNC_PAYLOAD_LEN		4
#define ACK_PAYLOAD_LEN			0
#define BCSP_HEADER_LEN			4
#define BCSP_BOUNDARY_LEN		2		/* One start byte0xc0 and one end byte 0xc0 */
#define UI_TX_HDL_FIELD_LEN		2		/* The first UI_TX_HDL_FIELD_LEN bytes of packet in two 
										   packet lists are field ID of the UI_SEND_PACKET_HANDLE. */
#define RETRY_COUNTER_LEN		1		/* The byte of packet after the field ID in two packet lists 
										   is retries. */
#define HCI_TYPE_LEN			1		/* One byte HCI packet type, this byte would not
                                           be passed through UART */
#ifdef BCSP_CRC_PRESENT
#define BCSP_CRC_LEN			2
#define CRC_PRESENT_MASK		0X40
#else
#define BCSP_CRC_LEN			0
#define CRC_PRESENT_MASK		0x00
#endif

/************************ Error Code ****************************/
#define BCSP_ERR_NONE			    0
#define BCSP_ERR_INVALID_CHKSUM     1
#define BCSP_ERR_INVALID_SEQ	    2		/* A packet with an invalid SEQ number is received. */
#define BCSP_ERR_LEN_TOO_SHORT	    3		/* The real size of a packet received is smaller 
                                             than the size of a packet header (4bytes). */
#define BCSP_ERR_LEN_MISMATCH	    4		/* The real size of the payload received is different 
										     from the value of "Payload Length" field in the header. */
#define BCSP_ERR_INVALID_CRC	    5
#define BCSP_ERR_CHOKE_ON		    6		/* Choke is set now */
#define BCSP_ERR_ADDLIST	 	    7		/* Fail to add packet to a list */
#define BCSP_ERR_CREATE_BUFFER_FAIL 8		/* An error occurs when pass a HCI packet through BCSP stack */
#define BCSP_ERR_ACKTO              9       /* ACK timeout and retransmit */
#define BCSP_ERR_TOO_MORE_RETRIES   10      /* Retransmission up to limit */

/************************ Timeout Value *************************/
#define CTRL_LINK_ESTABL_TO_VALUE	12000
#define CTRL_SHY_TO_VALUE			250
#define CTRL_CONF_TO_VALUE			250
#define TL_WAIT_ACK_TO_VALUE		300

/********************* Type of BCSP Packet **********************/
enum {
    ACK_PACKET = 0,
    LINK_ESTABLISHMENT_PROTOCOL = 1,
    CHIP_MANAGEMENT_HC = 2,
    CHIP_MANAGEMENT_CH = 3,
    DEVICE_MANAGEMENT = 4,
    COMMAND = 5,
    EVENT = 5,
    ACL_DATA = 6,
    SCO_DATA = 7,
    TYPE_NONE = 8
};

enum {
    UNRELIABLE_DATAGRAM = 0,
    RELIABLE_DATAGRAM = 0x80
};

enum {
    SYNC_REQ_PACKET,
    SYNC_RESP_PACKET,
    CONF_REQ_PACKET,
    CONF_RESP_PACKET
};

/****************** User defined Idex for message *******************/
enum {
    BCSP_MSG_EXIT_THREAD,
    BCSP_MSG_LINK_TO,
    BCSP_MSG_SHY_TO,
    BCSP_MSG_CONF_TO,
    BCSP_MSG_WAIT_ACK_TO,

    BCSP_MSG_SYNC,
    BCSP_MSG_TL_SEND_PACKET,
    BCSP_MSG_TL_SEND_SCO,

    BCSP_MSG_TL_EP_PACKET_IN,
    BCSP_MSG_TL_DATA_PACKET_IN
};

/***************** Link Resync status value - LinkFailInd ********************/
enum {
    BCSP_LINK_RESYNC_START,
    BCSP_LINK_RESYNC_SUCCEED,
    BCSP_LINK_RESYNC_TIMEOUT
};

/*************** Link Sync status value - LinkEstablishedInd *****************/
enum {
    BCSP_LINK_SYNC_SUCCEED,
    BCSP_LINK_SYNC_TIMEOUT
};

/*****************************************************************************
 *    Definition of public structures                                        *
 *****************************************************************************/
struct BcspStru {
	struct ListStru packet_wait4tx_list;
	struct ListStru packet_wait4ack_list;
	struct FsmTimer *ft_shy;
	struct FsmTimer *ft_conf;
	struct FsmTimer *ft_ack;
	WORD  ctrl_state;
	UCHAR choke;
	UCHAR cur_ack;
	UCHAR cur_seq;
	UCHAR conf_cnt;
	UCHAR muzzled;
	UCHAR win_space;
	UCHAR start_string_detected;
	UCHAR last_ack;
	UCHAR is_resync;
};

struct UartPacketStru {
	DWORD len;
	UCHAR *data;
};

struct BcspParamStru {
	void *inp;
	void *outp;
};


/*****************************************************************************
 *    Global variables declaration		                                     *
 *****************************************************************************/
extern struct FsmInst *g_bcsp_fsminst;

extern const char timer_shy_str[];
extern const char timer_conf_str[];
extern const char timer_wack_str[];
extern const char timer_sack_str[];

extern const UCHAR syncreq_packet[];
extern const UCHAR syncresp_packet[];
extern const UCHAR confreq_packet[];
extern const UCHAR confresp_packet[];


/*****************************************************************************
 *    Prototypes of functions exported by HCI and UART layer		         *
 *****************************************************************************/
extern int TransmitBytesToUart(UCHAR *data, ULONG len);
extern void ReceiveDataFromBcsp(UCHAR type, struct BuffStru *buf);
extern void LinkFailInd(UCHAR status);
extern void LinkEstablishedInd(UCHAR status);


/*****************************************************************************
 *    Debug output macros and functions								         *
 *****************************************************************************/
#if (defined(CONFIG_BT_DEBUG) && defined(CONFIG_BCSP_TRACE_ON))

#define DBG_ERROR_TRACE_ON			0x00000001
#define DBG_RX_PACKET_TRACE_ON		0x00000002
#define DBG_TX_PACKET_TRACE_ON		0x00000004
#define DBG_CONTINUE_RXCOMMENT		0x00000008
#define DBG_CONTINUE_TXCOMMENT		0x00000010
#define DBG_ALL_TRACE_ON			0x0000001F

void Bcsp_Dbg_SetDbgFlags(UINT32 flags);
void Bcsp_Dbg_TXPacket(struct BuffStru *buf);
void Bcsp_Dbg_RXPacket(struct BuffStru *buf);
void Bcsp_Dbg_Error(struct BuffStru *buf, UINT8 direction, UINT8 err_code, UINT32 Hexp_Lgot);

#else
#define Bcsp_Dbg_SetDbgFlags
#define Bcsp_Dbg_TXPacket
#define Bcsp_Dbg_RXPacket
#define Bcsp_Dbg_Error

#endif

#endif
