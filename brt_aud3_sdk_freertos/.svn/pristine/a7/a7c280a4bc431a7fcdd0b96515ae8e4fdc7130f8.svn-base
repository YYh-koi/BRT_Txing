/*****************************************************************************
 * BCSP Transport Layer DLL Source File                                      *
 *                                                                           *
 * MODULE : bcspslip.c														 *
 *                                                                           *
 * VERSION : BCSP-1.0.4-020902                                               *
 *                                                                           *
 * DESCRIPTION :                                                             *
 *                                                                           *
 *    This file contains functions implementing the BCSP Slip layer.		 *
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
#include "bcspslip.h"
#include "bcsphlp.h"
#include "bcspfsm.h"
#include "bcspintegrity.h"


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Private Variables																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static UCHAR slip_buffer[8];
struct BuffStru* s_tmp_buf = NULL;
static DWORD slip_packet_len = 0;
static UCHAR s_get_header = 0;
static UCHAR byte_db_detected = FALSE;
static UCHAR start_byte_detected = FALSE;


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *																					 *
 *    Public Functions																 *
 *																					 *
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*************************************************************************************
Description:
Translate the packet stream into a byte stream.
Arguments:
struct BuffStru * pstru;	Pointer to the BcspPacketStru structure
                                containing the packet to be converted.
Return:
Packet after slipping, NULL if fail.
*************************************************************************************/
struct BuffStru* PutPacketIntoSlipLayer(struct BuffStru *pstru)
{
	struct BuffStru	*buf;
	WORD index;

	buf = BuffNew((UINT16)(GET_PACKET_LEN(pstru) << 1 + BCSP_BOUNDARY_LEN), 0);
	index = buf->len - 1;
	SLIP_CONVERSION(GET_PACKET_HEADER(pstru), buf->data, GET_PACKET_LEN(pstru), index);
	buf->off = index;
	buf->len -= index;
	return buf;
}

/************************************************************************************
Description:
	Translate the byte stream from UART into a packet stream.
Arguments:
    UCHAR datain;	Pointer to the buffer containing the data to be handled.
	WORD  len;		Length of the data to be handled.
Return:
	NONE.
************************************************************************************/
void PutByteIntoSlipLayer(UCHAR *datain, DWORD len)
{
	DWORD i;

	for (i = 0; i < len; i ++) {
		if ((start_byte_detected == FALSE) && (datain[i] != 0xc0))
			continue;
		/* The first start byte is detected. */
		if ((start_byte_detected == FALSE) && (datain[i] == 0xc0)) {
			start_byte_detected = TRUE;
			continue;
		}
		/* Another start/end byte is detected. */
		if ((datain[i] == 0xc0) && (!slip_packet_len)) {
			continue;
		}

		/* The packet received is complete */
		if ((datain[i] == 0xc0) && slip_packet_len) {
			if (s_tmp_buf != NULL) {
				if (CheckIntegrity(s_tmp_buf->data, &slip_packet_len)) {
					s_tmp_buf->len = (WORD)slip_packet_len;
					if (GET_PROT_ID(s_tmp_buf) == LINK_ESTABLISHMENT_PROTOCOL)
						FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_ReceiveLEPPacket, s_tmp_buf);
					else
						FsmEvent_ExternalTx(g_bcsp_fsminst, Bcsp_FI_ReceiveDataPacket, s_tmp_buf);
				} else {
					BuffFree(s_tmp_buf);
				}
			} else {
				/* Less than 4 bytes (header length) are received between two 0xC0. */
				Bcsp_Dbg_Error(NULL, 0, BCSP_ERR_LEN_TOO_SHORT, slip_packet_len);
			}
			slip_packet_len = 0;
			byte_db_detected = FALSE;
			s_tmp_buf = NULL;
			s_get_header = 0;
			continue;
		}
		/* Receive a valid byte before find the end byte */
		if (s_get_header) {
			if (slip_packet_len < s_tmp_buf->len) {
				REVERSE_CONVERSION(s_tmp_buf->data, datain[i], slip_packet_len);
			} else {
				/* Packet overflow: Header error or terminated 0xC0 missed. */
				Bcsp_Dbg_Error(s_tmp_buf, 0, BCSP_ERR_LEN_MISMATCH, (s_tmp_buf->len << 16) + (slip_packet_len + 1));
				BuffFree(s_tmp_buf);
				slip_packet_len = 0;
				byte_db_detected = FALSE;
				s_tmp_buf = NULL;
				s_get_header = 0;
				continue;
			}
		} else {
			REVERSE_CONVERSION(slip_buffer, datain[i], slip_packet_len);
			if ((slip_packet_len == BCSP_HEADER_LEN && !byte_db_detected) ||
			    slip_packet_len > BCSP_HEADER_LEN) {
				WORD buf_len = (slip_buffer[1] >> 4) + (((WORD)slip_buffer[2] << 4) & 0xff0) +
				               BCSP_HEADER_LEN + 3; /* 3 = 2 CRC + 1 Slip byte */
				s_tmp_buf = BuffNew(buf_len, 0);
				/* Compitable to the version using different BuffNew, e.g. in the embedded Linux platform. */
				s_tmp_buf->len -= s_tmp_buf->off;
				s_tmp_buf->off = 0;
				memcpy(s_tmp_buf->data, slip_buffer, slip_packet_len);
				s_get_header = 1;
			}
		}
	}
	return;
}

/************************************************************************************
Description:
	Reset Slip layer.
Arguments:
    UCHAR datain;	Pointer to the buffer containing the data to be handled.
	WORD  len;		Length of the data to be handled.
Return:
	NONE.
************************************************************************************/
void SlipDone(void)
{
	if (s_tmp_buf != NULL) {
		BuffFree(s_tmp_buf);
		s_tmp_buf = NULL;
	}
	s_get_header = 0;
	slip_packet_len = 0;
	byte_db_detected = FALSE;
	start_byte_detected = FALSE;
}
